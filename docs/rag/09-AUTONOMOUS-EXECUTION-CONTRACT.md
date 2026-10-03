# Autonomous issue execution contract

Every PP implementation issue incorporates this document by reference.

## Start-of-issue protocol

1. Read the issue completely.
2. Read `docs/rag/INDEX.md`, `00-PROJECT-BRIEF.md`, `06-TEST-CI-STRATEGY.md`, `08-REVISED-ROADMAP.md`, this contract, and every additional RAG doc named by the issue.
3. Inspect current `main`, open/closed issues and open/merged PRs relevant to the task. Do not redo work that already landed.
4. Verify dependency issues are genuinely satisfied on `main`, not merely closed.
5. Record/inspect the current Orca upstream/base commit for source-touching work.
6. Create a dedicated branch named `pp-NNN-short-description` or an equivalently traceable name.

## Implementation rules

- Implement the acceptance criteria, not merely the title.
- Preserve project invariants and upstream semantics unless the issue explicitly changes them.
- Search globally for affected enum/helper/config references; do not patch only the first search hit.
- Add/update automated tests with the implementation.
- Do not bypass the sandbox, config identity model or native geometry pipeline for expediency.
- Do not introduce silent fallbacks.
- Do not disable, weaken or skip tests/CI to obtain a green run.
- Keep unrelated cleanup out of the PR unless it is required to complete the issue safely.
- Update RAG docs in the same PR when implementation discoveries make them materially stale.

## Validation protocol

Before merge:

1. run focused tests for the changed subsystem;
2. run all repository-required automated checks/builds applicable to the change;
3. inspect failures and repair root causes;
4. if upstream/test infrastructure is broken independently, document concrete evidence and block rather than falsely declaring success.

A rerun is not a repair unless evidence shows the prior failure was external/flaky and the project has an accepted policy for it.

## PR protocol

Open a PR linked to the issue.

PR description must include:

- `Closes #<issue>` only when merge will genuinely satisfy the issue;
- summary of implementation;
- files/subsystems materially changed;
- tests/checks and results;
- upstream base/provenance relevant to the work;
- deviations from the plan and why;
- known limitations that remain within acceptance criteria.

Prefer reviewable commits; do not hide a large implementation behind generated/vendor noise without explanation.

## Merge gate

Merge **only after required automated checks pass**.

If repository settings do not technically enforce branch protection, the agent must still inspect check state and apply this rule manually.

Use the repository's accepted merge strategy. Do not force-push/merge around a failing gate.

## Post-merge verification

After merge:

1. verify the PR reports merged;
2. verify the resulting commit/change is reachable from current `main`;
3. verify required post-merge/main checks if the repository uses them;
4. confirm acceptance criteria are still true on `main`;
5. only then treat the issue as complete/closed.

If GitHub auto-closes through `Closes #...`, reopen or record a follow-up blocker if acceptance criteria were not actually satisfied.

## Blocked-work protocol

If genuinely blocked:

- record the exact blocker, commands/checks, logs/links and attempted fixes in the issue/PR;
- leave partial work in a reviewable branch/PR when useful;
- do **not** merge a knowingly incomplete implementation merely to advance the dependency graph;
- stop at the blocker rather than inventing evidence.

## Security-sensitive changes

For PP-005, PP-006, PP-014, PP-015 and PP-016:

- include negative/adversarial tests;
- identify new trust boundaries in the PR;
- do not expand host capabilities beyond the issue without explicit design documentation;
- treat project-carried script source as untrusted until explicit trust/import.

## Licence/provenance

For external algorithm/runtime code:

- record source URL/project, version/commit and licence;
- include required notices;
- distinguish mathematical inspiration/reference from copied implementation;
- do not transplant code whose licence compatibility has not been established.

## Issue completion statement

A final issue comment or PR summary should state, with evidence:

- acceptance criteria satisfied;
- automated checks passed;
- PR merged;
- merge verified on `main`;
- any intentionally deferred work and its issue reference.
