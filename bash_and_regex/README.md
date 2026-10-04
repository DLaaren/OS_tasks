# bash_and_regex

## Quick start

```bash
# 1. Generate the working tree (task1..task5 dirs + empty stub scripts)
python3 generate.py

# 2. Implement your solutions in task<N>/<script>.sh

# 3. Run the self-check
python3 check.py                 # all five tasks
python3 check.py --task task3    # a single task
python3 check.py --dump-tests    # dump every fixture + expected result to tests_dump/ (runs nothing)

# 4. Debugging: each failing check saves input/expected/actual to failed/<task>_<i>/
cat failed/task3_random_2/{input.txt,expected.txt,actual.txt}

# 5. Reset (careful — see below)
python3 clean.py                 # asks for confirmation
python3 clean.py --dry-run       # show what would be deleted
python3 clean.py --yes           # no confirmation
```

> **Warning:** `clean.py` deletes the `task1..task5` directories — **including
> your written scripts** — plus `failed/`. Back up your solutions or use
> `--dry-run` first.

Note: `generate.py` refuses to run if any `task<N>/` directory already exists —
delete them with `clean.py` (or manually) if you want to start over.
