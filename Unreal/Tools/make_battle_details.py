"""Original spear silhouette, editable OBJ in centimeters. Preserve existing art."""
from pathlib import Path
import runpy
import math

root = Path(__file__).resolve().parent
Mesh = runpy.run_path(str(root/'make_battle_meshes.py'))['Mesh']
blade = Mesh()
# A narrow socket, raised central spine and leaf-shaped steel blade.
for side in (-1, 1):
    tip = (0, 0, 50)
    spine = (side*6, 0, -5)
    left = (0, -17, -12)
    right = (0, 17, -12)
    root_point = (0, 0, -40)
    blade.face((tip, left, spine))
    blade.face((tip, spine, right))
    blade.face((left, root_point, spine))
    blade.face((right, spine, root_point))
blade.box((0,0,-38),(7,7,23))
blade.save('SM_SpearheadV2')

wheel = Mesh()
for i in range(24):
    for j in range(6):
        def point(segment,profile):
            angle=segment*math.tau/24
            section=profile*math.tau/6
            radius=42+8*math.cos(section)
            return (radius*math.cos(angle),8*math.sin(section),radius*math.sin(angle))
        wheel.face((point(i,j),point(i+1,j),point(i+1,j+1),point(i,j+1)))
for i in range(10):
    angle=i*math.tau/10
    direction=(math.cos(angle),math.sin(angle))
    side=(-math.sin(angle)*2.5,math.cos(angle)*2.5)
    p=[(direction[0]*r+side[0]*s,y,direction[1]*r+side[1]*s)
       for r,y,s in ((5,-4,-1),(42,-4,-1),(42,-4,1),(5,-4,1),(5,4,-1),(42,4,-1),(42,4,1),(5,4,1))]
    for ids in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):
        wheel.face([p[k] for k in ids])
wheel.box((0,0,0),(14,22,14))
wheel.save('SM_CartWheelV2')
