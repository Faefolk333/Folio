"""Deterministic packaging assets using the existing owner-supplied icon.

No generated artwork: this only scales/composites that icon and typesets the
app/author names. A short silent PCM track avoids adding a banner sound.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import wave

root=Path(__file__).resolve().parents[1]
destination=root/'assets'/'cia'
destination.mkdir(parents=True,exist_ok=True)
banner=Image.new('RGBA',(256,128),'#191c20')
icon=Image.open(root/'assets/icon.png').convert('RGBA')
icon.thumbnail((86,86),Image.Resampling.LANCZOS)
banner.alpha_composite(icon,(12,21))
draw=ImageDraw.Draw(banner)
font=root/'assets/fonts/NotoSans-Regular.ttf'
draw.text((111,43),'Folio',font=ImageFont.truetype(str(font),28),fill='#f3e3c3')
banner.convert('RGB').save(destination/'banner.png')
with wave.open(str(destination/'silence.wav'),'wb') as sound:
    sound.setnchannels(1)
    sound.setsampwidth(2)
    sound.setframerate(32000)
    sound.writeframes(bytes(6400))
print('Built banner from supplied icon and silent WAV.')
