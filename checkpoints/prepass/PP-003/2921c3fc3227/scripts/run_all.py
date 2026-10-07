#!/usr/bin/env python3
import argparse,subprocess,sys
from pathlib import Path
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--out',type=Path,default=R/'results');a=p.parse_args();a.out=a.out.resolve();a.out.mkdir(parents=True,exist_ok=True)
for s in ['check_decisions.py','check_hilbert.py','check_edge_cases.py']:
    subprocess.run([sys.executable,str(R/'scripts'/s),'--out',str(a.out)],check=True,timeout=45)
subprocess.run([sys.executable,str(R/'scripts/scan_repository.py'),'--self-test','--out',str(a.out/'scan_selftest.json')],check=True,timeout=10)
print('ALL OFFLINE PREPASS CHECKS PASSED; native Orca tests were NOT executed.')
