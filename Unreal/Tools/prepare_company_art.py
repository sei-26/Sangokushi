"""Import versioned company art; preserve edited meshes, material graphs and overrides."""
from pathlib import Path
import runpy
import unreal

root = Path(__file__).resolve().parent
runpy.run_path(str(root/'make_company_meshes.py'),run_name='__main__')
A = unreal.EditorAssetLibrary
E = unreal.MaterialEditingLibrary
T = unreal.AssetToolsHelpers.get_asset_tools()
names = ('SM_LamellarArmorV7','SM_CommandSwordV7','SM_CommandGlaiveV7','SM_CommandSerpentV7')
for name in names:
    if not A.does_asset_exist('/Game/Art/Battle/'+name):
        options = unreal.FbxImportUI()
        for key,value in (('import_mesh',True),('import_as_skeletal',False),('import_materials',False),
                          ('import_textures',False),('automated_import_should_detect_type',False),
                          ('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)):
            options.set_editor_property(key,value)
        data = options.get_editor_property('static_mesh_import_data')
        for key,value in (('combine_meshes',True),('auto_generate_collision',False),('convert_scene',False),
                          ('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)):
            data.set_editor_property(key,value)
        task = unreal.AssetImportTask()
        task.filename = str(root.parent/'ArtSource'/'Battle'/(name+'.obj'))
        task.destination_path='/Game/Art/Battle';task.destination_name=name
        task.automated=True;task.save=True;task.factory=unreal.FbxFactory();task.options=options
        T.import_asset_tasks([task])
    mesh=A.load_asset('/Game/Art/Battle/'+name)
    assert isinstance(mesh,unreal.StaticMesh),name
    bounds=mesh.get_bounding_box()
    assert max(bounds.max.x-bounds.min.x,bounds.max.y-bounds.min.y,bounds.max.z-bounds.min.z)<160,name

path='/Game/Art/Materials/M_CompanyClothV7'
if not A.does_asset_exist(path):
    m=T.create_asset('M_CompanyClothV7','/Game/Art/Materials',unreal.Material,unreal.MaterialFactoryNew())
    def node(cls): return E.create_material_expression(m,cls)
    def link(a,b,name):
        inputs=E.get_material_expression_input_names(b)
        if name not in inputs and len(inputs)==1: name=inputs[0]
        assert E.connect_material_expressions(a,'',b,name),(name,inputs)
    channels=[]
    for i in range(3):
        n=node(unreal.MaterialExpressionPerInstanceCustomData)
        n.set_editor_property('data_index',i);n.set_editor_property('const_default_value',.15)
        channels.append(n)
    rg=node(unreal.MaterialExpressionAppendVector);link(channels[0],rg,'A');link(channels[1],rg,'B')
    rgb=node(unreal.MaterialExpressionAppendVector);link(rg,rgb,'A');link(channels[2],rgb,'B')
    interpolator=node(unreal.MaterialExpressionVertexInterpolator);link(rgb,interpolator,'Input')
    assert E.connect_material_property(interpolator,'',unreal.MaterialProperty.MP_BASE_COLOR)
    roughness=node(unreal.MaterialExpressionConstant);roughness.set_editor_property('r',.82)
    assert E.connect_material_property(roughness,'',unreal.MaterialProperty.MP_ROUGHNESS)
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.layout_material_expressions(m);E.recompile_material(m);A.save_loaded_asset(m)

levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
fields=[a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_class().get_name()=='TacticalField']
assert len(fields)==1
field=fields[0]
for prop,old_names,asset in (
    ('armour_mesh',('SM_LamellarArmor',),'/Game/Art/Battle/SM_LamellarArmorV7'),
    ('commander_sword_mesh',(),'/Game/Art/Battle/SM_CommandSwordV7'),
    ('commander_glaive_mesh',(),'/Game/Art/Battle/SM_CommandGlaiveV7'),
    ('commander_serpent_mesh',(),'/Game/Art/Battle/SM_CommandSerpentV7'),
    ('ally_material',('M_Ally','M_BattleAllyCloth'),path),
    ('enemy_material',('M_Enemy','M_BattleEnemyCloth'),path)):
    old=field.get_editor_property(prop)
    if not old or old.get_name() in old_names: field.set_editor_property(prop,A.load_asset(asset))
levels.save_current_level()
unreal.log('GUNYU_COMPANY_ART: PASS (four versioned meshes, instanced livery, editable overrides)')
