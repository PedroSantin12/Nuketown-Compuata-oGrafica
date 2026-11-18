from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
assets=Path(__file__).resolve().parent
assets.mkdir(parents=True,exist_ok=True)
fontdir=Path(r'C:\Windows\Fonts')
def font(size,bold=False):
    path=fontdir/('arialbd.ttf' if bold else 'arial.ttf')
    if not path.exists(): path=Path(__file__).resolve().parents[4]/'Fonts/arial.ttf'
    return ImageFont.truetype(str(path),size)
def centered(draw,text,y,f,fill,width=2048):
    bbox=draw.textbbox((0,0),text,font=f);draw.text(((width-(bbox[2]-bbox[0]))/2,y),text,font=f,fill=fill)
# These are code-authored lettering assets, kept with the project for reproducibility.
im=Image.new('RGB',(2048,832),(225,216,178));d=ImageDraw.Draw(im)
d.rounded_rectangle((12,12,2036,820),radius=55,outline=(41,78,74),width=16)
d.line((90,204,1958,204),fill=(156,66,47),width=8)
centered(d,'WELCOME TO',58,font(96,True),(48,78,70))
centered(d,'NUKETOWN',220,font(245,True),(153,49,35))
d.line((160,553,1888,553),fill=(48,78,70),width=5)
centered(d,'POPULATION: 0',589,font(104,True),(48,65,57))
centered(d,'A PICTURE-PERFECT COMMUNITY',734,font(40),(87,91,72))
im.resize((1024,416),Image.Resampling.LANCZOS).save(assets/'NUKETOWN.BMP')
im=Image.new('RGB',(2048,512),(212,213,194));d=ImageDraw.Draw(im)
d.rectangle((14,14,2033,497),outline=(37,45,40),width=10)
centered(d,'MILITARY POLICE',108,font(175,True),(32,39,35))
centered(d,'U.S. ARMY',344,font(66),(49,57,49))
im.resize((1024,256),Image.Resampling.LANCZOS).save(assets/'POLICE.BMP')
im=Image.new('RGB',(2048,1024),(224,218,190));d=ImageDraw.Draw(im)
for y in [40,66,936,962]:d.line((0,y,2048,y),fill=(132,47,34),width=9)
centered(d,'WESTWARD',117,font(190,True),(131,49,35))
centered(d,'MOVING &',399,font(155,True),(113,48,36))
centered(d,'DELIVERY',575,font(155,True),(113,48,36))
centered(d,'CAREFUL HANDLING - COAST TO COAST',843,font(49,True),(97,80,57))
im.resize((1024,512),Image.Resampling.LANCZOS).save(assets/'DELIVERY.BMP')
# Keep the generator alongside its lettering outputs; it is not used by the executable.
print('Three original text labels generated:',assets)
