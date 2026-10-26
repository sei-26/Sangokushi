"""Editable V5 landscape materials and light. Run after the C++ module build.
Version assets instead of replacing hand-edited source graphs; no downloads.
"""
from pathlib import Path
import runpy
import unreal

A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
FOLDER='/Game/Art/Materials'
root=Path(__file__).resolve().parent
runpy.run_path(str(root/'make_landscape_details.py'),run_name='__main__')
runpy.run_path(str(root/'import_battle_meshes.py'),run_name='__main__')

def node(m,cls): return E.create_material_expression(m,cls)
def link(a,b,name,output=''):
    names=E.get_material_expression_input_names(b)
    if name not in names and len(names)==1: name=names[0]
    assert E.connect_material_expressions(a,output,b,name),(name,names)
def prop(a,p,output=''): assert E.connect_material_property(a,output,p),str(p)
def scalar(m,value,name=None):
    n=node(m,unreal.MaterialExpressionScalarParameter if name else unreal.MaterialExpressionConstant)
    n.set_editor_property('default_value' if name else 'r',value)
    if name: n.set_editor_property('parameter_name',name)
    return n
def color(m,value,name):
    n=node(m,unreal.MaterialExpressionVectorParameter)
    n.set_editor_property('parameter_name',name);n.set_editor_property('default_value',unreal.LinearColor(*value,1))
    return n
def multiply(m,a,b):
    n=node(m,unreal.MaterialExpressionMultiply);link(a,n,'A');link(b,n,'B');return n
def finish(m):
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m);E.layout_material_expressions(m);A.save_loaded_asset(m)
def duplicate(name,source):
    return None if A.does_asset_exist(FOLDER+'/'+name) else A.duplicate_asset(FOLDER+'/'+source,FOLDER+'/'+name)

def tint_material(name,source,tint,parameter):
    m=duplicate(name,source)
    if m:
        old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
        prop(multiply(m,old,color(m,tint,parameter)),unreal.MaterialProperty.MP_BASE_COLOR)
        finish(m)
    return m

tint_material('M_LandscapeGroundV5','M_MapGroundV3',(.82,.95,1.04),'EarthBalance')
tint_material('M_LandscapeRockV5','M_MapRockV3',(.57,.65,.70),'StoneBalance')
m=duplicate('M_LandscapeCanopyV5','M_CommandCanopyV4')
if m:
    old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    random=node(m,unreal.MaterialExpressionPerInstanceRandom)
    variation=node(m,unreal.MaterialExpressionLinearInterpolate)
    link(color(m,(.87,.95,1.08),'CoolLeaf'),variation,'A')
    link(color(m,(1.24,1.03,.85),'WarmLeaf'),variation,'B');link(random,variation,'Alpha')
    prop(multiply(m,old,variation),unreal.MaterialProperty.MP_BASE_COLOR)
    # The observed-unit opacity mask and vertex wind from V4 stay connected.
    finish(m)

m=duplicate('M_LandscapeWaterV5','M_BattleWater')
if m:
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    # Use alpha for visibility, independently from the depth tint in RGB.
    alpha=node(m,unreal.MaterialExpressionComponentMask);alpha.set_editor_property('a',False)
    alpha.set_editor_property('r',True);alpha.set_editor_property('g',False);alpha.set_editor_property('b',False)
    link(vertex,alpha,'Input','A')
    reflected=multiply(m,color(m,(.18,.27,.32),'ReflectedSky'),alpha)
    fresnel=node(m,unreal.MaterialExpressionFresnel)
    fresnel.set_editor_property('exponent',4.5);fresnel.set_editor_property('base_reflect_fraction',.04)
    weight=multiply(m,fresnel,scalar(m,.65,'ReflectionStrength'))
    blend=node(m,unreal.MaterialExpressionLinearInterpolate)
    link(vertex,blend,'A');link(reflected,blend,'B');link(weight,blend,'Alpha')
    prop(blend,unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.24,'WaterRoughness'),unreal.MaterialProperty.MP_ROUGHNESS)
    prop(scalar(m,.60,'WaterSpecular'),unreal.MaterialProperty.MP_SPECULAR)
    # Existing time-driven wave normals stay; the water surface never displaces.
    finish(m)

# Repair only the alpha pin of this generated water version if an early
# graph used the RGB output. Preserve all other authored parameter values.
water=A.load_asset(FOLDER+'/M_LandscapeWaterV5')
pending=[E.get_material_property_input_node(water,unreal.MaterialProperty.MP_BASE_COLOR)]
visited=set()
repaired=False
while pending:
    expression=pending.pop()
    if not expression or expression.get_path_name() in visited: continue
    visited.add(expression.get_path_name())
    inputs=E.get_inputs_for_material_expression(water,expression)
    if isinstance(expression,unreal.MaterialExpressionComponentMask) and expression.get_editor_property('a'):
        sources=[source for source in inputs if isinstance(source,unreal.MaterialExpressionVertexColor)]
        if sources:
            expression.set_editor_property('a',False);expression.set_editor_property('r',True)
            link(sources[0],expression,'Input','A');repaired=True
    pending.extend(source for source in inputs if source)
if repaired: finish(water)

m=duplicate('M_LandscapeBarkV5','M_Bark')
if m:
    old=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    position=node(m,unreal.MaterialExpressionWorldPosition)
    dot=node(m,unreal.MaterialExpressionDotProduct)
    link(position,dot,'A');link(color(m,(.035,.045,.002),'BarkGrainScale'),dot,'B')
    sine=node(m,unreal.MaterialExpressionSine);link(dot,sine,'Input')
    band=multiply(m,sine,scalar(m,.13,'BarkContrast'))
    level=node(m,unreal.MaterialExpressionAdd);link(band,level,'A');link(scalar(m,.90),level,'B')
    prop(multiply(m,old,level),unreal.MaterialProperty.MP_BASE_COLOR);finish(m)

if not A.does_asset_exist(FOLDER+'/M_LandscapeGrassV5'):
    m=T.create_asset('M_LandscapeGrassV5',FOLDER,unreal.Material,unreal.MaterialFactoryNew())
    random=node(m,unreal.MaterialExpressionPerInstanceRandom)
    blend=node(m,unreal.MaterialExpressionLinearInterpolate)
    link(color(m,(.055,.105,.038),'GreenGrass'),blend,'A')
    link(color(m,(.15,.17,.065),'DryGrass'),blend,'B');link(random,blend,'Alpha')
    prop(blend,unreal.MaterialProperty.MP_BASE_COLOR)
    prop(scalar(m,.88,'GrassRoughness'),unreal.MaterialProperty.MP_ROUGHNESS)
    m.set_editor_property('two_sided',True);finish(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
field=next(a for a in actors.get_all_level_actors() if a.get_class()==field_class)
for property_name,old_names,name in (
    ('terrain_material',('M_MapGroundV3',),'M_LandscapeGroundV5'),
    ('rock_material',('M_MapRockV3',),'M_LandscapeRockV5'),
    ('foliage_material',('M_CommandCanopyV4',),'M_LandscapeCanopyV5'),
    ('water_material',('M_BattleWater',),'M_LandscapeWaterV5'),
    ('rock_mesh',('SM_RockOutcrop',),'SM_WeatheredBoulderV5')):
    current=field.get_editor_property(property_name)
    if not current or current.get_name() in old_names:
        folder='/Game/Art/Battle/' if property_name=='rock_mesh' else FOLDER+'/'
        field.set_editor_property(property_name,A.load_asset(folder+name))
for component in field.get_components_by_class(unreal.InstancedStaticMeshComponent):
    if component.get_name()=='TreeTrunks':
        current_mesh=component.get_editor_property('static_mesh')
        if not current_mesh or current_mesh.get_name()=='Cube':
            component.set_static_mesh(A.load_asset('/Engine/BasicShapes/Cylinder.Cylinder'))
    if component.get_name() in ('TreeTrunks','GrassTufts'):
        name='M_LandscapeBarkV5' if component.get_name()=='TreeTrunks' else 'M_LandscapeGrassV5'
        current=component.get_material(0)
        if not current or current.get_name() in ('M_Bark','M_BattleGrass'):
            component.set_material(0,A.load_asset(FOLDER+'/'+name))

# Authored once. Later script runs keep the user's light edits.
tag=unreal.Name('GunyuLandscapeV5')
tags=list(field.get_editor_property('tags'))
if tag not in tags:
    for actor in actors.get_all_level_actors():
        if isinstance(actor,unreal.DirectionalLight):
            component=actor.get_component_by_class(unreal.DirectionalLightComponent)
            actor.set_actor_rotation(unreal.Rotator(-39,-42,0),False)
            component.set_editor_property('intensity',3.8)
            component.set_editor_property('light_source_angle',2.3)
            component.set_light_color(unreal.LinearColor(1,.96,.87,1))
        if isinstance(actor,unreal.SkyLight):
            component=actor.get_component_by_class(unreal.SkyLightComponent)
            component.set_editor_property('intensity',2.0)
            component.set_light_color(unreal.LinearColor(.78,.87,1,1))
        if isinstance(actor,unreal.ExponentialHeightFog):
            component=actor.get_component_by_class(unreal.ExponentialHeightFogComponent)
            component.set_editor_property('fog_density',.008)
            component.set_editor_property('fog_height_falloff',.12)
            component.set_editor_property('start_distance',2300)
            component.set_editor_property('fog_max_opacity',.28)
            component.set_editor_property('fog_inscattering_luminance',unreal.LinearColor(.25,.34,.42,1))
    tags.append(tag);field.set_editor_property('tags',tags)
levels.save_current_level()
for property_name in ('terrain_material','foliage_material','water_material','rock_material'):
    assert field.get_editor_property(property_name)
canopy=A.load_asset(FOLDER+'/M_LandscapeCanopyV5')
assert E.get_material_property_input_node(canopy,unreal.MaterialProperty.MP_OPACITY_MASK)
unreal.log('GUNYU_LANDSCAPE_ART: PASS (versioned materials, observed canopy mask, water depth, editable light, original boulder)')
