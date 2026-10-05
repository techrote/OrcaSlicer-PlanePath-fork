#!/usr/bin/env python3
"""Verify CRC, membership, safe paths, declared sizes and SHA-256 of this packet."""
import hashlib, json, sys, zipfile
from pathlib import PurePosixPath

def verify(path):
 with zipfile.ZipFile(path) as z:
  if z.testzip() is not None: raise ValueError('ZIP CRC failure')
  names=z.namelist()
  if len(names)!=len(set(names)):raise ValueError('Duplicate archive members')
  for n in names:
   p=PurePosixPath(n)
   if p.is_absolute() or '..' in p.parts or '\\' in n:raise ValueError('Unsafe member '+n)
  manifest={}
  for line in z.read('MANIFEST.sha256').decode('utf-8').splitlines():
   digest,name=line.split('  ',1)
   if name in manifest:raise ValueError('Duplicate manifest member')
   manifest[name]=digest
  if set(names)!=set(manifest)|{'MANIFEST.sha256'}:raise ValueError('Manifest membership mismatch')
  for name,digest in manifest.items():
   data=z.read(name)
   if len(data)!=z.getinfo(name).file_size:raise ValueError('Size mismatch '+name)
   if hashlib.sha256(data).hexdigest()!=digest:raise ValueError('SHA256 mismatch '+name)
  return {'archive_members':len(names),'sha256_records':len(manifest),'crc':'pass','membership':'pass','sha256':'pass','paths':'pass'}
if __name__=='__main__':
 try:print(json.dumps(verify(sys.argv[1]),indent=2))
 except (IndexError,ValueError,KeyError,zipfile.BadZipFile) as e:raise SystemExit(str(e))
