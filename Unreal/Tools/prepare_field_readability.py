"""Editable fortification props and observed-unit canopy cutaways. No downloads."""
from pathlib import Path
import runpy
import unreal

A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
FOLDER='/Game/Art/Materials'
root=Path(__file__).resolve().parent
runpy.run_path(str(root/'make_battlefield_props.py'),run_name='__main__')
runpy.run_path(str(root/'import_battle_meshes.py'),run_name='__main__')

def node(m,cls): return E.create_material_expression(m,cls)
def link(a,b,name,output=''):
    names=E.get_material_expression_input_names(b)
    if name not in names and len(names)==1: name=names[0]
    assert E.connect_material_expressions(a,output,b,name),name
def scalar(m,value):
    n=node(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
def finish(m):
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m);E.layout_material_expressions(m);A.save_loaded_asset(m)
def solid(name,rgb):
    if A.does_asset_exist(FOLDER+'/'+name): return
    m=T.create_asset(name,FOLDER,unreal.Material,unreal.MaterialFactoryNew())
    c=node(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*rgb,1))
    assert E.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    assert E.connect_material_property(scalar(m,.88),'',unreal.MaterialProperty.MP_ROUGHNESS)
    m.set_editor_property('two_sided',True);finish(m)

solid('M_CampCanvasV4',(.29,.24,.15))
solid('M_CrateWoodV4',(.20,.105,.035))
path=FOLDER+'/M_CommandCanopyV4'
if not A.does_asset_exist(path):
    m=A.duplicate_asset(FOLDER+'/M_CanopyCinematicV2',path)
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
    m.set_editor_property('opacity_mask_clip_value',.333)
    world=node(m,unreal.MaterialExpressionWorldPosition)
    xy=node(m,unreal.MaterialExpressionComponentMask)
    xy.set_editor_property('r',True);xy.set_editor_property('g',True)
    xy.set_editor_property('b',False);xy.set_editor_property('a',False)
    link(world,xy,'Input')
    combined=None
    for i in range(6):
        p=node(m,unreal.MaterialExpressionVectorParameter)
        p.set_editor_property('parameter_name','UnitClear%d'%i)
        p.set_editor_property('default_value',unreal.LinearColor(-100000,-100000,0,0))
        uv=node(m,unreal.MaterialExpressionComponentMask)
        for key,value in (('r',True),('g',True),('b',False),('a',False)): uv.set_editor_property(key,value)
        link(p,uv,'Input')
        distance=node(m,unreal.MaterialExpressionDistance);link(xy,distance,'A');link(uv,distance,'B')
        offset=node(m,unreal.MaterialExpressionSubtract);link(distance,offset,'A');link(p,offset,'B','A')
        feather=node(m,unreal.MaterialExpressionDivide);link(offset,feather,'A');link(scalar(m,35),feather,'B')
        mask=node(m,unreal.MaterialExpressionSaturate);link(feather,mask,'Input')
        if combined is not None:
            minimum=node(m,unreal.MaterialExpressionMin);link(combined,minimum,'A');link(mask,minimum,'B');combined=minimum
        else: combined=mask
    dither=node(m,unreal.MaterialExpressionMaterialFunctionCall)
    function=A.load_asset('/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA')
    assert function and dither.set_material_function(function)
    inputs=E.get_material_expression_input_names(dither)
    assert inputs
    link(combined,dither,inputs[0])
    assert E.connect_material_property(dither,'',unreal.MaterialProperty.MP_OPACITY_MASK)
    finish(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cls=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
field=next(a for a in actors.get_all_level_actors() if a.get_class()==cls)
for prop,name in (('palisade_mesh','SM_PalisadeStakeV4'),('watchtower_mesh','SM_WatchtowerV4'),('tent_mesh','SM_CampTentV4'),('supply_crate_mesh','SM_SupplyCrateV4')):
    if not field.get_editor_property(prop): field.set_editor_property(prop,A.load_asset('/Game/Art/Battle/'+name))
current=field.get_editor_property('foliage_material')
if not current or current.get_name() in ('M_Trees','M_BattleLeaves','M_BattleLeavesWind','M_CanopyCinematicV2'):
    field.set_editor_property('foliage_material',A.load_asset(path))
levels.save_current_level()
unreal.log('GUNYU_FIELD_READABILITY: PASS (four original meshes, public fortifications, observed-unit canopy material)')
