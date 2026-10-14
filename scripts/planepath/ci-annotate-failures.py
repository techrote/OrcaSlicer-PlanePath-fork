#!/usr/bin/env python3
"""Turn unit-test failures into GitHub Actions error annotations.

CI logs and artifacts are not reachable from every agent environment, but
check-run annotations are (REST: /check-runs/<id>/annotations). This reads the
captured console output of the test step (Catch2 + ctest --output-on-failure)
and emits one ::error per failed assertion, plus the ctest list of failed
tests, so the failure reason is visible without downloading anything.

Usage: ci-annotate-failures.py <test-output.log>
Always exits 0; it only reports.
"""
import re
import sys

MAX_ANNOTATIONS = 9          # GitHub keeps 10 errors per step; last one is the overflow note
MAX_MESSAGE = 3500
RULE = re.compile(r"^-{20,}\s*$")
DOTS = re.compile(r"^\.{20,}\s*$")
EQUALS = re.compile(r"^={20,}\s*$")
FAILED_AT = re.compile(r"^(?P<path>.+?)[:(](?P<line>\d+)\)?:? (?:FAILED|failed):?\s*$")
CTEST_FAILED = re.compile(r"^\s*\d+\s+-\s+(?P<name>.+?)\s+\((?P<why>[^)]+)\)\s*$")
CRASH = re.compile(r"(SIGSEGV|SIGABRT|Segmentation fault|Exception: |terminate called|"
                   r"Subprocess aborted|\*\*\*Exception)")


def esc_data(s):
    return s.replace("%", "%25").replace("\r", "").replace("\n", "%0A")


def esc_prop(s):
    return esc_data(s).replace(":", "%3A").replace(",", "%2C")


def repo_relative(path):
    p = path.replace("\\", "/")
    for root in ("/tests/", "/src/", "/scripts/"):
        i = p.rfind(root)
        if i >= 0:
            return p[i + 1:]
    return None


def strip_ctest_prefix(line):
    # ctest -j prefixes captured output with "<n>: "
    return re.sub(r"^\d+: ", "", line.rstrip("\n"))


def parse(lines):
    failures, failed_tests, crashes = [], [], []
    test_case = ""
    i = 0
    in_failed_list = False
    while i < len(lines):
        line = lines[i]
        if RULE.match(line) and i + 1 < len(lines) and not RULE.match(lines[i + 1]):
            # Catch2 header: rule / test case [/ sections] / rule
            j = i + 1
            names = []
            while j < len(lines) and not RULE.match(lines[j]) and j - i < 8:
                names.append(lines[j].strip())
                j += 1
            if j < len(lines) and RULE.match(lines[j]) and names:
                test_case = " / ".join(n for n in names if n)
                i = j + 1
                continue
        if EQUALS.match(line):
            test_case = ""
        m = FAILED_AT.match(line)
        if m:
            body = []
            j = i + 1
            while j < len(lines) and len(body) < 14:
                nxt = lines[j]
                if (RULE.match(nxt) or DOTS.match(nxt) or EQUALS.match(nxt)
                        or FAILED_AT.match(nxt)):
                    break
                if nxt.strip() and not (nxt[0] in " \t" or nxt.startswith(("with ", "due to "))):
                    break
                body.append(nxt.rstrip())
                j += 1
            while body and not body[-1].strip():
                body.pop()
            failures.append((test_case, m.group("path"), m.group("line"), "\n".join(body)))
            i = j
            continue
        if "The following tests FAILED:" in line:
            in_failed_list = True
        elif in_failed_list:
            fm = CTEST_FAILED.match(line)
            if fm:
                failed_tests.append(f"{fm.group('name')} ({fm.group('why')})")
            elif line.strip():
                in_failed_list = False
        if CRASH.search(line) and len(crashes) < 5:
            crashes.append((test_case, line.strip()))
        i += 1
    return failures, failed_tests, crashes


def emit(title, message, path=None, line=None):
    props = [f"title={esc_prop(title[:200])}"]
    if path:
        props.append(f"file={esc_prop(path)}")
        if line:
            props.append(f"line={line}")
    print(f"::error {','.join(props)}::{esc_data(message[:MAX_MESSAGE])}")


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 0
    try:
        with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
            lines = [strip_ctest_prefix(l) for l in f]
    except OSError as e:
        emit("Test output missing", f"Could not read {sys.argv[1]}: {e}")
        return 0

    failures, failed_tests, crashes = parse(lines)
    emitted = 0
    if failed_tests:
        emit(f"{len(failed_tests)} test(s) failed", "\n".join(failed_tests))
        emitted += 1
    unique, seen = [], set()
    for f in failures:
        if f[:3] not in seen:
            seen.add(f[:3])
            unique.append(f)
    shown = 0
    for case, path, line, body in unique:
        if emitted >= MAX_ANNOTATIONS:
            break
        rel = repo_relative(path)
        emit(case or "Assertion failed", f"{path}:{line}\n{body}", rel, line if rel else None)
        emitted += 1
        shown += 1
    for case, text in crashes:
        if emitted >= MAX_ANNOTATIONS:
            break
        emit(f"Crash/exception: {case or 'unknown test'}", text)
        emitted += 1
    remaining = len(unique) - shown
    if not (failures or failed_tests or crashes):
        tail = "\n".join(l for l in lines[-25:] if l.strip())
        emit("Test step failed without a recognised test failure", tail or "(no output captured)")
    elif remaining > 0:
        emit("More failures", f"{remaining} further failure(s) not annotated; see the job log.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
