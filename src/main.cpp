#include "erp.h"
#include <iostream>
#include <string>

// Maximum meaningful command is well below 512 characters. Reject longer
// lines instead of attempting to tokenize an unbounded number of words.
const std::string::size_type MAX_LINE_LENGTH = 512;
const int MAX_WORDS = 7;

struct Words {
    std::string word[MAX_WORDS];
    int count;
    Words() : count(0) {}
};

bool splitWords(const std::string& line, Words* out) {
    if (out == 0 || line.size() > MAX_LINE_LENGTH) return false;
    std::string::size_type i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t' ||
               line[i] == '\r')) ++i;
        if (i == line.size()) break;
        if (out->count == MAX_WORDS) return false;
        std::string::size_type start = i;
        while (i < line.size() && line[i] != ' ' &&
               line[i] != '\t' && line[i] != '\r') ++i;
        out->word[out->count++] = line.substr(start, i - start);
    }
    return true;
}

bool validIdentifier(const std::string& value, int maxLength) {
    return !value.empty() && value.size() <= static_cast<std::string::size_type>(maxLength);
}

ErrorCode processCreate(ERP& erp, const Words& w, int lineNumber) {
    const std::string& cmd = w.word[0];
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    if (cmd == "PROGRAM") {
        if (lineNumber != 1 || w.count != 6 ||
            !parseUnsigned(w.word[1], 100000, &a) ||
            !parseUnsigned(w.word[2], 100000, &b) ||
            !parseGpa(w.word[3], &c) ||
            !parseUnsigned(w.word[4], 100000, &d) ||
            !parseUnsigned(w.word[5], 100000, &e)) return E_PARSE;
        return erp.configure(a, b, c, d, e);
    }
    if (cmd == "COURSE") {
        if (w.count != 6 || !validIdentifier(w.word[1], 10) ||
            !validIdentifier(w.word[5], 10) ||
            !parseUnsigned(w.word[3], 100000, &a)) return E_PARSE;
        CourseType type;
        if (w.word[2] == "REGULAR") type = REGULAR;
        else if (w.word[2] == "SU") type = SU;
        else if (w.word[2] == "THESIS") type = THESIS;
        else return E_BAD_COURSE;
        if (w.word[4] != "CORE" && w.word[4] != "ELECTIVE") return E_BAD_COURSE;
        return erp.addCourse(w.word[1], type, a, w.word[4] == "CORE", w.word[5]);
    }
    if (cmd == "STUDENT") {
        if (w.count != 3 || !validIdentifier(w.word[1], 10) ||
            !validIdentifier(w.word[2], 30)) return E_PARSE;
        return erp.addStudent(w.word[1], w.word[2]);
    }
    return E_PARSE;
}

ErrorCode processChange(ERP& erp, const Words& w) {
    const std::string& cmd = w.word[0];
    int term = 0;
    if (cmd == "REGISTER" || cmd == "GRADE") {
        if (w.count != (cmd == "GRADE" ? 5 : 4) ||
            !validIdentifier(w.word[1], 10) || !validIdentifier(w.word[2], 10) ||
            !parseUnsigned(w.word[3], 12, &term) || term < 1) return E_PARSE;
        if (cmd == "REGISTER") return erp.registerCourse(w.word[1], w.word[2], term);
        return erp.submitGrade(w.word[1], w.word[2], term, w.word[4]);
    }
    if (cmd == "CONVERT") {
        if (w.count != 4 || !validIdentifier(w.word[1], 10) ||
            !validIdentifier(w.word[2], 10)) return E_PARSE;
        return erp.convertThesis(w.word[1], w.word[2], w.word[3]);
    }
    return E_PARSE;
}

ErrorCode processQuery(ERP& erp, const Words& w) {
    const std::string& cmd = w.word[0];
    if (w.count < 2 || !validIdentifier(w.word[1], 10)) return E_PARSE;
    if (cmd == "GPA" && w.count == 2) {
        int h = 0;
        ErrorCode result = erp.gpaHundredths(w.word[1], &h);
        if (result != OK) return result;
        std::cout << "GPA " << w.word[1] << ' ' << h / 100 << '.';
        if (h % 100 < 10) std::cout << '0';
        std::cout << h % 100 << '\n';
        return OK;
    }
    if ((cmd == "AVG" || cmd == "GRAD") && w.count == 2) {
        std::string answer;
        ErrorCode result = (cmd == "AVG")
            ? erp.averageGrade(w.word[1], &answer)
            : erp.graduation(w.word[1], &answer);
        if (result != OK) return result;
        std::cout << cmd << ' ' << w.word[1] << ' ' << answer << '\n';
        return OK;
    }
    if (cmd == "ELIGIBLE" && w.count == 4) {
        int term = 0;
        if (!validIdentifier(w.word[2], 10) ||
            !parseUnsigned(w.word[3], 12, &term) || term == 0) return E_PARSE;
        int failed = 0;
        ErrorCode result = erp.eligibility(w.word[1], w.word[2], term, &failed);
        if (result == E_PARSE) return result;
        std::cout << "ELIGIBLE " << w.word[1] << ' ' << w.word[2];
        if (failed == 0) std::cout << " YES\n";
        else std::cout << " NO E" << failed << '\n';
        return OK;
    }
    return E_PARSE;
}

ErrorCode processLine(ERP& erp, const std::string& line, int number) {
    if (line.size() > MAX_LINE_LENGTH) return E_PARSE;
    Words words;
    if (!splitWords(line, &words)) return E_PARSE;
    if (words.count == 0 || words.word[0][0] == '#') return OK;
    const std::string& cmd = words.word[0];
    if (cmd == "PROGRAM" || cmd == "COURSE" || cmd == "STUDENT")
        return processCreate(erp, words, number);
    if (cmd == "REGISTER" || cmd == "GRADE" || cmd == "CONVERT")
        return processChange(erp, words);
    return processQuery(erp, words);
}

int main() {
    ERP erp;
    std::string line;
    int lineNumber = 0;
    while (std::getline(std::cin, line)) {
        ++lineNumber;
        ErrorCode result = processLine(erp, line, lineNumber);
        if (result != OK)
            std::cout << "ERROR " << lineNumber << ' ' << errorName(result) << '\n';
    }
    return 0;
}
