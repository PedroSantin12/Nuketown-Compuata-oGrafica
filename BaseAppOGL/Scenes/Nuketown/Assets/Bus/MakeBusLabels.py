from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
assets=Path(__file__).resolve().parent
image=Image.new('RGB',(1024,128),(237,174,41))
draw=ImageDraw.Draw(image)
font_path=Path('C:/Windows/Fonts/arialbd.ttf')
if not font_path.exists(): font_path=assets.parents[2]/'Fonts'/'arial.ttf'
font=ImageFont.truetype(str(font_path),100)
label='SCHOOL BUS'
box=draw.textbbox((0,0),label,font=font)
draw.text(((1024-box[2])/2,(128-(box[3]-box[1]))/2-box[1]),label,font=font,fill=(24,28,27))
image.save(assets/'BUSLABEL.BMP')
