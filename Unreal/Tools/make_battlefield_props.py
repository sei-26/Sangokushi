"""Original field fortifications, cm/Z-up. Keep existing hand-edited OBJ files."""
from pathlib import Path
import runpy
import math

Mesh=runpy.run_path(str(Path(__file__).resolve().parent/'make_battle_meshes.py'))['Mesh']

stake=Mesh()
bottom=[(4*math.cos(i*math.tau/8),4*math.sin(i*math.tau/8),0) for i in range(8)]
top=[(x,y,51+(i%2)*2) for i,(x,y,z) in enumerate(bottom)]
for i in range(8):
    j=(i+1)%8
    stake.face((bottom[i],bottom[j],top[j],top[i]))
    stake.face((top[i],top[j],(0,0,63)))
stake.face(tuple(reversed(bottom)))
stake.save('SM_PalisadeStakeV4')

tower=Mesh()
for x in (-20,20):
    for y in (-20,20):
        tower.box((x,y,52),(4,4,104))
for i in range(7): tower.box((-21+i*7,0,84),(6,49,4))
for side in (-1,1):
    for z in (99,112):
        tower.box((side*22,0,z),(3,48,3))
        tower.box((0,side*22,z),(48,3,3))
    tower.box((side*8,-26,38),(3,3,76))
for i in range(9): tower.box((0,-27,7+i*8),(17,3,2))
tower.save('SM_WatchtowerV4')

tent=Mesh()
# The front is open: a visible doorway and overlapping rolled-back cloth flaps.
for side in (-1,1):
    tent.face(((side*35,-30,4),(side*35,30,4),(0,30,59),(0,-30,59)))
tent.face(((-35,30,4),(35,30,4),(0,30,59)))
tent.face(((-35,-30,4),(-12,-30,4),(0,-30,59)))
tent.face(((12,-30,4),(35,-30,4),(0,-30,59)))
tent.save('SM_CampTentV4')

crate=Mesh()
for i in range(5):
    crate.box((-16+i*8,0,14),(7.5,30,27))
for side in (-1,1):
    for z in (4,24): crate.box((0,side*16,z),(43,3,4))
    for x in (-17,17): crate.box((x,0,29),(4,34,3))
crate.save('SM_SupplyCrateV4')
