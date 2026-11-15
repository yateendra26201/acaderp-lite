# AcadERP-Lite — two-person C++11 project

**Status: Member A milestone implemented; NOT final submission.** `src/advanced.cpp` currently contains explicitly marked Member B placeholders. Do not submit this starter as the finished assignment.

## Build and test

```sh
make
./run_tests.sh
# or: make test
./acaderp < tests/T08.in
make clean
```

Exact flags: `g++ -std=c++11 -Wall -Wextra -fno-exceptions`.

## Files and ownership

| File | Owner | Responsibility |
|---|---|---|
| `src/erp.h` | A (shared interface) | Enums, fixed limits, model definitions and method signatures |
| `src/core.cpp` | A | Student/course storage, regular/SU grades, thesis progress grade, GPA |
| `src/input.cpp` | A | Strict numeric parsing and error-code names |
| `src/main.cpp` | A | Command loop, word counts, syntax checks, exact response printing |
| `src/advanced.cpp` | B | Replace placeholders with E1–E6, CONVERT, AVG, GRAD |
| `tests/` | Both | A adds initial tests; B adds and verifies full required table |
| `README.md`, `run_tests.sh`, `Makefile` | A starts, B finalizes | Build, documentation and regression runner |

## Data layout and memory

`ERP` contains arrays of up to 50 courses and 100 students. Each successfully added student gets a dynamically allocated `Registration[MAX_REGS]` array via `new[]` in `ERP::addStudent`. `ERP::~ERP` calls `delete[]` once for each such allocation. The ERP and students cannot be copied. Every insertion checks the array limit before writing. Course references are array indices, not raw dangling pointers.

Student and course lookup uses straightforward loops. GPA counts only the latest graded attempt per course, skips SU and unconverted theses, and uses integer-only half-up rounding.

## Assumptions (review together before final delivery)

- `PROGRAM` is accepted only on physical input line 1, including comment/blank lines in the numbering.
- In this starter, an unknown course type/category is `E_BAD_COURSE`; malformed numeric fields are `E_PARSE`.
- GPA thresholds are accepted only in fixed two-decimal notation between `0.00` and `10.00`.
- Program credit thresholds and load limits are non-negative integers up to 100000, to keep all calculations within `int` range.
- Identifiers may contain non-whitespace characters; their lengths are limited as required.
- The command length is limited to 512 characters; excess is `E_PARSE`. `std::getline` still constructs a line before checking its size; a streaming length cap would require a different input-reading strategy.
- A thesis with progress grade `S` is not yet passed, and makes no GPA contribution until converted.
- `GRADE` rejects all grade changes once a registration has any grade, including thesis progress grades.
- For different attempts, the latest graded attempt is selected by the largest term number.
- **Still to decide with B:** whether a student can have multiple course registrations in the same term, how concurrent different-term attempts behave, and any ambiguity about thesis conversion selection. B must document the final choices.

## Safety/design notes

- Uses `std::string`, not C buffers, `strcpy` or unsafe character-array copying.
- Numeric parsing validates each character and checks the limit **before** multiplication, preventing integer overflow and silent truncation.
- Number of words, identifier sizes, terms, course credits and array indices are checked.
- Fallible operations return `ErrorCode`; callers check them.
- New state is written only after checks, so a failed user command doesn't partially modify an existing record.
- The specification requires `std::getline`, so processing one genuinely enormous line could still allocate significant memory before the line-length rejection. Do not claim this starter has a hard heap-memory cap.
- In the course-restricted header set, `new[]` without the forbidden `<new>`/exception facilities cannot report allocation failure gracefully. At the fixed maximum of 100 × 40 registrations, memory usage is small but allocation failure is not recoverable here.

## Remaining Member B checklist

1. Implement eligibility checks **E1 to E6 in order**, returning the corresponding `E_...` code for REGISTER and E-number for ELIGIBLE.
2. Implement thesis `CONVERT`: only `IN_PROGRESS` may transition to `FINAL`; support conversion to F, repeated failed theses and correct errors.
3. Implement `AVG` using the rounded integer GPA bands.
4. Implement `GRAD` with all failing rules R1–R5 in required order and exact comma-separated format.
5. Add the remaining test cases from assignment T05–T23 plus its full worked sample and invalid-input boundary tests; ensure `make test` passes.
6. Review edge cases with A, document final assumptions and submit the archive after integration.

The optional TRANSCRIPT/SAVE/LOAD bonus commands are not implemented in this starter.
