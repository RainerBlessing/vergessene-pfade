"""Original 16x16 Japanese-inspired tiles. Run to regenerate atlas.bmp."""
from pathlib import Path
import struct
# Ink, moss, jade, bamboo, straw, washi, cedar, ochre, indigo,
# water, slate, white, vermilion, sakura, skin, transparent.
pal=[(24,25,34),(36,57,47),(55,85,56),(104,135,75),(177,161,105),(234,219,176),(88,57,46),(168,112,62),(45,56,81),(72,116,135),(112,130,139),(247,238,210),(183,63,52),(219,146,162),(220,167,119),(255,0,255)]
w,h=56*16,16
pix=[[15]*w for _ in range(h)]
def rect(t,x,y,ww,hh,c):
 for yy in range(max(0,y),min(16,y+hh)):
  for xx in range(max(0,x),min(16,x+ww)):pix[yy][t*16+xx]=c
def shapes(t,rs):
 for r in rs:rect(t,*r)
def grass(t):
 rect(t,0,0,16,16,2)
 for x,y in [(1,3),(11,9),(5,13)]:rect(t,x,y,2,1,3)
for t in [0,3,4,9,10,11,19,20,24]:grass(t)
rect(1,0,0,16,16,4)
for x,y in [(2,3),(9,7),(4,13)]:rect(1,x,y,3,1,7)
rect(2,0,0,16,16,8)
for x,y in [(1,3),(9,7),(3,12)]:rect(2,x,y,6,1,9)
# Asymmetric layered pine and a snow-capped mountain.
shapes(3,[(7,6,2,10,6),(1,9,14,4,1),(2,8,10,3,3),(4,4,10,4,1),(5,3,8,3,3),(7,0,4,3,3)])
for y in range(2,16):rect(4,8-y//2,y,1+y,1,10 if y<8 else 8)
shapes(4,[(7,2,2,2,11),(6,4,4,2,11),(5,6,3,1,11),(9,6,2,1,11)])
# White plaster compound wall with charcoal roof tiles.
shapes(5,[(0,0,16,16,5),(0,0,16,4,8),(0,4,16,1,0),(0,13,16,3,10),(7,5,2,8,6)])
for x in range(0,16,4):rect(5,x,0,1,3,10)
# Shoji lattice, tatami, and split noren doorway.
shapes(6,[(0,0,16,16,6),(2,1,12,14,5)])
for x in [2,6,10,14]:rect(6,x,0,1,16,6)
for y in [4,8,12]:rect(6,0,y,16,1,6)
rect(7,0,0,16,16,4)
for y in range(2,15,3):rect(7,1,y,14,1,5)
rect(7,0,0,16,1,1);rect(7,0,0,1,16,1);rect(7,15,0,1,16,1)
shapes(8,[(0,0,16,16,6),(2,0,12,16,0),(2,0,5,8,8),(9,0,5,8,8),(4,2,1,3,5),(11,2,1,3,5),(2,14,12,2,4)])
# Torii: upturned black cap, two vermilion crossbeams and columns.
for t in [9,10]:
 shapes(t,[(0,0,2,2,0),(14,0,2,2,0),(1,1,14,2,0),(0,3,16,2,12),(3,5,2,11,12),(11,5,2,11,12),(2,7,12,2,12),(7,5,2,2,5)])
# Stone lantern.
shapes(11,[(3,14,10,2,10),(6,9,4,5,10),(4,5,8,5,8),(6,6,4,3,5),(2,4,12,2,10),(4,2,8,2,10),(7,0,2,2,10)])
# Traveller in indigo kimono and straw kasa, and villagers in distinct robes.
for t,robe,obi in [(12,8,12),(13,11,12),(14,3,4),(15,13,8),(16,8,4),(40,6,7)]:
 shapes(t,[(3,14,10,2,1),(5,13,2,2,6),(9,13,2,2,6),(4,6,8,8,robe),(3,7,2,4,robe),(11,7,2,4,robe),(5,2,6,5,14),(5,1,6,2,0),(6,4,1,1,0),(10,4,1,1,0),(6,7,1,2,5),(7,9,1,1,5),(4,10,8,2,obi)])
 if t==12:shapes(t,[(2,3,12,2,7),(3,2,10,1,4),(5,1,6,1,4),(7,0,2,1,5)])
 if t==13:shapes(t,[(4,1,2,6,0),(10,1,2,6,0),(4,12,8,2,12)])
 if t==15:shapes(t,[(7,0,4,2,0),(10,1,2,1,12)])
 if t==16:rect(t,12,3,1,12,6)
 if t==40:shapes(t,[(5,2,6,1,5),(12,5,1,10,6),(11,4,3,2,10)])
# Original thorn spirit, with branch antlers and a pale wooden mask.
shapes(17,[(3,5,10,9,1),(2,2,2,5,6),(1,1,2,2,6),(12,2,2,5,6),(13,1,2,2,6),(4,5,8,7,5),(5,7,2,2,12),(9,7,2,2,12),(7,10,2,1,0),(2,13,3,2,3),(11,13,3,2,3)])
shapes(18,[(5,2,6,12,7),(2,5,12,6,7),(4,4,8,8,4),(5,5,6,6,5),(6,5,2,2,11),(9,8,1,2,7)])
# Sakura, bamboo, flooded rice paddies, wooden bridge, tiled roof, lantern, gravel.
shapes(19,[(7,7,2,9,6),(4,9,4,2,6),(2,3,12,7,13),(4,1,8,11,13),(0,5,16,3,13)])
for x,y in [(4,2),(9,3),(2,6),(7,7),(12,6),(5,10)]:rect(19,x,y,2,2,5)
for x,y in [(2,14),(12,12)]:rect(19,x,y,1,1,13)
for x in [3,8,12]:
 rect(20,x,0,2,16,3)
 for y in [3,8,13]:rect(20,x,y,2,1,4)
shapes(20,[(0,4,4,2,1),(9,6,6,2,1),(4,10,5,2,1)])
rect(21,0,0,16,16,9)
for x in [2,7,12]:
 for y in [3,10]:shapes(21,[(x,y,1,4,3),(x-1,y-1,1,2,4),(x+1,y,1,2,3)])
rect(22,0,0,16,16,6)
for x in range(1,16,4):rect(22,x,1,3,14,7)
rect(22,0,0,16,2,4);rect(22,0,14,16,2,4)
rect(23,0,0,16,16,8)
for y in [1,5,9,13]:
 rect(23,0,y,16,1,10)
 for x in [1,5,9,13]:rect(23,x,y+1,1,2,0)
shapes(24,[(7,0,2,16,6),(3,2,10,2,0),(4,4,8,8,12),(5,5,6,6,5),(7,5,2,5,12),(5,12,6,1,0)])
rect(25,0,0,16,16,5)
for x,y in [(1,3),(7,8),(12,13)]:rect(25,x,y,2,1,4)
for t,flip in [(26,False),(27,True)]:
 grass(t)
 for y in range(2,16):
  start=max(0,8-y);end=16
  rect(t,0 if flip else start,y,end-start,1,10 if y%4==1 else 8)
# Walddorf tiles: boundary stone, stump, shrine, offering stone, old tree with a
# torn rope, mossy hollow, drag marks, tent, ledger table, woodpile, house mark, moss.
grass(28)
shapes(28,[(4,13,8,2,8),(5,3,6,11,10),(6,2,4,1,10),(5,3,2,3,3),(9,4,1,6,5),(6,7,3,1,0),(6,9,3,1,0)])
rect(29,0,0,16,16,1)
for x,y in [(2,3),(13,11),(11,1)]:rect(29,x,y,1,1,3)
shapes(29,[(3,13,2,1,6),(11,13,2,1,6),(4,8,8,6,6),(4,7,8,2,7),(6,7,4,1,4),(7,10,1,3,0)])
def gravel(t):
 rect(t,0,0,16,16,5)
 for x,y in [(1,3),(7,8),(12,13),(10,2),(3,12)]:rect(t,x,y,2,1,4)
gravel(30)
shapes(30,[(3,12,10,3,10),(4,6,8,6,6),(6,8,4,4,0),(3,2,10,1,8),(1,3,14,3,8),(7,6,2,2,11)])
gravel(31)
shapes(31,[(2,12,12,2,8),(3,7,10,6,10),(4,6,8,1,5),(5,9,6,1,8)])
rect(32,0,0,16,16,1)
shapes(32,[(0,0,16,6,2),(1,0,14,1,3),(5,4,6,12,6),(7,6,1,8,0),(5,8,2,1,4),(9,8,2,1,4),(10,9,1,3,4),(6,9,1,2,11),(3,14,3,2,6),(10,14,3,2,6)])
grass(33)
shapes(33,[(3,5,10,7,3),(4,6,8,5,1),(5,7,6,3,6)])
grass(34)
shapes(34,[(6,0,1,16,6),(9,0,1,16,6),(7,4,1,2,7),(8,11,1,2,7)])
grass(35)
for i,y in enumerate(range(2,15)):
 half=i//2+1
 rect(35,8-half,y,2*half,1,4 if i%3 else 7)
shapes(35,[(7,0,2,2,6),(7,9,2,6,0)])
grass(36)
shapes(36,[(2,8,12,3,6),(3,11,2,4,6),(11,11,2,4,6),(4,5,8,3,5),(7,5,2,3,12)])
gravel(37)
shapes(37,[(2,9,12,6,6),(3,10,3,2,7),(7,10,3,2,7),(11,10,3,2,7),(5,12,3,2,7),(9,12,3,2,7)])
shapes(38,[(0,0,16,16,6),(2,1,12,14,5)])
for x in [2,6,10,14]:rect(38,x,0,1,16,6)
for y in [4,8,12]:rect(38,0,y,16,1,6)
shapes(38,[(4,4,8,8,5),(5,5,6,1,0),(5,10,6,1,0),(5,5,1,6,0),(10,5,1,6,0),(7,7,2,2,0)])
rect(39,0,0,16,16,1)
for x,y in [(2,3),(9,6),(5,12),(13,11),(11,1)]:rect(39,x,y,1,1,3)
# Fox den with the wounded fox, the tended fox asleep, and paw prints.
for t in [41,42]:
 grass(t)
 shapes(t,[(1,6,14,10,6),(2,5,12,1,6),(4,9,8,7,0)])
shapes(41,[(5,11,7,4,7),(10,9,4,3,7),(10,8,1,1,7),(13,8,1,1,7),(12,10,1,1,0),(2,12,4,2,7),(2,12,1,2,11),(6,14,2,1,12)])
shapes(42,[(5,12,8,3,7),(10,11,3,3,7),(11,10,1,1,7),(3,12,3,2,7),(3,12,1,2,11),(7,13,3,1,11),(11,12,1,1,0)])
grass(43)
for x,y in [(2,2),(8,7),(3,12),(10,13)]:
 shapes(43,[(x,y+1,2,2,6),(x-1,y,1,1,6),(x+2,y,1,1,6),(x,y-1,1,1,6),(x+1,y-1,1,1,6)])
# Orihas shelf: the bowl while it dries, and with golden seams.
for t in [44,45]:
 rect(t,0,0,16,16,6)
 shapes(t,[(0,2,16,2,7),(0,9,16,2,7),(1,11,14,4,4)])
shapes(44,[(4,5,8,4,10),(5,4,6,1,10),(6,6,1,3,0),(9,6,1,2,0)])
shapes(45,[(4,5,8,4,5),(5,4,6,1,5),(6,6,1,3,7),(9,6,1,2,7),(7,5,1,4,7)])
# Boundary stake driven in beside the tracks.
grass(46)
shapes(46,[(7,2,2,13,6),(6,3,4,1,7),(5,14,6,1,4)])
# Futon, an empty den, a den with kits, a sapling, and the grey patch.
rect(47,0,0,16,16,4)
shapes(47,[(2,4,12,9,5),(2,4,12,2,11),(3,9,10,1,4),(1,3,14,1,6)])
grass(48)
shapes(48,[(1,6,14,10,6),(2,5,12,1,6),(4,9,8,7,0)])
grass(49)
shapes(49,[(1,6,14,10,6),(2,5,12,1,6),(4,9,8,7,0),(5,12,8,3,7),(10,11,3,3,7),(3,12,3,2,7),(3,12,1,2,11),(6,13,2,2,7),(8,13,2,2,7)])
grass(50)
shapes(50,[(7,8,2,7,6),(4,5,8,3,3),(6,3,4,3,3),(5,7,6,1,2)])
rect(51,0,0,16,16,10)
for x,y in [(3,4),(9,7),(5,12),(12,10)]:rect(51,x,y,2,1,5)
# Two signs beside their doors, and Orihas workbench with bowls and brushes.
for t,mark in [(52,'bed'),(53,'bowl')]:
 shapes(t,[(0,0,16,16,6),(1,1,14,10,4),(1,1,14,1,7),(1,10,14,1,7),(6,11,4,5,6)])
 if mark=='bed':shapes(t,[(3,4,10,4,5),(3,3,4,1,5),(3,8,10,1,7)])
 else:shapes(t,[(5,4,6,3,5),(4,3,8,1,5),(6,7,4,1,7),(11,3,1,5,12)])
shapes(54,[(0,0,16,16,6),(0,5,16,3,7),(1,8,14,7,4),(2,2,4,3,5),(3,1,2,1,5),(8,2,1,4,12),(8,1,1,1,0),(11,2,3,3,5),(12,1,1,1,11)])
# Deadwood stacked on forest ground.
grass(55)
shapes(55,[(2,9,12,6,6),(3,10,3,2,7),(7,10,3,2,7),(11,10,3,2,7),(5,12,3,2,7),(9,12,3,2,7)])
rows=bytearray()
for row in reversed(pix):
 for c in row:rows.extend(bytes(pal[c][::-1]))
 rows.extend(b'\0'*((-w*3)%4))
header=struct.pack('<2sIHHI',b'BM',54+len(rows),0,0,54)+struct.pack('<IiiHHIIiiII',40,w,h,1,24,0,len(rows),2835,2835,0,0)
Path(__file__).with_name('atlas.bmp').write_bytes(header+rows)
