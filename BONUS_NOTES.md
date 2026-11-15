# Bonus features — Syandan and Yateendra

The two optional features are implemented in `src/bonus.cpp`, using the existing
`ERP` data model. No banned STL containers or exceptions were introduced.
`<fstream>` is used exclusively for the SAVE/LOAD bonus, as expressly permitted
by the assignment.

## Transcript (+5)

Command: `TRANSCRIPT <roll>`

The PDF specifies **term order**, but not the exact text format. We chose:

```
TRANSCRIPT R1
1 CS101 A 4
2 CS499 S 8
3 HS201 - 3
```

The first line is the student roll. Each subsequent line is
`<term> <course> <current-grade> <course-credits>` sorted by ascending term;
registration insertion order breaks ties. `-` means ungraded. A converted thesis
shows its final letter grade. A student with no registrations prints only the
header. Unknown students produce `ERROR <line> E_UNKNOWN_STUDENT`.

## Save and Load (+5)

Commands: `SAVE <file>` and `LOAD <file>`.

SAVE writes a text snapshot (format version `ACADERP 1`) containing the program
rules, catalogue, students and every registration, including ungraded records
and thesis states. LOAD validates the entire snapshot using bounded counts,
identifier lengths, grade/type compatibility and registration indices before
replacing in-memory data. A failed LOAD leaves the current ERP unchanged.
SAVE writes to `<file>.tmp`, then renames it upon successful completion to
avoid replacing a valid snapshot with a partially written one. Successful
SAVE/LOAD operations print nothing.

Only relative paths of at most 100 characters, made from letters, digits,
`_`, `-`, `.`, and `/` are accepted. Paths containing `..` and absolute paths
are rejected. The parent directory must already exist. Missing, malformed or
unwritable files, or invalid filenames, return `E_PARSE` (the PDF defines no
file-I/O-specific error code). The snapshot is intended for this project's
SAVE/LOAD implementation, not as a general-purpose data-exchange format.

## Usage

```sh
make clean && make
./run_tests.sh
./acaderp < tests/sample.in
```

The bonus test suite adds T24, T25 and T26, bringing the total to 26 cases.
Temporary test snapshots are removed by `run_tests.sh`.

## Integration steps

In a fresh branch in Yateendra's Git clone, copy the new/modified `src/erp.h`,
`src/main.cpp`, `src/bonus.cpp`, `Makefile`, `run_tests.sh` and bonus test files.
Do not overwrite your existing, personalized README. Append these notes or
summarize them in its Bonus section.

Commit authentic implementation and tests, push the branch to Yateendra's fork,
then open a pull request targeting `syandanreddy7/acaderp-lite:main`.
