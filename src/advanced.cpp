#include "erp.h"

// MEMBER B owns this file. These are explicit milestone-1 placeholders.
// Eligibility E1 is implemented; E2-E6 and advanced queries are not yet done.
ErrorCode ERP::eligibility(const std::string& roll, const std::string& code,
                           int term, int* failedRule) const {
    if (failedRule == 0 || term < 1 || term > 12) return E_PARSE;
    *failedRule = 0;
    if (findStudent(roll) < 0) { *failedRule = 1; return E_UNKNOWN_STUDENT; }
    if (findCourse(code) < 0) { *failedRule = 1; return E_UNKNOWN_COURSE; }
    return OK;
}

ErrorCode ERP::convertThesis(const std::string&, const std::string&,
                             const std::string&) {
    return E_PARSE; // TODO(member B): implement thesis state transitions
}

ErrorCode ERP::averageGrade(const std::string&, std::string*) const {
    return E_PARSE; // TODO(member B): implement GPA bands
}

ErrorCode ERP::graduation(const std::string&, std::string*) const {
    return E_PARSE; // TODO(member B): implement R1-R5
}
