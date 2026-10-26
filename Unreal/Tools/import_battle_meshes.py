"""Import the original, reproducible battle meshes without overwriting edits."""
from pathlib import Path
import unreal

root = Path(__file__).resolve().parents[1] / 'ArtSource' / 'Battle'
names = ('SM_LeafCrown','SM_RockOutcrop','SM_LamellarArmor','SM_WarHelmet','SM_InfantryShield','SM_GrassClump','SM_GateRoof','SM_SpearheadV2','SM_CartWheelV2','SM_PalisadeStakeV4','SM_WatchtowerV4','SM_CampTentV4','SM_SupplyCrateV4','SM_WeatheredBoulderV5')
for name in names:
    path = '/Game/Art/Battle/' + name
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        options = unreal.FbxImportUI()
        options.set_editor_property('import_mesh',True)
        options.set_editor_property('import_as_skeletal',False)
        options.set_editor_property('import_materials',False)
        options.set_editor_property('import_textures',False)
        options.set_editor_property('automated_import_should_detect_type',False)
        options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)
        data=options.get_editor_property('static_mesh_import_data')
        data.set_editor_property('combine_meshes',True)
        data.set_editor_property('auto_generate_collision',False)
        data.set_editor_property('convert_scene',False)
        data.set_editor_property('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
        task=unreal.AssetImportTask()
        task.filename=str(root/(name+'.obj')); task.destination_path='/Game/Art/Battle'
        task.destination_name=name; task.automated=True; task.save=True
        task.factory=unreal.FbxFactory(); task.options=options
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=unreal.EditorAssetLibrary.load_asset(path)
    assert isinstance(mesh,unreal.StaticMesh), name
    bounds=mesh.get_bounding_box()
    unreal.log('GUNYU_MESH: %s bounds=%s' % (name,bounds))
    assert max(bounds.max.x-bounds.min.x,bounds.max.y-bounds.min.y,bounds.max.z-bounds.min.z)<160, name+' unexpected scale'
unreal.log('GUNYU_BATTLE_MESHES: PASS')
