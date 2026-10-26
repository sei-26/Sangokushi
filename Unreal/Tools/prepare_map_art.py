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


from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).resolve().parent/'import_art.py'),run_name='__main__')
texture=A.load_asset('/Game/Art/Battle/T_MapRockV3')
assert isinstance(texture,unreal.Texture2D)
texture.set_editor_property('power_of_two_mode',unreal.TexturePowerOfTwoSetting.RESIZE_TO_SPECIFIC_RESOLUTION)
texture.set_editor_property('resize_during_build_x',1024)
texture.set_editor_property('resize_during_build_y',1024)
texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
texture.set_editor_property('max_texture_size',1024)
A.save_loaded_asset(texture)
solid('M_MapPlasterV3',(.42,.37,.265),.94)

def rock_sample(m):
    uv=node(m,unreal.MaterialExpressionTextureCoordinate)
    uv.set_editor_property('u_tiling',.7);uv.set_editor_property('v_tiling',.7)
    sample=node(m,unreal.MaterialExpressionTextureSample)
    sample.set_editor_property('texture',texture);link(uv,sample,'UVs')
    tint=node(m,unreal.MaterialExpressionMultiply)
    link(sample,tint,'A','RGB');link(scalar(m,.60),tint,'B')
    return tint

if not A.does_asset_exist(FOLDER+'/M_MapRockV3'):
    m=create('M_MapRockV3')
    prop(rock_sample(m),unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.91),unreal.MaterialProperty.MP_ROUGHNESS)
    finish(m)
if not A.does_asset_exist(FOLDER+'/M_MapGroundV3'):
    m=A.duplicate_asset(FOLDER+'/M_GroundCinematicV2',FOLDER+'/M_MapGroundV3')
    old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    normal=node(m,unreal.MaterialExpressionVertexNormalWS)
    up=node(m,unreal.MaterialExpressionDotProduct)
    link(normal,up,'A');link(color(m,(0,0,1,1)),up,'B')
    slope=node(m,unreal.MaterialExpressionOneMinus);link(up,slope,'Input')
    gain=node(m,unreal.MaterialExpressionMultiply);link(slope,gain,'A');link(scalar(m,5.5),gain,'B')
    weight=node(m,unreal.MaterialExpressionSaturate);link(gain,weight,'Input')
    # Vertex tint carries only known terrain visibility; the new texture must
    # respect it as well instead of brightening unexplored mountain slopes.
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    tinted=node(m,unreal.MaterialExpressionMultiply)
    link(rock_sample(m),tinted,'A');link(vertex,tinted,'B')
    level=node(m,unreal.MaterialExpressionMultiply);link(tinted,level,'A');link(scalar(m,4),level,'B')
    blend=node(m,unreal.MaterialExpressionLinearInterpolate)
    link(old,blend,'A');link(level,blend,'B');link(weight,blend,'Alpha')
    prop(blend,unreal.MaterialProperty.MP_BASE_COLOR)
    finish(m)
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cls=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
field=next(a for a in actors.get_all_level_actors() if a.get_class()==cls)
current=field.get_editor_property('terrain_material')
if not current or current.get_name() in ('M_Field','M_BattleGround','M_BattleGroundTextured','M_GroundCinematicV2'):
    field.set_editor_property('terrain_material',A.load_asset(FOLDER+'/M_MapGroundV3'))
if not field.get_editor_property('rock_material'):
    field.set_editor_property('rock_material',A.load_asset(FOLDER+'/M_MapRockV3'))
levels.save_current_level()
unreal.log('GUNYU_MAP_ART: PASS (original rock texture, slope blend, plaster, editable references)')
