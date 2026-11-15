#include "erp.h"

ERP::ERP() : courseCount(0), studentCount(0) {}

ERP::~ERP() {
    for (int i = 0; i < studentCount; ++i) {
        delete[] students[i].regs;
        students[i].regs = 0;
    }
}

ErrorCode ERP::configure(int total, int elective, int minGpa,
                         int maxLoad, int thesis) {
    if (total < 0 || elective < 0 || minGpa < 0 || minGpa > 1000 ||
        maxLoad < 0 || thesis < 0 || total > 100000 ||
        elective > 100000 || maxLoad > 100000 || thesis > 100000)
        return E_PARSE;
    rules.totalCredits = total;
    rules.electiveCredits = elective;
    rules.minimumGpaHundredths = minGpa;
    rules.maximumLoad = maxLoad;
    rules.thesisCredits = thesis;
    return OK;
}

int ERP::findCourse(const std::string& code) const {
    for (int i = 0; i < courseCount; ++i)
        if (courses[i].code == code) return i;
    return -1;
}

int ERP::findStudent(const std::string& roll) const {
    for (int i = 0; i < studentCount; ++i)
        if (students[i].roll == roll) return i;
    return -1;
}

int ERP::findRegistration(int s, int c, int term) const {
    for (int i = 0; i < students[s].regCount; ++i) {
        const Registration& r = students[s].regs[i];
        if (r.course == c && r.term == term) return i;
    }
    return -1;
}

int ERP::latestGradedRegistration(int s, int c) const {
    int latest = -1;
    for (int i = 0; i < students[s].regCount; ++i) {
        const Registration& r = students[s].regs[i];
        if (r.course != c || r.grade.empty()) continue;
        if (latest < 0 || r.term > students[s].regs[latest].term ||
            (r.term == students[s].regs[latest].term && i > latest)) latest = i;
    }
    return latest;
}

ErrorCode ERP::addCourse(const std::string& code, CourseType type, int credits,
                         bool core, const std::string& prereq) {
    if (findCourse(code) >= 0) return E_DUPLICATE;
    if (courseCount == MAX_COURSES) return E_FULL;
    if (credits < 0 || credits > 24 || (credits == 0 && type != SU) ||
        prereq == code) return E_BAD_COURSE;
    int prereqIndex = -1;
    if (prereq != "-") {
        prereqIndex = findCourse(prereq);
        if (prereqIndex < 0) return E_BAD_COURSE;
    }
    Course& c = courses[courseCount];
    c.code = code;
    c.type = type;
    c.credits = credits;
    c.core = core;
    c.prerequisite = prereqIndex;
    ++courseCount;
    return OK;
}

ErrorCode ERP::addStudent(const std::string& roll, const std::string& name) {
    if (findStudent(roll) >= 0) return E_DUPLICATE;
    if (studentCount == MAX_STUDENTS) return E_FULL;
    Student& s = students[studentCount];
    s.regs = new Registration[MAX_REGS]; // freed by ERP::~ERP
    s.roll = roll;
    s.name = name;
    s.regCount = 0;
    ++studentCount;
    return OK;
}

int ERP::gradePoints(const std::string& g) {
    if (g == "A") return 10;
    if (g == "A-") return 9;
    if (g == "B") return 8;
    if (g == "B-") return 7;
    if (g == "C") return 6;
    if (g == "C-") return 5;
    if (g == "D") return 4;
    if (g == "F") return 0;
    return -1;
}

bool ERP::validGrade(const std::string& g) {
    return gradePoints(g) >= 0 || g == "S" || g == "X";
}

bool ERP::passed(int s, int c) const {
    int index = latestGradedRegistration(s, c);
    if (index < 0) return false;
    const Registration& r = students[s].regs[index];
    if (courses[c].type == SU) return r.grade == "S";
    if (courses[c].type == THESIS && r.thesisState != FINAL) return false;
    return gradePoints(r.grade) >= 4;
}

int ERP::earnedCredits(int s) const {
    int earned = 0;
    for (int c = 0; c < courseCount; ++c)
        if (passed(s, c)) earned += courses[c].credits;
    return earned;
}

ErrorCode ERP::registerCourse(const std::string& roll,
                              const std::string& code, int term) {
    int rule = 0;
    ErrorCode status = eligibility(roll, code, term, &rule);
    if (status != OK) return status;
    const int s = findStudent(roll);
    const int c = findCourse(code);
    if (students[s].regCount == MAX_REGS) return E_FULL;
    Registration& r = students[s].regs[students[s].regCount];
    r.course = c;
    r.term = term;
    r.grade = "";
    r.thesisState = (courses[c].type == THESIS) ? REGISTERED : NOT_THESIS;
    ++students[s].regCount;
    return OK;
}

ErrorCode ERP::submitGrade(const std::string& roll, const std::string& code,
                           int term, const std::string& symbol) {
    if (!validGrade(symbol)) return E_BAD_GRADE;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    const int c = findCourse(code);
    if (c < 0) return E_UNKNOWN_COURSE;
    const int index = findRegistration(s, c, term);
    if (index < 0) return E_NOT_REGISTERED;
    Registration& r = students[s].regs[index];
    if (!r.grade.empty()) return E_ALREADY_GRADED;
    const CourseType type = courses[c].type;
    if (type == REGULAR && gradePoints(symbol) < 0) return E_GRADE_TYPE;
    if (type != REGULAR && symbol != "S" && symbol != "X") return E_GRADE_TYPE;
    r.grade = symbol;
    if (type == THESIS) r.thesisState = symbol == "S" ? IN_PROGRESS : FAILED;
    return OK;
}

ErrorCode ERP::gpaHundredths(const std::string& roll, int* result) const {
    if (result == 0) return E_PARSE;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    int numerator = 0;
    int denominator = 0;
    for (int c = 0; c < courseCount; ++c) {
        const int index = latestGradedRegistration(s, c);
        if (index < 0 || courses[c].type == SU) continue;
        const Registration& r = students[s].regs[index];
        if (courses[c].type == THESIS && r.thesisState != FINAL) continue;
        const int points = gradePoints(r.grade);
        if (points < 0) continue;
        numerator += courses[c].credits * points;
        denominator += courses[c].credits;
    }
    if (denominator == 0) return E_NO_GPA;
    *result = (200 * numerator + denominator) / (2 * denominator);
    return OK;
}
