"""Editor-only: generate reproducible trial materials and a small starting map.

Run with UnrealEditor-Cmd -run=pythonscript -script=<this file>.
No remote assets or marketplace dependencies are downloaded.
"""
import unreal

tools = unreal.AssetToolsHelpers.get_asset_tools()
library = unreal.EditorAssetLibrary


def material(name, color=None):
    path = "/Game/Art/Materials/" + name
    if library.does_asset_exist(path):
        result = library.load_asset(path)
        if color is None and not unreal.MaterialEditingLibrary.get_material_property_input_node(result, unreal.MaterialProperty.MP_BASE_COLOR):
            expression = unreal.MaterialEditingLibrary.create_material_expression(result, unreal.MaterialExpressionVertexColor, -200, 0)
            assert unreal.MaterialEditingLibrary.connect_material_property(expression, "", unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.set_material_usage(result, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
        unreal.MaterialEditingLibrary.recompile_material(result)
        library.save_loaded_asset(result)
        return result
    result = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())
    expression_type = unreal.MaterialExpressionVertexColor if color is None else unreal.MaterialExpressionConstant3Vector
    expression = unreal.MaterialEditingLibrary.create_material_expression(result, expression_type, -200, 0)
    if color is not None:
        expression.set_editor_property("constant", unreal.LinearColor(*color))
    assert unreal.MaterialEditingLibrary.connect_material_property(expression, "", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(result, unreal.MaterialExpressionConstant, -200, 150)
    roughness.set_editor_property("r", 0.85)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    result.set_editor_property("two_sided", True)
    unreal.MaterialEditingLibrary.set_material_usage(result, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    unreal.MaterialEditingLibrary.recompile_material(result)
    library.save_loaded_asset(result)
    return result


material("M_Field")
material("M_Ally", (0.08, 0.35, 0.55, 1))
material("M_Enemy", (0.64, 0.14, 0.07, 1))
material("M_Trees", (0.10, 0.20, 0.065, 1))
material("M_Goal", (0.80, 0.51, 0.075, 1))

map_path = "/Game/Maps/TacticalTrial"
if not library.does_asset_exist(map_path):
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    levels.new_level(map_path)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1500), unreal.Rotator(-55, -35, 0))
    sun.light_component.set_editor_property("intensity", 5.0)
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 800))
    sky.light_component.set_editor_property("intensity", 1.5)
    sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    cubemap = library.load_asset("/Engine/EngineResources/DefaultTextureCube.DefaultTextureCube")
    if cubemap:
        sky.light_component.set_editor_property("cubemap", cubemap)
    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector())
    field_class = unreal.load_class(None, "/Script/GunyuTactics.TacticalField")
    field = actors.spawn_actor_from_class(field_class, unreal.Vector())
    field.set_actor_label("Tactical Field - select to edit Trial settings")
    for prop, name in (("terrain_material", "M_Field"), ("ally_material", "M_Ally"), ("enemy_material", "M_Enemy"), ("foliage_material", "M_Trees"), ("objective_material", "M_Goal")):
        field.set_editor_property(prop, library.load_asset("/Game/Art/Materials/" + name))
    levels.save_current_level()
unreal.log("GUNYU_BOOTSTRAP: PASS")
