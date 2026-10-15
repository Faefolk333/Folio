"""Integration fixtures for the actual C document parser; run in the MSYS shell."""
import pathlib, subprocess, tempfile, zipfile

runner = str(pathlib.Path("build/parser-test.exe").resolve())
container = '<container><rootfiles><rootfile full-path="OPS/book.opf"/></rootfiles></container>'
opf = """<package xmlns:dc="http://purl.org/dc/elements/1.1/">
<metadata><dc:title>Česká &amp; kniha</dc:title></metadata>
<manifest>
<item id="b" href="../Text/two.xhtml" media-type="application/xhtml+xml"/>
<item id="a" href="../Text/one%20chapter.xhtml" media-type="application/xhtml+xml"/>
</manifest><spine><itemref idref="a"/><itemref idref="b"/></spine></package>"""

def make(path, compression, package=opf, encrypted=False):
    with zipfile.ZipFile(path, "w", compression=compression) as z:
        z.writestr("mimetype", "application/epub+zip")
        z.writestr("META-INF/container.xml", container)
        z.writestr("OPS/book.opf", package)
        z.writestr("Text/two.xhtml", "<html><body><h1>Druhá kapitola</h1><p>Konec.</p></body></html>")
        z.writestr("Text/one chapter.xhtml", '<html><head><title>HIDDEN</title><style>HIDDEN</style></head><body><h1>První kapitola</h1><p>Příliš &#x17E;luťoučký &amp; kůň.</p><!-- ignored --><p>Druhý odstavec.</p></body></html>')
        if encrypted:
            z.writestr("META-INF/encryption.xml", "<encryption/>")

def run(path):
    return subprocess.run([runner, str(path)], capture_output=True, encoding="utf-8")

with tempfile.TemporaryDirectory() as directory:
    root = pathlib.Path(directory)
    for compression in (zipfile.ZIP_STORED, zipfile.ZIP_DEFLATED):
        path = root / "valid.epub"
        make(path, compression)
        result = run(path)
        assert result.returncode == 0, result.stderr
        assert "TITLE:Česká & kniha" in result.stdout
        assert "CHAPTERS:2" in result.stdout
        assert "Příliš žluťoučký & kůň." in result.stdout
        assert "HIDDEN" not in result.stdout
        body = result.stdout.split("CHAPTER:", 2)[-1]
        assert result.stdout.rfind("První kapitola") < result.stdout.rfind("Druhá kapitola")
    path = root / "encrypted.epub"
    make(path, zipfile.ZIP_DEFLATED, encrypted=True)
    assert run(path).returncode == 1
    path = root / "missing.epub"
    make(path, zipfile.ZIP_STORED, package=opf.replace('idref="b"', 'idref="missing"'))
    assert run(path).returncode == 1
    path = root / "truncated.epub"
    path.write_bytes(b"PK\x03\x04bad")
    assert run(path).returncode == 1
    path = root / "crc.epub"
    make(path, zipfile.ZIP_STORED)
    data = path.read_bytes().replace(b"Konec.", b"Xonec.")
    path.write_bytes(data)
    assert run(path).returncode == 1
print("PASS: stored/deflated EPUB, spine order, paths, Unicode/entities, chapters, encryption, missing item, truncation and CRC")
