"""Capture native draw calls with real BCFNT advances; approximate TTF rasterization.
This is visual layout QA, not emulation or a screenshot from 3DS hardware.
"""
import pathlib, subprocess
from PIL import Image, ImageDraw, ImageFont

ROOT = pathlib.Path(__file__).resolve().parents[1]
S = 3
def render(view, theme=1, focus=0,language=0,sample=None,bionic=0,font=3):
    args=[str(ROOT/'build/preview.exe'),str(view),str(theme),str(focus),str(language)]
    if sample is not None:args.extend(map(str,(sample,bionic,font)))
    output = subprocess.check_output(args,cwd=ROOT).decode('utf-8')
    screens = {}
    for line in output.splitlines():
        fields=line.split('\t')
        if fields[0]=='SCREEN':
            name=fields[1]; canvas=Image.new('RGBA',((400 if name=='top' else 320)*S,240*S));screens[name]=canvas;continue
        kind=fields[0];x,y,a,b=map(float,fields[1:5]);c=int(fields[5]);rgba=(c&255,(c>>8)&255,(c>>16)&255,(c>>24)&255)
        layer=Image.new('RGBA',canvas.size);draw=ImageDraw.Draw(layer)
        if kind=='T':
            filename={0:'NotoSans-Regular.ttf',1:'NotoSerif-Regular.ttf',3:'inter-regular.ttf',4:'ebgaramond-regular.ttf',5:'librebaskerville-regular.ttf'}[int(b)]
            font=ImageFont.truetype(str(ROOT/'assets/fonts'/filename),max(1,round(a*26.667*S)))
            draw.text((x*S,y*S),fields[6] if len(fields)>6 else '',font=font,fill=rgba,anchor='ls')
        else:
            box=(x*S,y*S,(x+a)*S,(y+b)*S)
            if kind=='R':draw.rectangle(box,fill=rgba)
            else:draw.ellipse(box,fill=rgba)
        canvas.alpha_composite(layer)
    return screens

if __name__=='__main__':
    sheet=Image.new('RGB',(400*S*3,510*S*2),'#303030')
    for v in range(6):
        screens=render(v,1,int(v==3));x=(v%3)*400*S;y=(v//3)*510*S
        sheet.paste(screens['top'].convert('RGB'),(x,y));sheet.paste(screens['bottom'].convert('RGB'),(x+40*S,y+250*S))
    sheet.save(ROOT/'build/ui-preview.png')
    print('Wrote build/ui-preview.png (native geometry; host rasterization)')
    sheet=Image.new('RGB',(400*S*3,510*S*2),'#303030')
    for language in range(2):
        for column,view in enumerate((4,3,6)):
            screens=render(view,1,0,language);x=column*400*S;y=language*510*S
            sheet.paste(screens['top'].convert('RGB'),(x,y));sheet.paste(screens['bottom'].convert('RGB'),(x+40*S,y+250*S))
    sheet.save(ROOT/'build/library-languages.png')
