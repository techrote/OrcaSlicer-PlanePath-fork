#!/usr/bin/env python3
"""Rebuild and run the LOCAL 5.4.7 mechanism surrogate; never substitutes in Orca."""
import argparse, hashlib, json, platform, shutil, subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--out',type=Path,required=True);a=p.parse_args()
root=Path(__file__).resolve().parent;out=a.out.resolve();out.mkdir(parents=True,exist_ok=True)
lib=Path('/usr/lib/x86_64-linux-gnu/liblua5.4.so.0.0.0')
expected='4c51276430a3d9e130169bff107c441f12a4cfe0b75ebb59c73cda8ad9f622ba'
if platform.machine()!='x86_64' or not lib.exists() or hashlib.sha256(lib.read_bytes()).hexdigest()!=expected:
 raise SystemExit('Local ABI-shim probe requires the recorded Debian amd64 Lua library; use the pinned-header build on other systems.')
summary=[]
for name,compiler,flags in [('gcc','g++',['-O2']),('clang','clang++',['-O2']),('sanitized','clang++',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
 exe=out/('probe-'+name)
 cmd=[compiler,'-std=c++17','-Wall','-Wextra','-Wpedantic','-pthread',*flags,str(root/'probe.cpp'),str(lib),'-o',str(exe)]
 with (out/('build-'+name+'.log')).open('w') as log:
  built=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=45)
 if built.returncode:raise SystemExit('Compiler failed: '+name)
 ran=subprocess.run([str(exe),str(root/'fixtures')],capture_output=True,text=True,timeout=15,
                    env={**__import__('os').environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
 (out/('probe-'+name+'.jsonl')).write_text(ran.stdout);(out/('probe-'+name+'.stderr')).write_text(ran.stderr)
 rows=[json.loads(x) for x in ran.stdout.splitlines()];cases=[x for x in rows if 'case' in x]
 if ran.returncode or len(cases)!=33 or not all(x.get('pass') for x in cases):raise SystemExit('Probe failed: '+name)
 summary.append({'name':name,'compile_command':cmd,'compile_returncode':built.returncode,'run_returncode':ran.returncode,
  'cases':len(cases),'stderr_bytes':len(ran.stderr),'runtime_sha256':expected,'binary_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
  'coverage':'host wrapper instrumented; prebuilt Lua core NOT instrumented' if name=='sanitized' else 'local 5.4.7 surrogate'})
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
