"""Original textured surfaces, geometric foliage and editable battlefield lighting.
Create versioned materials; keep user-authored assets and old materials intact.
"""
from pathlib import Path
import unreal

A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
folder='/Game/Art/Materials'

def node(mat, cls):
    return E.create_material_expression(mat, cls)

def link(a, b, input_name, output=''):
    names=E.get_material_expression_input_names(b)
    if input_name not in names:
        normalized=input_name.lower().replace(' ','').replace('_','')
        matching=[name for name in names if normalized in name.lower().replace(' ','').replace('_','')]
        if len(matching)==1: input_name=matching[0]
        elif len(names)==1: input_name=names[0]
    assert E.connect_material_expressions(a,output,b,input_name), (input_name,names)

def prop(a, output, target):
    assert E.connect_material_property(a,output,target), str(target)

def constant(mat, value):
    n=node(mat,unreal.MaterialExpressionConstant)
    n.set_editor_property('r',value)
    return n

def color(mat, value):
    n=node(mat,unreal.MaterialExpressionConstant3Vector)
    n.set_editor_property('constant',unreal.LinearColor(*value))
    return n

def material(name):
    return T.create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew())

def finish(m):
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m)
    E.layout_material_expressions(m)
    A.save_loaded_asset(m)

texture_path='/Game/Art/Battle/T_BattleGround'
if not A.does_asset_exist(texture_path):
    task=unreal.AssetImportTask()
    task.filename=str(Path(__file__).resolve().parents[1]/'ArtSource/Battle/ground-albedo-v1.png')
    task.destination_path='/Game/Art/Battle';task.destination_name='T_BattleGround'
    task.automated=True;task.save=True;task.factory=unreal.TextureFactory()
    T.import_asset_tasks([task])
texture=A.load_asset(texture_path)
assert isinstance(texture,unreal.Texture2D)
texture.set_editor_property('power_of_two_mode',unreal.TexturePowerOfTwoSetting.RESIZE_TO_SPECIFIC_RESOLUTION)
texture.set_editor_property('resize_during_build_x',1024)
texture.set_editor_property('resize_during_build_y',1024)
texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
A.save_loaded_asset(texture)

if not A.does_asset_exist(folder+'/M_BattleGroundTextured'):
    m=material('M_BattleGroundTextured')
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    uv=node(m,unreal.MaterialExpressionTextureCoordinate)
    sample=node(m,unreal.MaterialExpressionTextureSample)
    sample.set_editor_property('texture',texture)
    link(uv,sample,'UVs')
    tint=node(m,unreal.MaterialExpressionMultiply)
    link(vertex,tint,'A');link(color(m,(5.5,7,6.5,1)),tint,'B')
    base=node(m,unreal.MaterialExpressionMultiply)
    link(tint,base,'A');link(sample,base,'B','RGB')
    prop(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
    prop(constant(m,.91),'',unreal.MaterialProperty.MP_ROUGHNESS)
    # Two adjacent height samples produce inexpensive subtle tangent-space relief.
    channels=[]
    for step in ((1/texture.blueprint_get_size_x(),0),(0,1/texture.blueprint_get_size_y())):
        offset=node(m,unreal.MaterialExpressionConstant2Vector)
        offset.set_editor_property('r',step[0]);offset.set_editor_property('g',step[1])
        add=node(m,unreal.MaterialExpressionAdd);link(uv,add,'A');link(offset,add,'B')
        adjacent=node(m,unreal.MaterialExpressionTextureSample)
        adjacent.set_editor_property('texture',texture);link(add,adjacent,'UVs')
        delta=node(m,unreal.MaterialExpressionSubtract);link(sample,delta,'A','R');link(adjacent,delta,'B','R')
        gain=node(m,unreal.MaterialExpressionMultiply);link(delta,gain,'A');link(constant(m,2.0),gain,'B')
        channels.append(gain)
    xy=node(m,unreal.MaterialExpressionAppendVector);link(channels[0],xy,'A');link(channels[1],xy,'B')
    xyz=node(m,unreal.MaterialExpressionAppendVector);link(xy,xyz,'A');link(constant(m,1),xyz,'B')
    normal=node(m,unreal.MaterialExpressionNormalize);link(xyz,normal,'VectorInput')
    prop(normal,'',unreal.MaterialProperty.MP_NORMAL)
    m.set_editor_property('two_sided',True)
    finish(m)

def textured_solid(name, rgb, roughness, metallic=0, leaves=False):
    if A.does_asset_exist(folder+'/'+name): return
    m=material(name)
    position=node(m,unreal.MaterialExpressionWorldPosition)
    if leaves:
        # Real leaf geometry provides detail. Avoid expensive per-pixel noise on overlapping crowns.
        random=node(m,unreal.MaterialExpressionPerInstanceRandom)
        spread=node(m,unreal.MaterialExpressionMultiply)
        link(random,spread,'A');link(constant(m,.50),spread,'B')
        noise=node(m,unreal.MaterialExpressionAdd)
        link(spread,noise,'A');link(constant(m,.72),noise,'B')
    else:
        noise=node(m,unreal.MaterialExpressionNoise)
        noise.set_editor_property('scale',.5)
        noise.set_editor_property('levels',1);noise.set_editor_property('quality',1)
        noise.set_editor_property('output_min',.60);noise.set_editor_property('output_max',1.3)
        link(position,noise,'Position')
    multiply=node(m,unreal.MaterialExpressionMultiply)
    link(color(m,(*rgb,1)),multiply,'A');link(noise,multiply,'B')
    prop(multiply,'',unreal.MaterialProperty.MP_BASE_COLOR)
    prop(constant(m,roughness),'',unreal.MaterialProperty.MP_ROUGHNESS)
    prop(constant(m,metallic),'',unreal.MaterialProperty.MP_METALLIC)
    if leaves:
        m.set_editor_property('two_sided',True)
        # Cheap vertex wind: no CPU updates to hundreds of foliage instances.
        dot=node(m,unreal.MaterialExpressionDotProduct)
        link(position,dot,'A');link(color(m,(.02,.016,.01,1)),dot,'B')
        time=node(m,unreal.MaterialExpressionTime)
        add=node(m,unreal.MaterialExpressionAdd);link(dot,add,'A');link(time,add,'B')
        sine=node(m,unreal.MaterialExpressionSine);link(add,sine,'Input')
        wind=node(m,unreal.MaterialExpressionMultiply)
        link(sine,wind,'A');link(color(m,(1.2,.8,.2,1)),wind,'B')
        prop(wind,'',unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    finish(m)

textured_solid('M_BattleLeavesWind',(.065,.145,.028),.8,leaves=True)
textured_solid('M_BattleRock',(.24,.235,.19),.92)
textured_solid('M_BattleMetal',(.14,.16,.17),.38,.6)
textured_solid('M_BattleShield',(.10,.16,.15),.65,.25)
textured_solid('M_BattleGrass',(.085,.14,.028),.95)
textured_solid('M_BattleRoof',(.12,.105,.08),.9)
textured_solid('M_BattleAllyCloth',(.035,.20,.23),.86)
textured_solid('M_BattleEnemyCloth',(.40,.055,.028),.86)
if not A.does_asset_exist(folder+'/M_BattleWater'):
    m=material('M_BattleWater')
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    prop(vertex,'',unreal.MaterialProperty.MP_BASE_COLOR)
    prop(constant(m,.18),'',unreal.MaterialProperty.MP_ROUGHNESS)
    prop(constant(m,.1),'',unreal.MaterialProperty.MP_METALLIC)
    position=node(m,unreal.MaterialExpressionWorldPosition)
    time=node(m,unreal.MaterialExpressionTime)
    waves=[]
    for direction in ((.03,.05,0,1),(.05,-.025,0,1)):
        dot=node(m,unreal.MaterialExpressionDotProduct)
        link(position,dot,'A');link(color(m,direction),dot,'B')
        add=node(m,unreal.MaterialExpressionAdd);link(dot,add,'A');link(time,add,'B')
        sine=node(m,unreal.MaterialExpressionSine);link(add,sine,'Input')
        scale=node(m,unreal.MaterialExpressionMultiply);link(sine,scale,'A');link(constant(m,.09),scale,'B')
        waves.append(scale)
    xy=node(m,unreal.MaterialExpressionAppendVector);link(waves[0],xy,'A');link(waves[1],xy,'B')
    xyz=node(m,unreal.MaterialExpressionAppendVector);link(xy,xyz,'A');link(constant(m,1),xyz,'B')
    normalize=node(m,unreal.MaterialExpressionNormalize);link(xyz,normalize,'VectorInput')
    prop(normalize,'',unreal.MaterialProperty.MP_NORMAL)
    m.set_editor_property('two_sided',True)
    finish(m)
for name in ('M_BattleGrass','M_BattleRoof'):
    m=A.load_asset(folder+'/'+name)
    if not m.get_editor_property('two_sided'):
        m.set_editor_property('two_sided',True)
        finish(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class=unreal.load_class(None,'/Script/GunyuTactics.TacticalField')
cinematic_lighting=any(a.get_class()==field_class and unreal.Name('GunyuCinemaV2') in a.get_editor_property('tags') for a in actors.get_all_level_actors())
for actor in actors.get_all_level_actors():
    if actor.get_class()==field_class:
        for field,old_names,name in (
            ('terrain_material',('M_Field','M_BattleGround'),'M_BattleGroundTextured'),
            ('foliage_material',('M_Trees','M_BattleLeaves'),'M_BattleLeavesWind'),
            ('ally_material',('M_Ally',),'M_BattleAllyCloth'),
            ('enemy_material',('M_Enemy',),'M_BattleEnemyCloth')):
            old=actor.get_editor_property(field)
            if not old or old.get_name() in old_names:
                actor.set_editor_property(field,A.load_asset(folder+'/'+name))
    if isinstance(actor,unreal.DirectionalLight) and not cinematic_lighting:
        light=actor.get_component_by_class(unreal.DirectionalLightComponent)
        light.set_editor_property('intensity',4.5)
        light.set_light_color(unreal.LinearColor(1,.88,.72,1))
        light.set_editor_property('light_source_angle',2)
    if isinstance(actor,unreal.SkyLight) and not cinematic_lighting:
        light=actor.get_component_by_class(unreal.SkyLightComponent)
        light.set_editor_property('intensity',1.2)
        light.set_light_color(unreal.LinearColor(.75,.85,1,1))
fogs=[a for a in actors.get_all_level_actors() if isinstance(a,unreal.ExponentialHeightFog)]
if not fogs:
    fog=actors.spawn_actor_from_class(unreal.ExponentialHeightFog,unreal.Vector(0,0,-50))
    fog.set_actor_label('Battle atmosphere - editable')
    component=fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    component.set_editor_property('fog_density',.003)
    component.set_editor_property('fog_height_falloff',.18)
    component.set_editor_property('start_distance',1800)
levels.save_current_level()
unreal.log('GUNYU_BATTLE_FINISH: PASS')
