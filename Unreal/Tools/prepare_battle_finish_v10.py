"""Import sculpted units and AI ground albedo; author an editable battle finish once."""
from pathlib import Path
import runpy
import unreal

root=Path(__file__).resolve().parent
runpy.run_path(str(root/'make_battle_finish_v10.py'),run_name='__main__')
A=unreal.EditorAssetLibrary;E=unreal.MaterialEditingLibrary;T=unreal.AssetToolsHelpers.get_asset_tools()
FOLDER='/Game/Art/Materials'
def node(m,c): return E.create_material_expression(m,c)
def scalar(m,value):
    n=node(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
def color(m,rgb):
    n=node(m,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(*rgb,1));return n
def link(a,b,pin,output=''):
    pins=E.get_material_expression_input_names(b)
    if pin not in pins and len(pins)==1: pin=pins[0]
    assert E.connect_material_expressions(a,output,b,pin),(pin,pins)
def binary(m,cls,a,b):
    n=node(m,cls);link(a,n,'A');link(b,n,'B');return n
def multiply(m,a,b): return binary(m,unreal.MaterialExpressionMultiply,a,b)
def add(m,a,b): return binary(m,unreal.MaterialExpressionAdd,a,b)
def finish(m):
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.recompile_material(m);E.layout_material_expressions(m);A.save_loaded_asset(m)

palette={'Skin':((.42,.265,.17),.82,0),'Features':((.048,.028,.018),.9,0),
         'Hair':((.018,.013,.010),.93,0),'Iron':((.22,.24,.25),.52,.35),
         'Bronze':((.28,.19,.08),.52,.45),'Leather':((.065,.039,.023),.88,0),
         'Wood':((.12,.083,.043),.9,0),'Cloth':((.15,.15,.15),.88,0)}
for key,(rgb,rough,metal) in palette.items():
    name='M_Battle'+key+'V10';path=FOLDER+'/'+name
    if A.does_asset_exist(path): continue
    m=T.create_asset(name,FOLDER,unreal.Material,unreal.MaterialFactoryNew())
    base=color(m,rgb)
    if key=='Cloth':
        channels=[]
        for i in range(3):
            n=node(m,unreal.MaterialExpressionPerInstanceCustomData);n.set_editor_property('data_index',i);n.set_editor_property('const_default_value',.15);channels.append(n)
        rg=binary(m,unreal.MaterialExpressionAppendVector,channels[0],channels[1])
        all_rgb=binary(m,unreal.MaterialExpressionAppendVector,rg,channels[2])
        base=node(m,unreal.MaterialExpressionVertexInterpolator);link(all_rgb,base,'Input')
    random=node(m,unreal.MaterialExpressionPerInstanceRandom)
    shade=add(m,scalar(m,.86),multiply(m,random,scalar(m,.23)))
    assert E.connect_material_property(multiply(m,base,shade),'',unreal.MaterialProperty.MP_BASE_COLOR)
    assert E.connect_material_property(scalar(m,rough),'',unreal.MaterialProperty.MP_ROUGHNESS)
    assert E.connect_material_property(scalar(m,metal),'',unreal.MaterialProperty.MP_METALLIC)
    m.set_editor_property('two_sided',True);finish(m)

# One authored revision of our new surfaces; subsequent runs preserve edits.
for key in ('Iron','Bronze'):
    m=A.load_asset(FOLDER+'/M_Battle'+key+'V10')
    if A.get_metadata_tag(m,'GunyuFinishToneV10B')=='1': continue
    base=E.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    rgb,rough,metal=palette[key]
    colors=[n for n in E.get_inputs_for_material_expression(m,base) if isinstance(n,unreal.MaterialExpressionConstant3Vector)]
    assert len(colors)==1,key
    colors[0].set_editor_property('constant',unreal.LinearColor(*rgb,1))
    for prop,value in ((unreal.MaterialProperty.MP_ROUGHNESS,rough),(unreal.MaterialProperty.MP_METALLIC,metal)):
        n=E.get_material_property_input_node(m,prop);assert isinstance(n,unreal.MaterialExpressionConstant);n.set_editor_property('r',value)
    A.set_metadata_tag(m,'GunyuFinishToneV10B','1');finish(m)

names=('SM_TailoredClothV10','SM_InfantryHeadV10','SM_LamellarArmorV10','SM_WarHelmetV10','SM_CommandBeardV10','SM_CommandCapeV10','SM_InfantryShieldV10','SM_FieldCanopyV10')
for name in names:
    path='/Game/Art/Battle/'+name
    if not A.does_asset_exist(path):
        options=unreal.FbxImportUI()
        for prop,value in (('import_mesh',True),('import_as_skeletal',False),('import_materials',False),('import_textures',False),('automated_import_should_detect_type',False),('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)):
            options.set_editor_property(prop,value)
        data=options.get_editor_property('static_mesh_import_data')
        for prop,value in (('combine_meshes',True),('auto_generate_collision',False),('convert_scene',False),('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)):
            data.set_editor_property(prop,value)
        task=unreal.AssetImportTask();task.filename=str(root.parent/'ArtSource'/'Battle'/(name+'.obj'))
        task.destination_path='/Game/Art/Battle';task.destination_name=name;task.automated=True;task.save=True;task.factory=unreal.FbxFactory();task.options=options
        T.import_asset_tasks([task])
        mesh=A.load_asset(path);assert isinstance(mesh,unreal.StaticMesh),name
        for i,slot in enumerate(mesh.get_editor_property('static_materials')):
            key=str(slot.get_editor_property('material_slot_name')).split(':')[-1]
            assert key in palette or key=='Leaves',(name,key)
            mesh.set_material(i,A.load_asset(FOLDER+('/M_LandscapeCanopyV5' if key=='Leaves' else '/M_Battle'+key+'V10')))
        A.save_loaded_asset(mesh)
    mesh=A.load_asset(path);bounds=mesh.get_bounding_box()
    assert 80<bounds.max.z-bounds.min.z<120,(name,bounds)
unreal.log('GUNYU_FINISH_MODELS: PASS (eight original models, imported normals and material sections)')

tex_path='/Game/Art/Battle/T_BattleGroundV10'
if not A.does_asset_exist(tex_path):
    task=unreal.AssetImportTask();task.filename=str(root.parent/'ArtSource'/'Battle'/'ground-albedo-v10.png')
    task.destination_path='/Game/Art/Battle';task.destination_name='T_BattleGroundV10';task.automated=True;task.save=True;T.import_asset_tasks([task])
texture=A.load_asset(tex_path);assert isinstance(texture,unreal.Texture2D)
texture.set_editor_property('power_of_two_mode',unreal.TexturePowerOfTwoSetting.RESIZE_TO_SPECIFIC_RESOLUTION)
texture.set_editor_property('resize_during_build_x',1024);texture.set_editor_property('resize_during_build_y',1024)
A.save_loaded_asset(texture)
name='M_BattleGroundV10'
if not A.does_asset_exist(FOLDER+'/'+name):
    m=T.create_asset(name,FOLDER,unreal.Material,unreal.MaterialFactoryNew())
    world=node(m,unreal.MaterialExpressionWorldPosition)
    xy=node(m,unreal.MaterialExpressionComponentMask);xy.set_editor_property('r',True);xy.set_editor_property('g',True);xy.set_editor_property('b',False);xy.set_editor_property('a',False);link(world,xy,'Input')
    uv=multiply(m,xy,scalar(m,1/300))
    def sample(coordinates):
        n=node(m,unreal.MaterialExpressionTextureSample);n.set_editor_property('texture',texture);link(coordinates,n,'UVs');return n
    detail=sample(uv);macro=sample(multiply(m,xy,scalar(m,1/1700)))
    blend=node(m,unreal.MaterialExpressionLinearInterpolate);link(detail,blend,'A','RGB');link(macro,blend,'B','RGB');link(scalar(m,.15),blend,'Alpha')
    vertex=node(m,unreal.MaterialExpressionVertexColor)
    tint=add(m,color(m,(.65,.65,.65)),multiply(m,vertex,color(m,(2.5,2.5,2.5))))
    base=multiply(m,blend,tint)
    assert E.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
    assert E.connect_material_property(scalar(m,.94),'',unreal.MaterialProperty.MP_ROUGHNESS)
    gradients=[]
    for offset in ((1/1024,0),(0,1/1024)):
        d=node(m,unreal.MaterialExpressionConstant2Vector);d.set_editor_property('r',offset[0]);d.set_editor_property('g',offset[1])
        adjacent=sample(add(m,uv,d));delta=node(m,unreal.MaterialExpressionSubtract);link(detail,delta,'A','R');link(adjacent,delta,'B','R');gradients.append(multiply(m,delta,scalar(m,1.8)))
    gradient=binary(m,unreal.MaterialExpressionAppendVector,gradients[0],gradients[1])
    vector=binary(m,unreal.MaterialExpressionAppendVector,gradient,scalar(m,1))
    normal=node(m,unreal.MaterialExpressionNormalize);link(vector,normal,'VectorInput')
    assert E.connect_material_property(normal,'',unreal.MaterialProperty.MP_NORMAL)
    finish(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);assert levels.load_level('/Game/Maps/TacticalTrial')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
fields=[a for a in actors.get_all_level_actors() if a.get_class().get_name()=='TacticalField'];assert len(fields)==1
field=fields[0]
for prop,old_names,new in (
    ('infantry_cloth_mesh',(),'SM_TailoredClothV10'),('infantry_head_mesh',(),'SM_InfantryHeadV10'),
    ('commander_beard_mesh',(),'SM_CommandBeardV10'),('commander_cape_mesh',(),'SM_CommandCapeV10'),
    ('armour_mesh',('SM_LamellarArmor','SM_LamellarArmorV7'),'SM_LamellarArmorV10'),
    ('war_helmet_mesh',('SM_WarHelmet',),'SM_WarHelmetV10'),('shield_mesh',('SM_InfantryShield',),'SM_InfantryShieldV10'),
    ('foliage_mesh',('SM_LeafCrown',),'SM_FieldCanopyV10')):
    old=field.get_editor_property(prop)
    if not old or old.get_name() in old_names: field.set_editor_property(prop,A.load_asset('/Game/Art/Battle/'+new))
for prop,old_names,new in (
    ('terrain_material',('M_Field','M_BattleGroundTextured','M_GroundCinematicV2','M_MapGroundV3','M_LandscapeGroundV5'),'M_BattleGroundV10'),
    ('ally_material',('M_CompanyClothV7',),'M_BattleClothV10'),('enemy_material',('M_CompanyClothV7',),'M_BattleClothV10')):
    old=field.get_editor_property(prop)
    if not old or old.get_name() in old_names: field.set_editor_property(prop,A.load_asset(FOLDER+'/'+new))

tag=unreal.Name('GunyuBattleFinishV10LightB');tags=list(field.get_editor_property('tags'))
if tag not in tags:
    for actor in actors.get_all_level_actors():
        if isinstance(actor,unreal.DirectionalLight):
            light=actor.get_component_by_class(unreal.DirectionalLightComponent)
            actor.set_actor_rotation(unreal.Rotator(-37,-38,0),False)
            for prop,value in (('intensity',4.6),('light_source_angle',1.8),('shadow_bias',.25),('contact_shadow_length',.18),('dynamic_shadow_distance_movable_light',10000),('dynamic_shadow_cascades',4)):
                light.set_editor_property(prop,value)
            light.set_light_color(unreal.LinearColor(1,.955,.875,1))
        if isinstance(actor,unreal.SkyLight):
            light=actor.get_component_by_class(unreal.SkyLightComponent);light.set_editor_property('intensity',1.7);light.set_light_color(unreal.LinearColor(.80,.88,1,1))
    tags.append(tag);field.set_editor_property('tags',tags)
levels.save_current_level()
unreal.log('GUNYU_BATTLE_FINISH_V10: PASS (sculpted units, restrained standards, world-scale ground, directional/contact light, preserved custom references)')
