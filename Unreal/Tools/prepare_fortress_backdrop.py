"""Versioned native materials for masonry and a painted distant skyline.
No external images, marketplace models or replacement of user-authored graphs.
"""
import unreal
A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
F='/Game/Art/Materials/'
def node(m,cls): return E.create_material_expression(m,cls)
def link(a,b,name,output=''):
    names=E.get_material_expression_input_names(b)
    if name not in names and len(names)==1: name=names[0]
    assert E.connect_material_expressions(a,output,b,name),(name,list(names))
def prop(n,p): assert E.connect_material_property(n,'',p),str(p)
def scalar(m,value,name=None):
    n=node(m,unreal.MaterialExpressionScalarParameter if name else unreal.MaterialExpressionConstant)
    n.set_editor_property('default_value' if name else 'r',value)
    if name: n.set_editor_property('parameter_name',name)
    return n
def color(m,value,name):
    n=node(m,unreal.MaterialExpressionVectorParameter)
    n.set_editor_property('parameter_name',name);n.set_editor_property('default_value',unreal.LinearColor(*value,1));return n
def binary(m,cls,a,b):
    n=node(m,cls);link(a,n,'A');link(b,n,'B');return n
def unary(m,cls,a):
    n=node(m,cls);link(a,n,'Input');return n
def mask(m,source,axis):
    n=node(m,unreal.MaterialExpressionComponentMask)
    for c in ('r','g','b','a'): n.set_editor_property(c,c==axis)
    link(source,n,'Input');return n
def blend(m,a,b,weight):
    n=node(m,unreal.MaterialExpressionLinearInterpolate);link(a,n,'A');link(b,n,'B');link(weight,n,'Alpha');return n
def finish(m):
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m);E.layout_material_expressions(m);assert A.save_loaded_asset(m)
if not A.does_asset_exist(F+'M_FortressMasonryV6'):
    m=A.duplicate_asset(F+'M_MapRockV3',F+'M_FortressMasonryV6')
    old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    world=node(m,unreal.MaterialExpressionWorldPosition)
    normal=node(m,unreal.MaterialExpressionVertexNormalWS)
    nx=unary(m,unreal.MaterialExpressionAbs,mask(m,normal,'r'))
    horizontal=blend(m,mask(m,world,'r'),mask(m,world,'g'),nx)
    safe_w=binary(m,unreal.MaterialExpressionMax,scalar(m,74,'BrickWidth'),scalar(m,8))
    safe_h=binary(m,unreal.MaterialExpressionMax,scalar(m,28,'BrickHeight'),scalar(m,8))
    u=binary(m,unreal.MaterialExpressionDivide,horizontal,safe_w)
    v=binary(m,unreal.MaterialExpressionDivide,mask(m,world,'b'),safe_h)
    row=unary(m,unreal.MaterialExpressionFloor,v)
    stagger=binary(m,unreal.MaterialExpressionMultiply,row,scalar(m,.5))
    u=unary(m,unreal.MaterialExpressionFrac,binary(m,unreal.MaterialExpressionAdd,u,stagger))
    v=unary(m,unreal.MaterialExpressionFrac,v)
    edges=[]
    for value,a,b in ((u,.025,.065),(v,.035,.10)):
        n=node(m,unreal.MaterialExpressionSmoothStep)
        link(scalar(m,a),n,'Min');link(scalar(m,b),n,'Max');link(value,n,'Value');edges.append(n)
    brick=binary(m,unreal.MaterialExpressionMultiply,*edges)
    depth=node(m,unreal.MaterialExpressionPixelDepth)
    far=unary(m,unreal.MaterialExpressionSaturate,binary(m,unreal.MaterialExpressionDivide,binary(m,unreal.MaterialExpressionSubtract,depth,scalar(m,2800)),scalar(m,4800)))
    # Fade mortar at distance to keep a sub-pixel brick pattern from sparkling.
    brick=blend(m,brick,scalar(m,1),far)
    tinted=binary(m,unreal.MaterialExpressionMultiply,old,color(m,(.92,1.01,1.02),'StoneTint'))
    prop(blend(m,color(m,(.025,.033,.029),'Mortar'),tinted,brick),unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.93,'MasonryRoughness'),unreal.MaterialProperty.MP_ROUGHNESS);finish(m)
if not A.does_asset_exist(F+'M_PaintedDistanceV6'):
    m=T.create_asset('M_PaintedDistanceV6','/Game/Art/Materials',unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided',True)
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    tint=binary(m,unreal.MaterialExpressionMultiply,vertex,color(m,(1,1,1),'DistancePalette'))
    prop(tint,unreal.MaterialProperty.MP_EMISSIVE_COLOR);finish(m)
for name,rgb in (('M_FortressTimberV6',(.16,.055,.027)),('M_FortressRoofV6',(.065,.079,.073))):
    if A.does_asset_exist(F+name): continue
    m=T.create_asset(name,'/Game/Art/Materials',unreal.Material,unreal.MaterialFactoryNew())
    random=node(m,unreal.MaterialExpressionPerInstanceRandom)
    variation=binary(m,unreal.MaterialExpressionAdd,binary(m,unreal.MaterialExpressionMultiply,random,scalar(m,.24)),scalar(m,.86))
    prop(binary(m,unreal.MaterialExpressionMultiply,color(m,rgb,'Tint'),variation),unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.86,'Roughness'),unreal.MaterialProperty.MP_ROUGHNESS)
    m.set_editor_property('two_sided',True);finish(m)
if not A.does_asset_exist(F+'M_InkMountainV6'):
    m=A.duplicate_asset(F+'M_PaintedDistanceV6',F+'M_InkMountainV6')
    old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    world=node(m,unreal.MaterialExpressionWorldPosition)
    uv=node(m,unreal.MaterialExpressionComponentMask)
    uv.set_editor_property('r',True);uv.set_editor_property('g',False);uv.set_editor_property('b',True);uv.set_editor_property('a',False)
    link(world,uv,'Input')
    scale=binary(m,unreal.MaterialExpressionDivide,uv,scalar(m,950,'RockGrainScale'))
    sample=node(m,unreal.MaterialExpressionTextureSample)
    sample.set_editor_property('texture',A.load_asset('/Game/Art/Battle/T_MapRockV3'));link(scale,sample,'UVs')
    grain=node(m,unreal.MaterialExpressionMultiply);link(sample,grain,'A','R');link(scalar(m,.9),grain,'B')
    grain=binary(m,unreal.MaterialExpressionAdd,grain,scalar(m,.60))
    depth=node(m,unreal.MaterialExpressionPixelDepth)
    far=unary(m,unreal.MaterialExpressionSaturate,binary(m,unreal.MaterialExpressionDivide,binary(m,unreal.MaterialExpressionSubtract,depth,scalar(m,3500)),scalar(m,7000)))
    grain=blend(m,grain,scalar(m,1),far)
    prop(binary(m,unreal.MaterialExpressionMultiply,old,grain),unreal.MaterialProperty.MP_EMISSIVE_COLOR);finish(m)
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cls=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
field=next(a for a in actors.get_all_level_actors() if a.get_class()==cls)
for prop_name,old_names,new_name in (
    ('fortress_material',('M_MapRockV3','M_LandscapeRockV5'),'M_FortressMasonryV6'),
    ('distant_material',('M_Field','M_MapGroundV3','M_LandscapeGroundV5','M_PaintedDistanceV6'),'M_InkMountainV6')):
    current=field.get_editor_property(prop_name)
    if not current or current.get_name() in old_names: field.set_editor_property(prop_name,A.load_asset(F+new_name))
for component in field.get_components_by_class(unreal.InstancedStaticMeshComponent):
    name=component.get_name()
    current=component.get_material(0)
    if name=='BackdropTimber' and (not current or current.get_name()=='M_Bark'): component.set_material(0,A.load_asset(F+'M_FortressTimberV6'))
    if name=='BackdropRoofs' and (not current or current.get_name()=='M_BattleRoof'): component.set_material(0,A.load_asset(F+'M_FortressRoofV6'))
assert levels.save_current_level()
unreal.log('GUNYU_FORTRESS_ART: PASS (editable masonry, roof/timber, painted mountain material, saved map)')
