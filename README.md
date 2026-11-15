# AcadERP-Lite
### Academic Records and Graduation Audit System

A command-line Mini-ERP application developed in **C++11** for the Object-Oriented Programming and Design (OOPD) project. The system maintains a course catalogue and student records, processes course registrations and grades, and evaluates academic progress against program requirements.

## Team and contributions

| Contributor | GitHub | Work completed |
| --- | --- | --- |
| **Syandan** | [@syandanreddy7](https://github.com/syandanreddy7) | Project setup and class definitions; student and course data management; input parsing and validation; registration and grade-recording foundations; exact GPA calculations; command processing; initial test cases and build setup. |
| **Yateendra** | [@yateendra26201](https://github.com/yateendra26201) | Complete registration eligibility rules (E1–E6); thesis state transitions and conversion; average-grade calculation; graduation audit (R1–R5); additional test cases; integration updates and documentation. |

We integrated the contributions through separate Git branches and a pull request. We also verified the integrated application using the automated test suite.

## Features

- **Program configuration:** optional academic requirements with assignment-defined defaults.
- **Course catalogue:** regular, satisfactory/unsatisfactory (SU), and thesis courses, with credits, categories, and optional prerequisites.
- **Student records:** bounded storage for up to 100 students and 50 courses.
- **Registration:** prerequisite, repeat, duplicate registration, term-credit-limit, and thesis-credit eligibility checks.
- **Grade processing:** valid grades by course type; prevention of unauthorized regrading; repeat attempts after failure.
- **Thesis workflow:** progress grade (`S` or `X`) followed by letter-grade conversion when allowed.
- **Academic queries:** GPA, average letter grade, graduation status, and course eligibility.
- **Error recovery:** invalid commands return specified error codes; processing continues with the next input line.

## Build and run

**Requirements:** a C++11-compatible `g++`, `make`, and a Unix-like shell.

```sh
make
./acaderp < tests/sample.in
./run_tests.sh
```

The Makefile builds the executable `acaderp` with the assignment's required flags:

```sh
g++ -std=c++11 -Wall -Wextra -fno-exceptions src/main.cpp src/input.cpp src/core.cpp src/advanced.cpp -o acaderp
```

To rebuild from scratch:

```sh
make clean
make
```

## Input and output

The application reads **one command per line** from standard input and writes query results or errors to standard output. Successful data-changing commands produce no output. Blank lines and comment lines beginning with `#` are ignored but count toward line numbers.

Supported commands:

```text
PROGRAM <total> <elective> <mingpa> <maxload> <thesiscredits>
COURSE <code> <REGULAR|SU|THESIS> <credits> <CORE|ELECTIVE> <prereq>
STUDENT <roll> <name>
REGISTER <roll> <course> <term>
GRADE <roll> <course> <term> <symbol>
CONVERT <roll> <thesis> <letter>
GPA <roll>
AVG <roll>
GRAD <roll>
ELIGIBLE <roll> <course> <term>
```

For example, a GPA query returns `GPA <roll> <X.YY>`, and an invalid command returns `ERROR <line> <CODE>`. See `tests/sample.in` and `tests/sample.out` for the assignment's complete worked example.

## Source layout

```text
acaderp-lite/
├── Makefile            # C++11 build
├── README.md           # Project documentation
├── run_tests.sh        # Automated regression tests
├── src/
│   ├── erp.h           # Types, constants, and ERP interface
│   ├── input.cpp       # Input validation and error names
│   ├── core.cpp        # Records, registrations, grades, GPA
│   ├── advanced.cpp    # Eligibility, thesis, AVG, graduation
│   └── main.cpp        # Command reader and dispatcher
└── tests/
    ├── T01.in / T01.out
    ├── ...
    ├── T23.in / T23.out
    └── sample.in / sample.out
```

## Implementation and safety

We used classes, structs, fixed-size arrays, `std::string`, and bounded searches rather than STL containers. Operations that can fail report an `ErrorCode`, which the command processor checks. The implementation validates command structure, numeric syntax, identifier length, course credits, semester range, array capacity, and grade/course compatibility.

**Memory ownership:** `ERP` stores up to 50 `Course` records and 100 `Student` records. When a student is added, `src/core.cpp` allocates `new Registration[MAX_REGS]` for that student's maximum 40 registrations. `ERP::~ERP` releases each allocation with `delete[]`. Copying the ERP and student objects is disallowed to prevent accidental duplicate ownership.

**GPA correctness:** the credit-weighted grade-point numerator and denominator use integer arithmetic, with half-up rounding to hundredths; no floating-point arithmetic is used for GPA. Failed regular grades count in the denominator. Only each course's counting attempt contributes.

**Error handling:** the parser processes commands line by line and reports errors without modifying academic records on failed operations. Eligibility checks occur in the specified E1–E6 order; the graduation query reports each failed R1–R5 rule in order.

**Known bounds:** input lines are checked against a 512-character policy after `std::getline` reads them, so this is not a strict allocation bound for extremely long lines. `new[]` allocation failure is not recoverable under the specified no-exceptions build; successful allocations are released by the destructor.

## Testing and verification

We provide **23** paired test cases, `tests/T01.in`–`tests/T23.in`, with corresponding expected `.out` files. The regression script runs the program on each input and compares exact output.

Coverage includes invalid grade symbols and types; missing registrations; repeated grading; thesis conversion and repeats; integer GPA rounding; average-grade boundaries; graduation rules; prerequisite and credit-load eligibility; catalogue/student limits; parsing errors; and recovery after errors.

During integration, the project compiled with the required flags and produced **23/23 passing tests**.

To verify the supplied worked example separately:

```sh
./acaderp < tests/sample.in | diff -u tests/sample.out -
```

## Assumptions and interpretations

The assignment allows reasonable interpretations of unspecified cases. We used the following:

1. `PROGRAM`, when present, must be the first **nonblank, noncomment** command. If omitted, defaults are total credits 160, elective credits 24, minimum GPA 5.00, maximum term load 24, and thesis prerequisite credits 100.
2. Course type, category, and grade symbols are case-sensitive.
3. A course's counting attempt is its latest graded attempt by term. Where attempts share a term, the most recently added is selected.
4. A thesis progress grade of `S` leaves the thesis in progress: it does not yet earn credits or enter GPA, and blocks another concurrent registration. An `X` or a final `F` permits a later attempt.
5. Conversion acts on the latest thesis registration. A thesis converted to `C` or better qualifies for graduation rule R4; conversion to `D` earns credits but does not satisfy R4.
6. All course registrations in a term count toward its load, including attempts that later fail. A load equal to the maximum is allowed.
7. Program configuration values are limited to 0–100000; minimum GPA must be written `X.YY` and fall between 0.00 and 10.00.
8. Unknown course type or category returns `E_BAD_COURSE`; malformed numbers, terms, command lengths, and identifiers return `E_PARSE`.
9. Blank/comment lines still count toward error line numbers. Input is limited by policy to 512 characters per line and at most seven command words.
10. Under the catalogue limits (50 courses, 24 credits per course), credit totals and weighted grade-point sums fit safely in an `int`.

## Scope

The required assignment commands are implemented. Optional bonus commands (`TRANSCRIPT`, `SAVE`, and `LOAD`) are **not** implemented.
