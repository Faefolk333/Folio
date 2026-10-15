"""Report encoding statistics only; never print book content."""
import zipfile, re, collections, sys
with zipfile.ZipFile(sys.argv[1]) as z:
    files=[n for n in z.namelist() if n.lower().endswith(('.xhtml','.html','.htm','.opf'))]
    print('Content files:',len(files));print('Encryption manifest:', 'META-INF/encryption.xml' in z.namelist())
    enc=collections.Counter();entities=collections.Counter();numeric=collections.Counter();bad=[];combining=0;latin=collections.Counter()
    for n in files:
        b=z.read(n)
        enc.update(x.decode(errors='replace') for x in re.findall(rb'''(?:encoding|charset)=["']([^"']+)''',b[:1000]))
        entities.update(x.decode() for x in re.findall(rb'&([a-zA-Z]+);',b))
        numeric.update(int(x[1:],16) if x[:1].lower()==b'x' else int(x) for x in re.findall(rb'&#(x[0-9a-fA-F]+|[0-9]+);',b))
        try:
            s=b.decode('utf-8-sig');combining+=sum(0x300<=ord(c)<=0x36f for c in s)
            latin.update(c for c in s if 128<ord(c)<592)
        except UnicodeDecodeError:bad.append(n)
    print('Encodings:',enc);print('Named entities:',entities.most_common(30));print('Invalid UTF8 files:',bad[:10])
    print('Combining accents:',combining);print('Latin characters:',[(hex(ord(c)),n) for c,n in latin.most_common(40)])
    print('Numeric entities:',[(hex(c),n) for c,n in numeric.most_common(40)])
