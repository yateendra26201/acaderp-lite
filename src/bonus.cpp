#include "erp.h"
#include <fstream>  // Allowed for SAVE/LOAD bonus only.
#include <cstdio>   // rename/remove for safer saves.
#include <string>

// The assignment leaves transcript formatting and save format unspecified.
// All such decisions are documented in README.md.

// Build a transcript in memory: the caller prints it only after success.
ErrorCode ERP::transcript(const std::string& roll, std::string* result) const {
    if (result == 0) return E_PARSE;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    std::string text = "TRANSCRIPT " + roll;
    const Student& student = students[s];
    // Semesters 1..12; preserve registration order within a semester.
    for (int term = 1; term <= 12; ++term) {
        for (int i = 0; i < student.regCount; ++i) {
            const Registration& r = student.regs[i];
            if (r.term != term) continue;
            const Course& c = courses[r.course];
            const std::string grade = r.grade.empty() ? "-" : r.grade;
            text += "\n" + std::to_string(term) + " " + c.code +
                    " " + grade + " " + std::to_string(c.credits);
        }
    }
    *result = text;
    return OK;
}

// Reject whitespace, traversal, absolute paths, and metacharacters.
// Relative subdirectories are permitted, but must exist already.
static bool safeFilename(const std::string& name) {
    if (name.empty() || name.size() > 100 || name[0] == '/' ||
        name.find("..") != std::string::npos) return false;
    for (std::string::size_type i = 0; i < name.size(); ++i) {
        const char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' ||
              c == '.' || c == '/')) return false;
    }
    return name[name.size() - 1] != '/';
}

ErrorCode ERP::save(const std::string& filename) const {
    if (!safeFilename(filename)) return E_PARSE;
    const std::string temporary = filename + ".tmp";
    std::ofstream output(temporary.c_str(), std::ios::out | std::ios::trunc);
    if (!output) return E_PARSE;
    output << "ACADERP 1\n";
    output << "PROGRAM " << rules.totalCredits << ' ' << rules.electiveCredits
           << ' ' << rules.minimumGpaHundredths << ' ' << rules.maximumLoad
           << ' ' << rules.thesisCredits << '\n';
    output << "COURSES " << courseCount << '\n';
    for (int i = 0; i < courseCount; ++i) {
        const Course& c = courses[i];
        output << "C " << c.code << ' ' << static_cast<int>(c.type) << ' '
               << c.credits << ' ' << (c.core ? 1 : 0) << ' '
               << c.prerequisite << '\n';
    }
    output << "STUDENTS " << studentCount << '\n';
    for (int s = 0; s < studentCount; ++s) {
        const Student& student = students[s];
        output << "S " << student.roll << ' ' << student.name << ' '
               << student.regCount << '\n';
        for (int i = 0; i < student.regCount; ++i) {
            const Registration& r = student.regs[i];
            output << "R " << r.course << ' ' << r.term << ' '
                   << (r.grade.empty() ? "-" : r.grade) << ' '
                   << static_cast<int>(r.thesisState) << '\n';
        }
    }
    output << "END\n";
    output.close();
    if (!output) { std::remove(temporary.c_str()); return E_PARSE; }
    if (std::rename(temporary.c_str(), filename.c_str()) != 0) {
        std::remove(temporary.c_str());
        return E_PARSE;
    }
    return OK;
}

struct FileWords {
    std::string word[7];
    int count;
    FileWords() : count(0) {}
};

// Bound the line length and word count before accepting a file record.
static bool readWords(std::ifstream& input, FileWords* out) {
    if (out == 0) return false;
    std::string line;
    if (!std::getline(input, line) || line.size() > 256) return false;
    out->count = 0;
    std::string::size_type pos = 0;
    while (pos < line.size()) {
        while (pos < line.size() && line[pos] == ' ') ++pos;
        if (pos == line.size()) break;
        if (out->count == 7) return false;
        const std::string::size_type start = pos;
        while (pos < line.size() && line[pos] != ' ') {
            const unsigned char ch = static_cast<unsigned char>(line[pos]);
            if (ch < 33 || ch > 126) return false;
            ++pos;
        }
        out->word[out->count++] = line.substr(start, pos - start);
    }
    return true;
}

static bool number(const std::string& word, int low, int high, int* result) {
    if (!parseUnsigned(word, high, result)) return false;
    return *result >= low;
}

static bool validName(const std::string& name, int maxLen) {
    return !name.empty() && name.size() <= static_cast<std::string::size_type>(maxLen);
}

// Read all course records before exposing the new state.
static ErrorCode readCourses(std::ifstream& in, ERP* next) {
    FileWords w;
    if (!readWords(in, &w) || w.count != 2 || w.word[0] != "COURSES")
        return E_PARSE;
    int total = 0;
    if (!number(w.word[1], 0, MAX_COURSES, &total)) return E_PARSE;
    for (int i = 0; i < total; ++i) {
        if (!readWords(in, &w) || w.count != 6 || w.word[0] != "C" ||
            !validName(w.word[1], 10)) return E_PARSE;
        int type = 0, credits = 0, core = 0, prereq = 0;
        if (!number(w.word[2], 0, 2, &type) ||
            !number(w.word[3], 0, 24, &credits) ||
            !number(w.word[4], 0, 1, &core)) return E_PARSE;
        // The only negative index allowed is -1 (meaning no prerequisite).
        if (w.word[5] == "-1") prereq = -1;
        else if (!number(w.word[5], 0, i - 1, &prereq)) return E_PARSE;
        const std::string prereqCode = prereq < 0 ? "-" : next->courses[prereq].code;
        if (next->addCourse(w.word[1], static_cast<CourseType>(type),
                            credits, core == 1, prereqCode) != OK) return E_PARSE;
    }
    return OK;
}

static bool validRegistration(const ERP& next, const Registration& r) {
    const CourseType type = next.courses[r.course].type;
    if (type != THESIS) {
        if (r.thesisState != NOT_THESIS) return false;
        return r.grade.empty() || (type == REGULAR ?
            ERP::gradePoints(r.grade) >= 0 : (r.grade == "S" || r.grade == "X"));
    }
    if (r.thesisState == REGISTERED) return r.grade.empty();
    if (r.thesisState == IN_PROGRESS) return r.grade == "S";
    if (r.thesisState == FAILED) return r.grade == "X";
    if (r.thesisState == FINAL) return ERP::gradePoints(r.grade) >= 0;
    return false;
}

static ErrorCode readRegistrations(std::ifstream& in, ERP* next,
                                   Student* student, int count) {
    for (int i = 0; i < count; ++i) {
        FileWords w;
        if (!readWords(in, &w) || w.count != 5 || w.word[0] != "R") return E_PARSE;
        int course = 0, term = 0, state = 0;
        if (!number(w.word[1], 0, next->courseCount - 1, &course) ||
            !number(w.word[2], 1, 12, &term) ||
            !number(w.word[4], 0, 4, &state)) return E_PARSE;
        Registration record;
        record.course = course;
        record.term = term;
        record.grade = w.word[3] == "-" ? "" : w.word[3];
        record.thesisState = static_cast<ThesisState>(state);
        if (!validRegistration(*next, record)) return E_PARSE;
        for (int j = 0; j < i; ++j) {
            if (student->regs[j].course == course && student->regs[j].term == term)
                return E_PARSE;
        }
        student->regs[i].course = record.course;
        student->regs[i].term = record.term;
        student->regs[i].grade = record.grade;
        student->regs[i].thesisState = record.thesisState;
        student->regCount++;
    }
    return OK;
}

static ErrorCode readStudents(std::ifstream& in, ERP* next) {
    FileWords w;
    if (!readWords(in, &w) || w.count != 2 || w.word[0] != "STUDENTS")
        return E_PARSE;
    int total = 0;
    if (!number(w.word[1], 0, MAX_STUDENTS, &total)) return E_PARSE;
    for (int i = 0; i < total; ++i) {
        if (!readWords(in, &w) || w.count != 4 || w.word[0] != "S" ||
            !validName(w.word[1], 10) || !validName(w.word[2], 30)) return E_PARSE;
        int count = 0;
        if (!number(w.word[3], 0, MAX_REGS, &count)) return E_PARSE;
        if (next->addStudent(w.word[1], w.word[2]) != OK) return E_PARSE;
        Student* student = &next->students[i];
        if (readRegistrations(in, next, student, count) != OK) return E_PARSE;
    }
    return OK;
}

// Swap owners of registration arrays, so each destructor frees exactly its own.
void ERP::swapState(ERP& other) {
    for (int i = 0; i < MAX_COURSES; ++i) {
        Course temporary = courses[i];
        courses[i] = other.courses[i];
        other.courses[i] = temporary;
    }
    for (int i = 0; i < MAX_STUDENTS; ++i) {
        Student& a = students[i];
        Student& b = other.students[i];
        std::string temp = a.roll; a.roll = b.roll; b.roll = temp;
        temp = a.name; a.name = b.name; b.name = temp;
        Registration* regs = a.regs; a.regs = b.regs; b.regs = regs;
        int count = a.regCount; a.regCount = b.regCount; b.regCount = count;
    }
    int count = courseCount; courseCount = other.courseCount; other.courseCount = count;
    count = studentCount; studentCount = other.studentCount; other.studentCount = count;
    ProgramRules temp = rules; rules = other.rules; other.rules = temp;
}

ErrorCode ERP::load(const std::string& filename) {
    if (!safeFilename(filename)) return E_PARSE;
    std::ifstream input(filename.c_str());
    if (!input) return E_PARSE;
    ERP replacement;
    FileWords w;
    if (!readWords(input, &w) || w.count != 2 ||
        w.word[0] != "ACADERP" || w.word[1] != "1") return E_PARSE;
    if (!readWords(input, &w) || w.count != 6 || w.word[0] != "PROGRAM")
        return E_PARSE;
    int values[5];
    for (int i = 0; i < 5; ++i) {
        if (!number(w.word[i + 1], 0, i == 2 ? 1000 : 100000, &values[i]))
            return E_PARSE;
    }
    if (replacement.configure(values[0], values[1], values[2],
                              values[3], values[4]) != OK) return E_PARSE;
    if (readCourses(input, &replacement) != OK ||
        readStudents(input, &replacement) != OK) return E_PARSE;
    if (!readWords(input, &w) || w.count != 1 || w.word[0] != "END")
        return E_PARSE;
    // Reject extra data after END, including duplicate snapshots.
    std::string extra;
    if (std::getline(input, extra) || input.bad()) return E_PARSE;
    swapState(replacement);
    return OK;
}
