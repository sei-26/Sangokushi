"""Reproducible original meshes. Centimeters, Z up; no external model downloads.
The resulting OBJ sources remain editable/importable in Unreal or any DCC.
"""
import math
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / 'ArtSource' / 'Battle'

class Mesh:
    def __init__(self):
        self.vertices, self.faces = [], []

    def face(self, points):
        start = len(self.vertices) + 1
        self.vertices.extend(points)
        self.faces.append(tuple(range(start, start + len(points))))

    def box(self, center, scale):
        x, y, z = center
        a, b, c = (value / 2 for value in scale)
        v = [(x + i*a, y + j*b, z + k*c) for i, j, k in
             ((-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1))]
        for ids in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):
            self.face([v[i] for i in ids])

    def save(self, name):
        ROOT.mkdir(parents=True, exist_ok=True)
        if (ROOT / (name + '.obj')).exists():
            return  # Preserve hand-edited DCC sources; version new variants instead.
        lines = ['# Original Gunyu mesh; centimeters, Z up', 'o ' + name]
        lines += ['v %.5f %.5f %.5f' % p for p in self.vertices]
        lines += ['vt %.5f %.5f' % (p[1] / 100 + .5, p[2] / 100 + .5) for p in self.vertices]
        lines += ['s off']
        lines += ['f ' + ' '.join('%d/%d' % (i, i) for i in f) for f in self.faces]
        (ROOT / (name + '.obj')).write_text('\n'.join(lines) + '\n', encoding='utf-8')

def create():
    rng = random.Random(184)
    leaves = Mesh()
    for i in range(180):
        # Distributed foliage, with individual folded leaf planes and open gaps.
        az = rng.uniform(0, math.tau)
        h = rng.uniform(-1, 1)
        r = rng.random() ** (1/3)
        d = math.sqrt(1-h*h)
        p = (math.cos(az)*d*r*44, math.sin(az)*d*r*44, h*r*40)
        angle = rng.uniform(0, math.tau)
        u = (math.cos(angle), math.sin(angle), rng.uniform(-.4,.4))
        v = (-math.sin(angle), math.cos(angle), rng.uniform(-.2,.2))
        length, width = rng.uniform(7,15), rng.uniform(3.0,6.0)
        a = tuple(p[k] - u[k]*length for k in range(3))
        b = tuple(p[k] + u[k]*length for k in range(3))
        c = tuple(p[k] + v[k]*width for k in range(3))
        d = tuple(p[k] - v[k]*width for k in range(3))
        spine = (p[0],p[1],p[2]+2.5)
        for t in ((a,c,spine),(c,b,spine),(b,d,spine),(d,a,spine)):
            leaves.face(t)
    leaves.save('SM_LeafCrown')

    rock = Mesh()
    rings = []
    for level, radius in ((-40,25),(-16,48),(18,43),(46,18)):
        ring = []
        for i in range(9):
            a = i*math.tau/9
            r = radius*rng.uniform(.8,1.15)
            ring.append((math.cos(a)*r,math.sin(a)*r,level+rng.uniform(-8,8)))
        rings.append(ring)
    for j in range(3):
        for i in range(9):
            k=(i+1)%9
            rock.face((rings[j][i], rings[j][k], rings[j+1][k]))
            rock.face((rings[j][i], rings[j+1][k], rings[j+1][i]))
    rock.face(list(reversed(rings[0]))); rock.face(rings[-1])
    rock.save('SM_RockOutcrop')

    armor = Mesh()
    # Shaped chest, shoulder guards, individual overlapping lamellar rows.
    for row in range(6):
        z=39-row*15
        width=85 if row<2 else 70 if row<4 else 90
        for column in range(6):
            y=(column-2.5)*width/6
            x=24 + (1-(y/(width*.6))**2)*8
            armor.box((x,y,z),(10,width/6-1,17))
            armor.box((-x,y,z),(8,width/6-1,17))
    armor.box((0,-47,32),(62,18,24)); armor.box((0,47,32),(62,18,24))
    armor.box((0,0,-12),(72,93,7))
    armor.save('SM_LamellarArmor')

    helmet=Mesh()
    rings=[]
    for j in range(7):
        a=j*math.pi/12
        rings.append([(math.cos(i*math.tau/16)*50*math.cos(a),math.sin(i*math.tau/16)*50*math.cos(a),-6+50*math.sin(a)) for i in range(16)])
    for j in range(6):
        for i in range(16):
            k=(i+1)%16
            helmet.face((rings[j][i],rings[j][k],rings[j+1][k],rings[j+1][i]))
    # Neck guard and brim; front looks along +X.
    for i in range(14):
        a=(i+1)*math.tau/16
        b=(i+2)*math.tau/16
        helmet.face(((math.cos(a)*49,math.sin(a)*49,-5),(math.cos(b)*49,math.sin(b)*49,-5),
                     (math.cos(b)*54,math.sin(b)*54,-38),(math.cos(a)*54,math.sin(a)*54,-38)))
    helmet.box((26,0,-4),(56,103,6))
    helmet.save('SM_WarHelmet')

    shield=Mesh()
    # Bowed rectangular shield: a raised ridge and iron edge are real geometry.
    for side in (-1,1):
        shield.face(((0,0,-50),(0,0,50),(-12,side*43,45),(-12,side*43,-43)))
        shield.box((-10,side*42,0),(10,5,92))
    shield.box((-6,0,47),(13,87,6)); shield.box((-6,0,-47),(13,87,6))
    shield.box((3,0,0),(6,8,97))
    shield.save('SM_InfantryShield')

    grass=Mesh()
    for i in range(9):
        a=rng.uniform(0,math.tau)
        x,y=math.cos(a)*rng.uniform(0,26),math.sin(a)*rng.uniform(0,26)
        side=(math.cos(a)*6,math.sin(a)*6)
        bend=(-math.sin(a)*rng.uniform(8,22),math.cos(a)*rng.uniform(8,22))
        tip=(x+bend[0],y+bend[1],rng.uniform(20,50))
        left=(x-side[0],y-side[1],-50);right=(x+side[0],y+side[1],-50)
        mid=(x+bend[0]*.3,y+bend[1]*.3,0)
        grass.face((left,right,(mid[0]+side[0]*.55,mid[1]+side[1]*.55,0),(mid[0]-side[0]*.55,mid[1]-side[1]*.55,0)))
        grass.face(((mid[0]-side[0]*.55,mid[1]-side[1]*.55,0),(mid[0]+side[0]*.55,mid[1]+side[1]*.55,0),tip))
    grass.save('SM_GrassClump')

    roof=Mesh()
    def roof_point(x,y):
        return (x,y,28-abs(x)*.70+y*y*.006)
    for row in range(12):
        for column in range(8):
            x,y=-50+column*12.5,-50+row*(100/12)
            roof.face((roof_point(x,y),roof_point(x+12.5,y),roof_point(x+12.5,y+100/12),roof_point(x,y+100/12)))
            if column in (0,7):
                p=roof_point(x if column==0 else x+12.5,y)
                q=roof_point(x if column==0 else x+12.5,y+100/12)
                roof.face((p,(p[0],p[1],p[2]-8),(q[0],q[1],q[2]-8),q))
    roof.box((0,0,30),(6,108,8))
    roof.save('SM_GateRoof')

if __name__ == '__main__':
    create()
