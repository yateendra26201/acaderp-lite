#ifndef ACADERP_ERP_H
#define ACADERP_ERP_H

#include <string>

const int MAX_STUDENTS = 100;
const int MAX_COURSES = 50;
const int MAX_REGS = 40;

enum ErrorCode {
    OK, E_PARSE, E_FULL, E_DUPLICATE, E_UNKNOWN_STUDENT,
    E_UNKNOWN_COURSE, E_BAD_COURSE, E_BAD_GRADE, E_GRADE_TYPE,
    E_NOT_REGISTERED, E_ALREADY_GRADED, E_THESIS_STATE,
    E_ALREADY_PASSED, E_ALREADY_REGISTERED, E_PREREQ,
    E_CREDIT_LIMIT, E_THESIS_NOT_ALLOWED, E_NO_GPA
};

enum CourseType { REGULAR, SU, THESIS };
enum ThesisState { NOT_THESIS, REGISTERED, IN_PROGRESS, FINAL, FAILED };

struct Course {
    std::string code;
    CourseType type;
    int credits;
    bool core;
    int prerequisite; // index in courses, -1 if none
};

struct Registration {
    int course; // index in courses
    int term;
    std::string grade; // empty means ungraded; thesis S is a progress grade
    ThesisState thesisState;
};

struct Student {
    std::string roll;
    std::string name;
    Registration* regs;
    int regCount;
    Student() : regs(0), regCount(0) {}
    // ERP owns this allocation; never copy a Student.
private:
    Student(const Student&);
    Student& operator=(const Student&);
};

struct ProgramRules {
    int totalCredits;
    int electiveCredits;
    int minimumGpaHundredths;
    int maximumLoad;
    int thesisCredits;
    ProgramRules() : totalCredits(160), electiveCredits(24),
        minimumGpaHundredths(500), maximumLoad(24), thesisCredits(100) {}
};

class ERP {
public:
    ERP();
    ~ERP();
    ErrorCode configure(int total, int elective, int minGpa, int maxLoad, int thesis);
    ErrorCode addCourse(const std::string& code, CourseType type, int credits,
                        bool core, const std::string& prereq);
    ErrorCode addStudent(const std::string& roll, const std::string& name);
    ErrorCode registerCourse(const std::string& roll, const std::string& code, int term);
    ErrorCode submitGrade(const std::string& roll, const std::string& code,
                          int term, const std::string& symbol);
    ErrorCode gpaHundredths(const std::string& roll, int* result) const;

    // Advanced academic queries.
    ErrorCode eligibility(const std::string& roll, const std::string& code,
                          int term, int* failedRule) const;
    ErrorCode convertThesis(const std::string& roll, const std::string& code,
                            const std::string& letter);
    ErrorCode averageGrade(const std::string& roll, std::string* result) const;
    ErrorCode graduation(const std::string& roll, std::string* result) const;
    ErrorCode transcript(const std::string& roll, std::string* result) const;
    ErrorCode save(const std::string& filename) const;
    ErrorCode load(const std::string& filename);
    void swapState(ERP& other);

    int findCourse(const std::string& code) const;
    int findStudent(const std::string& roll) const;
    int findRegistration(int student, int course, int term) const;
    int latestGradedRegistration(int student, int course) const;
    bool passed(int student, int course) const;
    int earnedCredits(int student) const;
    static int gradePoints(const std::string& grade);
    static bool validGrade(const std::string& grade);

    // Bounded arrays of academic records.
    Course courses[MAX_COURSES];
    Student students[MAX_STUDENTS];
    int courseCount;
    int studentCount;
    ProgramRules rules;
private:
    ERP(const ERP&);
    ERP& operator=(const ERP&);
};

const char* errorName(ErrorCode code);
bool parseUnsigned(const std::string& word, int limit, int* result);
bool parseGpa(const std::string& word, int* result);

#endif
