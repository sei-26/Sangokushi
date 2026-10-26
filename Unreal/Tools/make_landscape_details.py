"""Original weathered boulder; editable OBJ, centimeters, deterministic."""
from pathlib import Path
import math

root=Path(__file__).resolve().parents[1]/'ArtSource'/'Battle'
path=root/'SM_WeatheredBoulderV5.obj'
if not path.exists():
    vertices=[]
    rings,segments=10,20
    for j in range(rings+1):
        phi=-math.pi/2+j*math.pi/rings
        for i in range(segments+1):
            theta=i*math.tau/segments
            # Broad fracture planes plus finer erosion; no random regeneration.
            shape=1+.085*math.sin(theta*3+phi*4)+.045*math.cos(theta*7-phi*5)
            r=math.cos(phi)*shape
            x=49*r*math.cos(theta)+6*math.sin(phi*2)
            y=43*r*math.sin(theta)
            z=44*math.sin(phi)+3*math.sin(theta*4)*math.cos(phi)**2
            vertices.append((x,y,z))
    lines=['# Original Gunyu weathered boulder, cm, Z up','o SM_WeatheredBoulderV5']
    lines += ['v %.5f %.5f %.5f'%v for v in vertices]
    lines += ['vt %.6f %.6f'%(i/segments,j/rings) for j in range(rings+1) for i in range(segments+1)]
    lines += ['s 1']
    for j in range(rings):
        for i in range(segments):
            a=j*(segments+1)+i+1;b=a+1;c=a+segments+2;d=c-1
            for tri in ((a,b,c),(a,c,d)):
                lines.append('f '+' '.join('%d/%d'%(k,k) for k in tri))
    root.mkdir(parents=True,exist_ok=True)
    path.write_text('\n'.join(lines)+'\n',encoding='utf-8')
