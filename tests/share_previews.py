"""Reproducible publication previews from native draw calls; NOT device screenshots."""
from preview import ROOT, render
from PIL import Image, ImageDraw, ImageFont

destination=ROOT/'sharing'/'previews'
destination.mkdir(parents=True,exist_ok=True)
caption_font=ImageFont.truetype(str(ROOT/'assets/fonts/NotoSans-Regular.ttf'),18)
scenes=[
 ('01-library', 'Your library',4,1,0,0,0,0,3),
 ('02-reading-paper', 'Alice / Paper',0,2,0,0,0,0,1),
 ('03-display-themes', 'Five reading themes',1,1,0,0,0,0,3),
 ('04-type', 'Pride and Prejudice / Type',2,0,0,0,1,0,4),
 ('05-settings', 'Frankenstein / Settings',3,3,1,0,2,1,3),
 ('06-bionic-focus', 'Bionic Reading + Focus Line',0,3,1,0,0,1,3),
 ('07-red-night-jump', 'Red Night / Go to',5,4,0,0,2,0,1),
 ('08-czech', 'Máj / Czech interface',0,1,0,1,3,0,1),
]
contact=Image.new('RGB',(800,4*520),'#171a1e')
for index,(name,caption,*args) in enumerate(scenes):
    screens=render(*args)
    image=Image.new('RGB',(800,1040),'#171a1e')
    image.paste(screens['top'].convert('RGB').resize((800,480),Image.Resampling.LANCZOS),(0,0))
    image.paste(screens['bottom'].convert('RGB').resize((640,480),Image.Resampling.LANCZOS),(80,490))
    draw=ImageDraw.Draw(image)
    draw.text((18,982),caption,font=caption_font,fill='#f1f2f3')
    draw.text((18,1010),'Folio 0.4.7 · Faefolk333 · Host-rendered preview',font=caption_font,fill='#a0a7ae')
    image.save(destination/(name+'.png'))
    contact.paste(image.resize((400,520),Image.Resampling.LANCZOS),((index%2)*400,(index//2)*520))
contact.save(destination/'overview.png')
print('Wrote 8 clearly labelled native-code previews and overview.png')
