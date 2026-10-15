"""Test actual parser output without reproducing any user book content."""
import pathlib,subprocess,tempfile,zipfile,unicodedata,sys
runner=str(pathlib.Path('build/parser-test.exe').resolve())
sample='Příliš žluťoučký kůň úpěl ďábelské ódy. ĚŠČŘŽÝÁÍÉŮÚĎŤŇ'
def run(path):return subprocess.run([runner,str(path)],capture_output=True).stdout.decode('utf-8')
def epub(path,content):
    with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
        z.writestr('META-INF/container.xml','<container><rootfiles><rootfile full-path="book.opf"/></rootfiles></container>')
        z.writestr('book.opf','<package><manifest><item id="c" href="chapter.xhtml" media-type="application/xhtml+xml"/></manifest><spine><itemref idref="c"/></spine></package>')
        z.writestr('chapter.xhtml',content)
with tempfile.TemporaryDirectory() as directory:
    root=pathlib.Path(directory)
    for encoding,label in [('utf-8','UTF-8'),('utf-16','UTF-16'),('utf-16-be','UTF-16BE'),('cp1250','windows-1250'),('iso8859_2','iso-8859-2')]:
        payload=f'<?xml version="1.0" encoding="{label}"?><html><body><p>{sample}</p></body></html>'.encode(encoding)
        if encoding=='utf-16-be':payload=b'\xfe\xff'+payload
        path=root/'test.epub';epub(path,payload);assert sample in run(path),encoding
    for content in [unicodedata.normalize('NFD',sample),'P&rcaron;&iacute;li&scaron; &zcaron;lu&tcaron;ou&ccaron;k&yacute; k&uring;&ncaron; &uacute;p&ecaron;l &dcaron;&aacute;belsk&eacute; &oacute;dy.', 'P&#345;&#237;li&#353; &#382;lu&#357;ou&#269;k&#253; k&#367;&#328; &#250;p&#283;l &#271;&#225;belsk&#233; &#243;dy.']:
        path=root/'test.epub';epub(path,('<html><body><p>'+content+'</p></body></html>').encode());assert sample.split(' Ě')[0] in run(path)
    path=root/'legacy.txt';path.write_bytes(sample.encode('cp1250'));assert sample in run(path)
    path=root/'replacement.epub';epub(path,'<p>a\ufffdb</p>'.encode());assert 'a\ufffdb' in run(path)
    path=root/'invalid.epub';epub(path,b'<?xml encoding="UTF-8"?><p>\xff</p>');assert subprocess.run([runner,str(path)],capture_output=True).returncode!=0
print('PASS: Czech UTF8/UTF16 LE/BE, CP1250, ISO8859-2, HTML entities, decomposed accents, invalid data')
if len(sys.argv)>1:
    result=run(pathlib.Path(sys.argv[1]));assert result.startswith('TITLE:')
    print('User EPUB parsed; output replacement-character count (including headings):',result.count('\ufffd'))
