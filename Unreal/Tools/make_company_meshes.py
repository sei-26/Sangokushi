"""Original, versioned infantry armour and three commander weapon silhouettes.
Centimeters, Z up. Reuse the source writer's non-overwrite protection.
These are creative costumes inspired by the novel, not archaeological replicas.
"""
from pathlib import Path
import math
import runpy

Mesh = runpy.run_path(str(Path(__file__).with_name('make_battle_meshes.py')))['Mesh']

armour = Mesh()
for row in range(7):
    top = 44-row*13
    rx, ry = (32, 47) if row < 4 else (35+(row-4)*2, 48+(row-4)*2)
    for col in range(16):
        a = col*math.tau/16+.018
        b = (col+1)*math.tau/16-.018
        def corner(angle, z, inner=False):
            depth = -4 if inner else 0
            return (math.cos(angle)*(rx+depth), math.sin(angle)*(ry+depth), z)
        p = [corner(a, top), corner(b, top), corner(b, top-14), corner(a, top-14),
             corner(a, top, True), corner(b, top, True), corner(b, top-14, True), corner(a, top-14, True)]
        for ids in ((0,3,2,1),(0,1,5,4),(3,7,6,2),(0,4,7,3),(1,2,6,5)):
            armour.face([p[i] for i in ids])
for side in (-1,1):
    for rank in range(3):
        y = side*(43+rank*7)
        z = 39-rank*5
        armour.face(((-29,y,z),(29,y,z),(25,y+side*9,z-7),(-25,y+side*9,z-7)))
        armour.face(((-29,y,z),(-25,y+side*9,z-7),(-25,y+side*9,z-10),(-29,y,z-3)))
        armour.face(((29,y,z),(29,y,z-3),(25,y+side*9,z-10),(25,y+side*9,z-7)))
armour.save('SM_LamellarArmorV7')

sword = Mesh()
for side in (-1,1):
    ridge = (side*3,0,6)
    for a,b in (((0,-9,-18),(0,-8,34)),((0,-8,34),(0,0,50)),
                ((0,0,50),(0,8,34)),((0,8,34),(0,9,-18)),((0,9,-18),(0,-9,-18))):
        sword.face((a,b,ridge) if side==1 else (b,a,ridge))
sword.box((0,0,-21),(9,39,6))
sword.box((0,0,-35),(7,9,23))
sword.box((0,0,-48),(11,16,4))
sword.save('SM_CommandSwordV7')

glaive = Mesh()
outline = [(-3,-50),(-9,-28),(-9,18),(0,50),(20,38),(38,10),(31,-12),(11,-24),(7,-50)]
for side in (-1,1):
    for i in range(len(outline)):
        y,z = outline[i]
        by,bz = outline[(i+1)%len(outline)]
        points = ((0,y,z),(0,by,bz),(side*4,4,-1))
        glaive.face(points if side==1 else tuple(reversed(points)))
glaive.box((0,0,-43),(9,10,14))
glaive.save('SM_CommandGlaiveV7')

serpent = Mesh()
for i in range(12):
    z = -38+i*88/12
    end = -38+(i+1)*88/12
    def edge(h,side):
        taper = max(0,(50-h)/88)
        y = math.sin((h+38)/88*math.pi*3)*9*taper
        return (0,y+side*7*min(1,taper*4),h)
    for side in (-1,1):
        center = (side*3,math.sin((z+38)/88*math.pi*3)*6,z)
        finish = (side*3*(50-end)/88,math.sin((end+38)/88*math.pi*3)*6*(50-end)/88,end)
        for a,b,c,d in ((edge(z,-1),edge(end,-1),finish,center),(center,finish,edge(end,1),edge(z,1))):
            serpent.face((a,b,c,d) if side==1 else (d,c,b,a))
serpent.box((0,0,-44),(8,8,12))
serpent.save('SM_CommandSerpentV7')
