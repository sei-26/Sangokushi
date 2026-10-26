"""Versioned battle art. Only stock references change; existing custom art stays.
Run after building the module. No downloads, marketplace packs or per-pixel noise.
"""
import unreal

A = unreal.EditorAssetLibrary
E = unreal.MaterialEditingLibrary
T = unreal.AssetToolsHelpers.get_asset_tools()
FOLDER = '/Game/Art/Materials'

def node(m, cls):
    return E.create_material_expression(m, cls)

def link(a, b, name, output=''):
    names=E.get_material_expression_input_names(b)
    if name not in names and len(names)==1:
        name=names[0]
    assert E.connect_material_expressions(a, output, b, name), name

def prop(a, mprop, output=''):
    assert E.connect_material_property(a, output, mprop), str(mprop)

def scalar(m, value):
    n = node(m, unreal.MaterialExpressionConstant)
    n.set_editor_property('r', value)
    return n

def color(m, value):
    n = node(m, unreal.MaterialExpressionConstant3Vector)
    n.set_editor_property('constant', unreal.LinearColor(*value))
    return n

def create(name):
    return T.create_asset(name, FOLDER, unreal.Material, unreal.MaterialFactoryNew())

def finish(m):
    E.set_material_usage(m, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m)
    E.layout_material_expressions(m)
    A.save_loaded_asset(m)

def solid(name, rgb, roughness, vertex=False):
    if A.does_asset_exist(FOLDER+'/'+name): return
    m = create(name)
    base = node(m, unreal.MaterialExpressionVertexColor) if vertex else color(m, (*rgb,1))
    prop(base, unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m, roughness), unreal.MaterialProperty.MP_ROUGHNESS)
    m.set_editor_property('two_sided', True)
    finish(m)

solid('M_ArmyStandardV2', (0,0,0), .83, vertex=True)
solid('M_InfantryLeatherV2', (.052,.061,.055), .92)

if not A.does_asset_exist(FOLDER+'/M_GroundCinematicV2'):
    m = create('M_GroundCinematicV2')
    texture = A.load_asset('/Game/Art/Battle/T_BattleGround')
    assert isinstance(texture, unreal.Texture2D)
    uv = node(m, unreal.MaterialExpressionTextureCoordinate)
    uv.set_editor_property('u_tiling', .66); uv.set_editor_property('v_tiling', .66)
    macro_uv = node(m, unreal.MaterialExpressionTextureCoordinate)
    macro_uv.set_editor_property('u_tiling', .143); macro_uv.set_editor_property('v_tiling', .173)
    def sample(coordinates):
        n = node(m, unreal.MaterialExpressionTextureSample)
        n.set_editor_property('texture', texture)
        link(coordinates,n,'UVs')
        return n
    detail, macro = sample(uv), sample(macro_uv)
    blend = node(m, unreal.MaterialExpressionLinearInterpolate)
    link(detail,blend,'A','RGB'); link(macro,blend,'B','RGB'); link(scalar(m,.24),blend,'Alpha')
    vertex = node(m, unreal.MaterialExpressionVertexColor)
    tint = node(m, unreal.MaterialExpressionMultiply)
    link(vertex,tint,'A'); link(color(m,(4.1,4.0,3.8,1)),tint,'B')
    base = node(m, unreal.MaterialExpressionMultiply)
    link(tint,base,'A'); link(blend,base,'B')
    prop(base,unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.90),unreal.MaterialProperty.MP_ROUGHNESS)
    channels=[]
    for dx,dy in ((1/1024,0),(0,1/1024)):
        offset = node(m,unreal.MaterialExpressionConstant2Vector)
        offset.set_editor_property('r',dx); offset.set_editor_property('g',dy)
        add = node(m,unreal.MaterialExpressionAdd); link(uv,add,'A'); link(offset,add,'B')
        adjacent = sample(add)
        delta = node(m,unreal.MaterialExpressionSubtract)
        link(detail,delta,'A','R'); link(adjacent,delta,'B','R')
        gain = node(m,unreal.MaterialExpressionMultiply)
        link(delta,gain,'A'); link(scalar(m,2.3),gain,'B'); channels.append(gain)
    xy=node(m,unreal.MaterialExpressionAppendVector); link(channels[0],xy,'A');link(channels[1],xy,'B')
    xyz=node(m,unreal.MaterialExpressionAppendVector);link(xy,xyz,'A');link(scalar(m,1),xyz,'B')
    normal=node(m,unreal.MaterialExpressionNormalize);link(xyz,normal,'VectorInput')
    prop(normal,unreal.MaterialProperty.MP_NORMAL)
    m.set_editor_property('two_sided',True)
    finish(m)

if not A.does_asset_exist(FOLDER+'/M_CanopyCinematicV2'):
    m=create('M_CanopyCinematicV2')
    variation=node(m,unreal.MaterialExpressionPerInstanceRandom)
    spread=node(m,unreal.MaterialExpressionMultiply);link(variation,spread,'A');link(scalar(m,.48),spread,'B')
    level=node(m,unreal.MaterialExpressionAdd);link(spread,level,'A');link(scalar(m,.76),level,'B')
    base=node(m,unreal.MaterialExpressionMultiply)
    link(color(m,(.045,.105,.030,1)),base,'A');link(level,base,'B')
    prop(base,unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.86),unreal.MaterialProperty.MP_ROUGHNESS)
    position=node(m,unreal.MaterialExpressionWorldPosition)
    dot=node(m,unreal.MaterialExpressionDotProduct);link(position,dot,'A');link(color(m,(.008,.006,.004,1)),dot,'B')
    time=node(m,unreal.MaterialExpressionTime)
    phase=node(m,unreal.MaterialExpressionAdd);link(dot,phase,'A');link(time,phase,'B')
    sine=node(m,unreal.MaterialExpressionSine);link(phase,sine,'Input')
    wind=node(m,unreal.MaterialExpressionMultiply);link(sine,wind,'A');link(color(m,(1.1,.7,.18,1)),wind,'B')
    prop(wind,unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    m.set_editor_property('two_sided',True)
    finish(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
fields=[a for a in actors.get_all_level_actors() if a.get_class()==field_class]
assert len(fields)==1
field=fields[0]
if not field.get_editor_property('spearhead_mesh'):
    field.set_editor_property('spearhead_mesh',A.load_asset('/Game/Art/Battle/SM_SpearheadV2'))
if not field.get_editor_property('cart_wheel_mesh'):
    field.set_editor_property('cart_wheel_mesh',A.load_asset('/Game/Art/Battle/SM_CartWheelV2'))
if not field.get_editor_property('standard_material'):
    field.set_editor_property('standard_material',A.load_asset(FOLDER+'/M_ArmyStandardV2'))
for name,previous,replacement in (
    ('terrain_material',('M_Field','M_BattleGround','M_BattleGroundTextured'),'M_GroundCinematicV2'),
    ('foliage_material',('M_Trees','M_BattleLeaves','M_BattleLeavesWind'),'M_CanopyCinematicV2')):
    current=field.get_editor_property(name)
    if not current or current.get_name() in previous:
        field.set_editor_property(name,A.load_asset(FOLDER+'/'+replacement))

# Apply the authored lighting once; later reruns respect the user's scene edits.
tag=unreal.Name('GunyuCinemaV2')
tags=list(field.get_editor_property('tags'))
if tag not in tags:
    for actor in actors.get_all_level_actors():
        if isinstance(actor,unreal.DirectionalLight):
            light=actor.get_component_by_class(unreal.DirectionalLightComponent)
            actor.set_actor_rotation(unreal.Rotator(-43,-35,0),False)
            light.set_editor_property('intensity',4.2)
            light.set_editor_property('light_source_angle',1.3)
            light.set_light_color(unreal.LinearColor(1,.95,.84,1))
        if isinstance(actor,unreal.SkyLight):
            light=actor.get_component_by_class(unreal.SkyLightComponent)
            light.set_editor_property('intensity',1.45)
            light.set_light_color(unreal.LinearColor(.78,.86,1,1))
    tags.append(tag); field.set_editor_property('tags',tags)
levels.save_current_level()
unreal.log('GUNYU_CINEMA_ART: PASS (versioned ground, canopy, cloth, lighting; custom references preserved)')
