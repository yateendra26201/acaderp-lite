#include "erp.h"

// All eligibility rules run in order. No stored state is changed here.
ErrorCode ERP::eligibility(const std::string& roll, const std::string& code,
                           int term, int* failedRule) const {
    if (failedRule == 0 || term < 1 || term > 12) return E_PARSE;
    *failedRule = 1;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    const int c = findCourse(code);
    if (c < 0) return E_UNKNOWN_COURSE;

    *failedRule = 2;
    if (passed(s, c)) return E_ALREADY_PASSED;

    *failedRule = 3;
    for (int i = 0; i < students[s].regCount; ++i) {
        const Registration& r = students[s].regs[i];
        if (r.course == c && (r.grade.empty() ||
            (courses[c].type == THESIS && r.thesisState == IN_PROGRESS)))
            return E_ALREADY_REGISTERED;
    }

    *failedRule = 4;
    const int prereq = courses[c].prerequisite;
    if (prereq >= 0 && !passed(s, prereq)) return E_PREREQ;

    *failedRule = 5;
    int termCredits = 0;
    for (int i = 0; i < students[s].regCount; ++i) {
        const Registration& r = students[s].regs[i];
        if (r.term == term) termCredits += courses[r.course].credits;
    }
    if (termCredits + courses[c].credits > rules.maximumLoad)
        return E_CREDIT_LIMIT;

    *failedRule = 6;
    if (courses[c].type == THESIS && earnedCredits(s) < rules.thesisCredits)
        return E_THESIS_NOT_ALLOWED;

    *failedRule = 0;
    return OK;
}

// Choose the latest attempt by term, including an ungraded registration.
static int latestThesisAttempt(const Student& student, int course) {
    int latest = -1;
    for (int i = 0; i < student.regCount; ++i) {
        const Registration& r = student.regs[i];
        if (r.course == course && (latest < 0 ||
            r.term > student.regs[latest].term ||
            (r.term == student.regs[latest].term && i > latest))) latest = i;
    }
    return latest;
}

ErrorCode ERP::convertThesis(const std::string& roll, const std::string& code,
                             const std::string& letter) {
    if (!validGrade(letter)) return E_BAD_GRADE;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    const int c = findCourse(code);
    if (c < 0) return E_UNKNOWN_COURSE;
    if (courses[c].type != THESIS) return E_GRADE_TYPE;
    if (gradePoints(letter) < 0) return E_GRADE_TYPE;

    const int index = latestThesisAttempt(students[s], c);
    if (index < 0) return E_THESIS_STATE;
    Registration& r = students[s].regs[index];
    if (r.thesisState == FINAL) return E_ALREADY_GRADED;
    if (r.thesisState != IN_PROGRESS) return E_THESIS_STATE;
    r.grade = letter;
    r.thesisState = FINAL;
    return OK;
}

ErrorCode ERP::averageGrade(const std::string& roll, std::string* result) const {
    if (result == 0) return E_PARSE;
    int h = 0;
    const ErrorCode error = gpaHundredths(roll, &h);
    if (error != OK) return error;
    if (h >= 950) *result = "A";
    else if (h >= 850) *result = "A-";
    else if (h >= 750) *result = "B";
    else if (h >= 650) *result = "B-";
    else if (h >= 550) *result = "C";
    else if (h >= 450) *result = "C-";
    else if (h >= 200) *result = "D";
    else *result = "F";
    return OK;
}

// R2: a core course with no passing latest attempt blocks graduation.
bool allCorePassed(const ERP& erp, int s) {
    for (int c = 0; c < erp.courseCount; ++c)
        if (erp.courses[c].core && !erp.passed(s, c)) return false;
    return true;
}

int earnedElectiveCredits(const ERP& erp, int s) {
    int credits = 0;
    for (int c = 0; c < erp.courseCount; ++c)
        if (!erp.courses[c].core && erp.passed(s, c))
            credits += erp.courses[c].credits;
    return credits;
}

bool hasQualifyingThesis(const ERP& erp, int s) {
    for (int c = 0; c < erp.courseCount; ++c) {
        if (erp.courses[c].type != THESIS) continue;
        const int index = erp.latestGradedRegistration(s, c);
        if (index < 0) continue;
        const Registration& r = erp.students[s].regs[index];
        if (r.thesisState == FINAL && ERP::gradePoints(r.grade) >= 6)
            return true;
    }
    return false;
}

static void addFailedRule(std::string* answer, const char* code) {
    if (*answer == "YES") *answer = "NO ";
    else *answer += ",";
    *answer += code;
}

ErrorCode ERP::graduation(const std::string& roll, std::string* result) const {
    if (result == 0) return E_PARSE;
    const int s = findStudent(roll);
    if (s < 0) return E_UNKNOWN_STUDENT;
    std::string answer = "YES";
    if (earnedCredits(s) < rules.totalCredits) addFailedRule(&answer, "R1");
    if (!allCorePassed(*this, s)) addFailedRule(&answer, "R2");
    if (earnedElectiveCredits(*this, s) < rules.electiveCredits)
        addFailedRule(&answer, "R3");
    if (!hasQualifyingThesis(*this, s)) addFailedRule(&answer, "R4");
    int h = 0;
    const ErrorCode status = gpaHundredths(roll, &h);
    if (status == E_NO_GPA || (status == OK && h < rules.minimumGpaHundredths))
        addFailedRule(&answer, "R5");
    else if (status != OK) return status;
    *result = answer;
    return OK;
}
