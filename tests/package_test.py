"""Independent structural checks; does not claim retail signatures or runtime tests."""
from pathlib import Path
import struct,hashlib,sys

root=Path(__file__).resolve().parents[1]
def u32(data,offset):return struct.unpack_from('<I',data,offset)[0]
def align(value):return (value+63)&~63
def digest(data):return hashlib.sha256(data).digest()
smdh=(root/'folio.smdh').read_bytes()
assert smdh[:4]==b'SMDH'
for language in range(12):
    start=8+language*512
    author=smdh[start+384:start+512].decode('utf-16le').rstrip('\0')
    title=smdh[start:start+128].decode('utf-16le').rstrip('\0')
    if title:assert author=='Faefolk333',(language,author)
x=(root/'folio.3dsx').read_bytes()
assert x[:4]==b'3DSX'
offset,size=struct.unpack_from('<II',x,32)
assert x[offset:offset+size]==smdh

cia=(Path(sys.argv[1]) if len(sys.argv)>1 else root/'folio.cia').read_bytes()
header,_,_,cert,ticket,tmd_size,footer,content_size=struct.unpack_from('<IHHIIIIQ',cia)
tmd_offset=align(header)+align(cert)+align(ticket)
tmd=cia[tmd_offset:tmd_offset+tmd_size]
assert struct.unpack_from('>I',tmd)[0]==0x10004
title_id=struct.unpack_from('>Q',tmd,0x140+0x4c)[0]
version,count=struct.unpack_from('>HH',tmd,0x140+0x9c)
assert title_id==0x000400000ff04100,hex(title_id)
assert count==1,count
record=tmd[0xb04:0xb34]
declared=struct.unpack_from('>Q',record,8)[0]
offset=tmd_offset+align(tmd_size)
ncch=cia[offset:offset+declared]
assert len(ncch)==declared==content_size
assert digest(ncch)==record[16:48],'CIA content hash'
assert ncch[256:260]==b'NCCH'
assert struct.unpack_from('<Q',ncch,0x118)[0]==title_id
exheader=ncch[512:512+2048]
priority=exheader[0x20f]
assert 0x18<=priority<=0x3f,('invalid kernel thread priority',priority)
assert priority==48,('unexpected Folio thread priority',priority)
assert struct.unpack_from('<Q',exheader,0x1c8)[0]==title_id,'Jump ID'
assert struct.unpack_from('<Q',exheader,0x200)[0]==title_id,'ACI program ID'
assert struct.unpack_from('<Q',exheader,0x600)[0]==title_id,'Access descriptor ID'
exh_size=u32(ncch,0x180)
assert digest(ncch[512:512+exh_size])==ncch[0x160:0x180],'Exheader hash'
exefs_offset=u32(ncch,0x1a0)*512
exefs_size=u32(ncch,0x1a4)*512
exefs=ncch[exefs_offset:exefs_offset+exefs_size]
hash_size=u32(ncch,0x1a8)*512
assert digest(exefs[:hash_size])==ncch[0x1c0:0x1e0],'ExeFS header hash'
files={}
for index in range(10):
    entry=exefs[index*16:index*16+16]
    name=entry[:8].rstrip(b'\0')
    if not name:continue
    start,length=struct.unpack_from('<II',entry,8)
    data=exefs[512+start:512+start+length]
    assert len(data)==length
    assert digest(data)==exefs[0x1e0-index*32:0x200-index*32],name
    files[name]=data
assert b'logo' in files and len(files[b'logo'])>0,'Missing HOME launch logo'
assert version==7,version  # CIA installation revision; app version is 0.4.7.
assert files[b'icon']==smdh
assert files[b'banner'][:4]==b'CBMD'
assert b'.code' in files
print('PASS: SMDH authors, embedded 3DSX metadata, CIA launch priority/IDs/version, content/exheader/ExeFS hashes, icon and banner')
print('Homebrew test signatures are not Nintendo retail signatures; CIA requires CFW. Runtime not tested.')
