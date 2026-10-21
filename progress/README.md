# Progress inputs

Run `bash tools/mkstatus.sh` from the repository to count admitted functions,
replace the public README with Current Progress, commit only README.md and push
to the current branch's upstream. `--dry-run` prints without changing anything;
`--no-push` writes README locally without committing. Requires Python 3.9+ and Git.
The script does not build, score bytes, regenerate META status or stage other files.
It refuses to publish when HEAD differs from upstream or README is already staged.
If a push fails after committing, resolve the cause and push that commit manually.

`functions.tsv` is a frozen **estimated** game-function denominator generated from
the original EXE and the fresh `nm/fns_all.txt`, range 0x401000–0x47e600 (end
exclusive). It excludes QvLib, CRT, zero-size entries and heuristic small EH cleanup
funclets. The heuristic is not a verified boundary audit. Correct this inventory
with evidence when boundaries are verified; never silently reuse old score totals.
Reference EXE SHA256:
801931afc6ca09fdfffafb9c2f7223b2417b5c3895f57005091a817f3ec15194.

`accepted.tsv` has one row per admitted original function: original address in hex,
repository-relative source path under src/, and repository-relative evidence file.
Add a row only after semantic review and the applicable compile/link/behavior gate;
remove it if admission is revoked. Both files must exist. The counter validates
identities and paths, not the truth of the review: evidence remains a review duty.
Multiple functions in a source file have separate rows. Do not list demos, stubs,
archived code or optional SDL3 helpers as reconstructed original functions.

This is function coverage, not a claim that the game launches or is playable.
