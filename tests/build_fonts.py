"""Compile redistributable fonts; run from project root with fontTools installed."""
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
import subprocess
for name in ('inter','ebgaramond','librebaskerville'):
    font=TTFont(f'assets/fonts/{name}-variable.ttf')
    axes={a.axisTag:(400 if a.axisTag=='wght' else a.defaultValue) for a in font['fvar'].axes}
    font=instantiateVariableFont(font,axes,inplace=True)
    if name=='librebaskerville':
        # The original has a Reserved Font Name; use a distinct derivative name.
        for record in font['name'].names:
            if record.nameID in (1,2,3,4,6,16,17):
                value={1:'Folio Baskerville',2:'Regular',3:'FolioBaskerville-Regular',4:'Folio Baskerville Regular',6:'FolioBaskerville-Regular',16:'Folio Baskerville',17:'Regular'}[record.nameID]
                record.string=value.encode(record.getEncoding())
    path=f'assets/fonts/{name}-regular.ttf';font.save(path)
    subprocess.run(['C:/devkitPro/msys64/opt/devkitpro/tools/bin/mkbcfnt.exe','-s','20','-w','assets/fonts/codepoints.txt','-o',f'assets/fonts/{name}.bcfnt',path],check=True)
