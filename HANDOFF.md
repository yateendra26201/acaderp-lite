# Branch & commit handoff

## Recommended history

`main` (empty repo) → `feature/member-a` (A's six commits) → merge PR into `main` without squash → `feature/member-b` created from the updated `main` (B's six commits) → merge PR without squash. No force-push, no fabricated authorship. Each person must configure **their own** GitHub-linked Git email and author their own changes.

### Member A: six commits

Commit 1: `chore: add project structure and C++11 build rules` — add `.gitignore`, `Makefile`, `src/erp.h`.

Commit 2: `feat: validate numeric inputs and map error codes` — add `src/input.cpp`.

Commit 3: `feat: manage courses students and registrations` — add `src/core.cpp`.

Commit 4: `feat: add command parser and baseline query routing` — add `src/main.cpp`, `src/advanced.cpp`. (Build now possible.)

Commit 5: `test: cover grade type errors and GPA rounding` — add `run_tests.sh`, `tests/T01.*`, `tests/T02.*`, `tests/T08.*`, `tests/T11.*`.

Commit 6: `docs: describe memory ownership and add baseline regressions` — add `README.md`, `HANDOFF.md`, `tests/T03.*`, `tests/T04.*`, `tests/T20.*`.

### Member B: six commits

Commit 1: `feat: enforce registration eligibility E1 through E6` — implement `ERP::eligibility` in `advanced.cpp` and tests T17–T19.

Commit 2: `feat: implement thesis conversion state machine` — implement `ERP::convertThesis`, tests T05–T07 and T15.

Commit 3: `feat: calculate average letter grades` — implement `ERP::averageGrade`, tests T09, T10, T13.

Commit 4: `feat: audit graduation rules R1 through R5` — implement `ERP::graduation`, tests T14, T16.

Commit 5: `test: cover repeats limits and malformed input` — add tests T12, T21–T23 and the 27-line sample, address any bugs identified with reviewed changes.

Commit 6: `docs: finalize assumptions and verify full test suite` — update `README.md`, tests, and note compiler/test results. Make an actual code or documentation improvement rather than an empty commit.

## Git setup for A

```sh
# Inside the downloaded extracted project directory:
git init
git branch -M main
git switch -c feature/member-a
# Add/commit each staged file group above in order (see six commit plan).
git remote add origin https://github.com/YOUR-USERNAME/acaderp-lite.git
git push -u origin feature/member-a
# Create and merge a Pull Request into main using GitHub (MERGE, not squash).
```

## Git setup for B — only after A's PR merged

```sh
git clone https://github.com/YOUR-USERNAME/acaderp-lite.git
cd acaderp-lite
git switch main
git pull --ff-only origin main
git switch -c feature/member-b
# Implement and commit the six changes using B's own name/email.
git push -u origin feature/member-b
# Create and merge a Pull Request into main using GitHub (MERGE, not squash).
```

To verify final history: `git log --format='%h %an <%ae> %s' main` and check that **each real contributor has at least six non-merge commits** on main. GitHub's contributions graph also requires a verified email associated with their GitHub account.
