# AcadERP-Lite — Academic Records and Graduation Audit

C++11 command-line implementation for the OOPD Mini-ERP assignment.

## Build and run

```sh
make
./run_tests.sh
./acaderp < tests/sample.in
```

Compile flags: `g++ -std=c++11 -Wall -Wextra -fno-exceptions`. The code uses only
`<iostream>` and `<string>` from the allowed header set and does not use STL
containers, exceptions, floating-point GPA calculation, or unchecked C-string copies.

## Responsibilities

- Member A: `erp.h`, `core.cpp`, `input.cpp`, initial `main.cpp`, Makefile, initial tests.
- Member B: `advanced.cpp` (eligibility, thesis conversion, average, graduation), extra
  tests and integration update to `main.cpp` for the first noncomment `PROGRAM` line.
- Both: regression testing, assumptions, output verification, and documentation.

## Design

`ERP` owns fixed arrays of up to 50 `Course` values and 100 `Student` objects.
Each successful `addStudent` allocates `new Registration[MAX_REGS]` (40 per student)
in `src/core.cpp`. `ERP::~ERP` deletes each allocation using `delete[]`, exactly once.
The ERP and Student objects cannot be copied. Lookup uses bounded linear searches.

The command reader splits each line into a fixed 7-word buffer, validates maximum
word/identifier sizes and integer syntax, then dispatches to functions returning
`ErrorCode`. Failed commands do not alter stored academic records. Registration
eligibility is tested in E1 through E6 order. Graduation lists all failed rules R1
through R5, in order. GPA uses integer-only weighted numerator/denominator and
half-up rounding (including failing grade F).

## Tests

`tests/T01.in` ... `tests/T23.in`, paired with `.out` files. `./run_tests.sh` runs
them all and checks byte-for-byte output. `tests/sample.in` and `sample.out`
contain the PDF assignment's worked example; run as shown above. Main test runner
covers Tnn pairs; the PDF sample can also be checked using:

```sh
./acaderp < tests/sample.in | diff -u tests/sample.out -
```

## Assumptions / interpretations

- The `PROGRAM` command must be the first **nonblank, noncomment** command. This
  follows the PDF's worked example, which begins with a comment and then PROGRAM.
- Grade symbols and course keywords are case-sensitive.
- A course's counting attempt is the latest graded attempt by term number; when
  two attempts share a term, the most recently added wins.
- A thesis with progress grade S is still in progress: not passed, not in GPA,
  and not eligible for another concurrent registration. Conversion applies to
  the latest thesis registration; converted F allows a repeat.
- A thesis converted to C or better satisfies graduation rule R4. At least one
  thesis must qualify for R4; a D conversion earns credits but fails R4.
- All registrations in a given term contribute to E5 load, even if later failed.
  Exactly the maximum load is permitted.
- Credit sums are accumulated in `int`: maximum 50 courses × 24 credits = 1200,
  so these sums and grade-point sums cannot overflow under catalogue limits.
- Configuration values are restricted to the range 0..100000; minimum GPA must
  be formatted `X.YY` and belong to 0.00..10.00. This is a documented policy for
  unspecified unusually large program configurations.
- Missing optional `PROGRAM` uses the specified defaults.
- Unknown course type/category yields E_BAD_COURSE; malformed numeric values,
  terms, word counts, and identifiers yield E_PARSE.
- Every line is counted for error locations, even comments and blank lines.
- Input length is limited to 512 characters per line and 7 words per command.
  `std::getline` stores the line before length validation; extremely long input
  lines can therefore still allocate memory. There are no fixed C-string buffers
  vulnerable to buffer overwrite, but this is not a strict memory-size cap.
- The required `new[]` may terminate on allocation failure under `-fno-exceptions`;
  no exception-based recovery is attempted, and allocation is capped at 100 × 40
  registrations. All successful allocations are freed.

## Bonus

Optional `TRANSCRIPT`, `SAVE`, and `LOAD` commands are not implemented.
