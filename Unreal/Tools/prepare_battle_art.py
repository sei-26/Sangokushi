"""Create editable battle materials; preserve existing custom material graphs."""
import unreal

assets = unreal.EditorAssetLibrary
edit = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

def solid(name, color, roughness=.85):
    if assets.does_asset_exist('/Game/Art/Materials/' + name):
        return
    mat = tools.create_asset(name, '/Game/Art/Materials', unreal.Material, unreal.MaterialFactoryNew())
    base = edit.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector)
    base.set_editor_property('constant', unreal.LinearColor(*color))
    edit.connect_material_property(base, '', unreal.MaterialProperty.MP_BASE_COLOR)
    rough = edit.create_material_expression(mat, unreal.MaterialExpressionConstant)
    rough.set_editor_property('r', roughness)
    edit.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    edit.set_material_usage(mat, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    edit.recompile_material(mat)
    assets.save_loaded_asset(mat)

solid('M_Bark', (.115, .066, .031, 1))
solid('M_Rock', (.24, .235, .21, 1))
solid('M_Grass', (.21, .26, .075, 1))
path = '/Game/Art/Materials/M_BattleGround'
if not assets.does_asset_exist(path):
    mat = tools.create_asset('M_BattleGround', '/Game/Art/Materials', unreal.Material, unreal.MaterialFactoryNew())
    vertex = edit.create_material_expression(mat, unreal.MaterialExpressionVertexColor, -600, 0)
    position = edit.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -1000, 220)
    noise = edit.create_material_expression(mat, unreal.MaterialExpressionNoise, -800, 220)
    noise.set_editor_property('scale', .12)
    noise.set_editor_property('levels', 2)
    noise.set_editor_property('quality', 1)
    noise.set_editor_property('output_min', .68)
    noise.set_editor_property('output_max', 1.16)
    edit.connect_material_expressions(position, '', noise, 'Position')
    multiply = edit.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 0)
    edit.connect_material_expressions(vertex, '', multiply, 'A')
    edit.connect_material_expressions(noise, '', multiply, 'B')
    edit.connect_material_property(multiply, '', unreal.MaterialProperty.MP_BASE_COLOR)
    rough = edit.create_material_expression(mat, unreal.MaterialExpressionConstant)
    rough.set_editor_property('r', .92)
    edit.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    mat.set_editor_property('two_sided', True)
    edit.recompile_material(mat)
    assets.save_loaded_asset(mat)

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level('/Game/Maps/TacticalTrial')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class = unreal.load_class(None, '/Script/GunyuTactics.TacticalField')
for actor in actors.get_all_level_actors():
    if actor.get_class() != field_class:
        continue
    old = actor.get_editor_property('terrain_material')
    # Only replace the stock bootstrap material. A user's custom asset remains assigned.
    if not old or old.get_path_name() == '/Game/Art/Materials/M_Field.M_Field':
        actor.set_editor_property('terrain_material', assets.load_asset(path))
levels.save_current_level()
unreal.log('GUNYU_BATTLE_ART: PASS')
