#include "erp.h"

bool parseUnsigned(const std::string& word, int limit, int* result) {
    if (result == 0 || word.empty()) return false;
    int value = 0;
    for (std::string::size_type i = 0; i < word.size(); ++i) {
        char ch = word[i];
        if (ch < '0' || ch > '9') return false;
        int digit = ch - '0';
        if (digit > limit || value > (limit - digit) / 10) return false;
        value = value * 10 + digit;
    }
    *result = value;
    return true;
}

bool parseGpa(const std::string& word, int* result) {
    std::string::size_type dot = word.find('.');
    if (dot == std::string::npos || dot == 0 ||
        dot + 3 != word.size()) return false;
    int whole = 0;
    int fraction = 0;
    if (!parseUnsigned(word.substr(0, dot), 10, &whole) ||
        !parseUnsigned(word.substr(dot + 1), 99, &fraction)) return false;
    const int value = whole * 100 + fraction;
    if (value > 1000) return false;
    *result = value;
    return true;
}

const char* errorName(ErrorCode code) {
    switch (code) {
        case OK: return "OK";
        case E_PARSE: return "E_PARSE";
        case E_FULL: return "E_FULL";
        case E_DUPLICATE: return "E_DUPLICATE";
        case E_UNKNOWN_STUDENT: return "E_UNKNOWN_STUDENT";
        case E_UNKNOWN_COURSE: return "E_UNKNOWN_COURSE";
        case E_BAD_COURSE: return "E_BAD_COURSE";
        case E_BAD_GRADE: return "E_BAD_GRADE";
        case E_GRADE_TYPE: return "E_GRADE_TYPE";
        case E_NOT_REGISTERED: return "E_NOT_REGISTERED";
        case E_ALREADY_GRADED: return "E_ALREADY_GRADED";
        case E_THESIS_STATE: return "E_THESIS_STATE";
        case E_ALREADY_PASSED: return "E_ALREADY_PASSED";
        case E_ALREADY_REGISTERED: return "E_ALREADY_REGISTERED";
        case E_PREREQ: return "E_PREREQ";
        case E_CREDIT_LIMIT: return "E_CREDIT_LIMIT";
        case E_THESIS_NOT_ALLOWED: return "E_THESIS_NOT_ALLOWED";
        case E_NO_GPA: return "E_NO_GPA";
    }
    return "E_PARSE";
}
