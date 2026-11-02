"""Original sculpted company parts; centimetres, +X forward, editable OBJ/MTL.
Versioned outputs preserve authored replacements. No downloaded models.
"""
from pathlib import Path
import math
import random

ROOT=Path(__file__).resolve().parents[1]/'ArtSource'/'Battle'

class Mesh:
    def __init__(self): self.faces=[]
    def face(self,points,material='Iron',smooth=False): self.faces.append((list(points),material,smooth))
    def rings(self,profiles,segments=24,material='Iron',fold=0):
        rings=[]
        for z,rx,ry in profiles:
            rings.append([(math.cos(a)*rx*(1+fold*math.cos(a*8)),math.sin(a)*ry*(1+fold*math.cos(a*8)),z) for a in (i*math.tau/segments for i in range(segments))])
        for a,b in zip(rings,rings[1:]):
            for i in range(segments):
                j=(i+1)%segments
                self.face((a[i],a[j],b[j],b[i]),material,True)
        self.face(reversed(rings[0]),material);self.face(rings[-1],material)
    def ellipsoid(self,center,radii,material):
        start=len(self.faces)
        self.rings([(math.sin(a)*radii[2],max(.001,math.cos(a))*radii[0],max(.001,math.cos(a))*radii[1]) for a in (-math.pi/2+i*math.pi/12 for i in range(13))],24,material)
        for f in range(start,len(self.faces)):
            p,m,s=self.faces[f];self.faces[f]=([tuple(v[k]+center[k] for k in range(3)) for v in p],m,s)
    def box(self,c,size,material):
        p=[tuple(c[k]+s[k]*size[k]/2 for k in range(3)) for s in ((-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1))]
        for ids in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)): self.face([p[i] for i in ids],material)
    def save(self,name):
        ROOT.mkdir(parents=True,exist_ok=True)
        path=ROOT/(name+'.obj')
        if path.exists(): return
        normals={};flat=[]
        for p,m,smooth in self.faces:
            a=[p[1][k]-p[0][k] for k in range(3)];b=[p[2][k]-p[0][k] for k in range(3)]
            n=(a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]);flat.append(n)
            if smooth:
                for v in p:
                    key=(m,tuple(round(x,5) for x in v));old=normals.get(key,(0,0,0));normals[key]=tuple(old[k]+n[k] for k in range(3))
        lines=['# Original Gunyu V10; centimetres, Z up, +X forward','mtllib '+name+'.mtl','o '+name];polygons=[];index=1
        for (p,m,smooth),n in zip(self.faces,flat):
            ids=[]
            for v in p:
                normal=normals[(m,tuple(round(x,5) for x in v))] if smooth else n
                length=max(1e-10,math.sqrt(sum(x*x for x in normal)))
                lines+=['v %.6f %.6f %.6f'%tuple(v),'vt %.6f %.6f'%(v[1]/100+.5,v[2]/100+.5),'vn %.6f %.6f %.6f'%tuple(x/length for x in normal)]
                ids.append(f'{index}/{index}/{index}');index+=1
            polygons+=['usemtl '+m,'f '+' '.join(ids)]
        path.write_text('\n'.join(lines+polygons)+'\n',encoding='utf-8')
        materials=sorted({m for _,m,_ in self.faces})
        (ROOT/(name+'.mtl')).write_text('\n'.join('newmtl '+m+'\nKd 0.3 0.3 0.3\nd 1.0\n' for m in materials),encoding='utf-8')

def generate():
    cloth=Mesh();cloth.rings([(-50,45,49),(-34,49,50),(0,50,47),(31,47,44),(50,39,38)],32,'Cloth',.045);cloth.save('SM_TailoredClothV10')
    head=Mesh();head.rings([(-50,9,13),(-40,23,29),(-26,32,37),(0,39,43),(25,37,40),(42,28,31),(50,4,5)],32,'Skin')
    for side in (-1,1):
        head.ellipsoid((0,side*43,0),(9,7,15),'Skin')
        head.ellipsoid((35,side*18,10),(3,7,2),'Features')
        head.box((35,side*18,18),(5,15,2.4),'Hair')
    head.face(((36,-6,12),(36,6,12),(53,0,-8)),'Skin')
    head.face(((36,-6,12),(53,0,-8),(37,-7,-12)),'Skin')
    head.face(((36,6,12),(37,7,-12),(53,0,-8)),'Skin')
    head.face(((37,-7,-12),(53,0,-8),(37,7,-12)),'Features')
    head.box((36,0,-25),(2,15,1.6),'Features');head.save('SM_InfantryHeadV10')
    armour=Mesh()
    for row in range(8):
        top=46-row*11.5;rx=34 if row<4 else 36+(row-4)*2;ry=45 if row<4 else 47+(row-4)*2
        for col in range(24):
            a=col*math.tau/24+.012;b=(col+1)*math.tau/24-.012
            p=[(math.cos(t)*r,math.sin(t)*q,z) for t,r,q,z in ((a,rx,ry,top),(b,rx,ry,top),(b,rx+1,ry+1,top-12),(a,rx+1,ry+1,top-12))]
            armour.face(reversed(p),'Iron');armour.face((p[0],p[1],tuple(v-(2 if k==2 else 0) for k,v in enumerate(p[1])),tuple(v-(2 if k==2 else 0) for k,v in enumerate(p[0]))),'Bronze')
    for z in (24,-16): armour.box((35,0,z),(5,74,5),'Leather')
    armour.box((40,0,-16),(5,13,9),'Bronze');armour.save('SM_LamellarArmorV10')
    helmet=Mesh();helmet.rings([(-6,50,49),(0,51,50),(7,48,47),(20,43,42),(34,33,32),(45,19,18),(50,2,2)],32,'Iron')
    helmet.rings([(-7,51,50),(-2,52,51),(2,51,50)],32,'Bronze')
    for side in (-1,1):
        helmet.face(((26,side*40,-4),(-35,side*40,-4),(-32,side*35,-32),(15,side*35,-36)),'Iron')
    helmet.face(((-38,-32,-5),(-38,32,-5),(-35,30,-35),(-35,-30,-35)),'Iron');helmet.save('SM_WarHelmetV10')
    beard=Mesh();beard.rings([(-50,2,4),(-32,12,22),(0,24,38),(33,28,43),(50,23,37)],24,'Hair',.1);beard.save('SM_CommandBeardV10')
    cape=Mesh()
    def point(u,v): return (-6-14*v+math.sin(u*math.pi*8)*(3+5*v),(u-.5)*(57+40*v),50-v*100+math.cos(u*math.pi*8)*v*3)
    for y in range(14):
        for x in range(24): cape.face((point(x/24,y/14),point((x+1)/24,y/14),point((x+1)/24,(y+1)/14),point(x/24,(y+1)/14)),'Cloth',True)
    cape.save('SM_CommandCapeV10')
    shield=Mesh()
    def edge(a,r): return (3,math.sin(a)*35*r,math.cos(a)*50*r)
    for i in range(40):
        a=i*math.tau/40;b=(i+1)*math.tau/40
        shield.face(((6,0,0),edge(b,.92),edge(a,.92)),'Wood',True)
        shield.face((edge(a,.92),edge(b,.92),edge(b,1),edge(a,1)),'Iron')
        shield.face((edge(a,1),edge(b,1),(-3,edge(b,1)[1],edge(b,1)[2]),(-3,edge(a,1)[1],edge(a,1)[2])),'Iron')
    for z in (-24,24): shield.box((-4,0,z),(3,57,5),'Leather')
    shield.ellipsoid((8,0,0),(7,11,12),'Bronze');shield.save('SM_InfantryShieldV10')
    canopy=Mesh();rng=random.Random(10184)
    for i in range(620):
        a=rng.uniform(0,math.tau);h=rng.uniform(-1,1);radius=rng.random()**(1/3)
        p=(math.cos(a)*math.sqrt(1-h*h)*radius*46,math.sin(a)*math.sqrt(1-h*h)*radius*46,h*radius*44)
        angle=rng.uniform(0,math.tau);length=rng.uniform(3.2,6);width=rng.uniform(1.1,2.2)
        u=(math.cos(angle),math.sin(angle),rng.uniform(-.3,.3));v=(-math.sin(angle),math.cos(angle),rng.uniform(-.3,.3))
        points=[tuple(p[k]+u[k]*length*side for k in range(3)) for side in (-1,1)]
        points += [tuple(p[k]+v[k]*width*side for k in range(3)) for side in (-1,1)]
        middle=(p[0],p[1],p[2]+.8)
        for a,b in ((0,2),(2,1),(1,3),(3,0)): canopy.face((points[a],points[b],middle),'Leaves')
    canopy.save('SM_FieldCanopyV10')

if __name__=='__main__': generate()
