"""Run with -ExecutePythonScript to check real editor assets and initial view."""
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert levels.load_level("/Game/Maps/TacticalTrial"), "Starting map must load in the editor"
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class = unreal.load_class(None, "/Script/GunyuTactics.TacticalField")
fields = [actor for actor in actors.get_all_level_actors() if actor.get_class() == field_class]
assert len(fields) == 1, "Exactly one editable tactical field is expected"
field = fields[0]
initial_rt=field.get_editor_property("realtime_command")
field.set_editor_property("realtime_command",not initial_rt)
assert field.get_editor_property("realtime_command")== (not initial_rt)
field.set_editor_property("realtime_command",initial_rt)
initial_alert_pause=field.get_editor_property("pause_on_battle_alert")
field.set_editor_property("pause_on_battle_alert",not initial_alert_pause)
assert field.get_editor_property("pause_on_battle_alert")== (not initial_alert_pause)
field.set_editor_property("pause_on_battle_alert",initial_alert_pause)
initial = field.get_editor_property("first_mission")
field.set_editor_property("first_mission", 1)
assert field.get_editor_property("first_mission") == 1
field.set_editor_property("first_mission", initial)
for prop in ("terrain_material", "water_material", "fortress_material", "distant_material", "ally_material", "enemy_material", "foliage_material", "objective_material", "standard_material", "rock_material"):
    assert field.get_editor_property(prop), "Editable art reference missing: " + prop
for prop in ("foliage_mesh", "rock_mesh", "armour_mesh", "war_helmet_mesh", "shield_mesh", "grass_mesh", "gate_roof_mesh", "spearhead_mesh", "cart_wheel_mesh", "palisade_mesh", "watchtower_mesh", "tent_mesh", "supply_crate_mesh"):
    assert isinstance(field.get_editor_property(prop), unreal.StaticMesh), "Editable battle mesh missing: " + prop
rocks=next(c for c in field.get_components_by_class(unreal.InstancedStaticMeshComponent) if c.get_name()=="RockOutcrops")
if initial==0:
    assert rocks.get_instance_count()==7, "Editor must rebuild the current ridge scenery after changing scenario properties"
foliage=field.get_editor_property("foliage_material")
if foliage.get_name() in ("M_CommandCanopyV4","M_LandscapeCanopyV5"):
    assert unreal.MaterialEditingLibrary.get_material_property_input_node(foliage,unreal.MaterialProperty.MP_OPACITY_MASK), "Observed-unit canopy mask must be connected"
    assert foliage.get_editor_property("blend_mode")==unreal.BlendMode.BLEND_MASKED
terrain = field.get_editor_property("terrain_material")
assert unreal.MaterialEditingLibrary.get_material_property_input_node(terrain, unreal.MaterialProperty.MP_BASE_COLOR), "Terrain colors must be connected"
for name in ("T_ShuPortraitSheet", "T_WeiPortraitSheet", "T_WuPortraitSheet"):
    texture = unreal.EditorAssetLibrary.load_asset("/Game/Art/Officers/" + name)
    assert isinstance(texture, unreal.Texture2D), "Portrait is not an imported texture: " + name
    assert texture.blueprint_get_size_x() > 0 and texture.blueprint_get_size_y() > 0
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(
    unreal.Vector(1600, 4050, 3600), unreal.Rotator(-53.13, -90, 0))
actors.set_selected_level_actors([field])
assert levels.save_current_level(), "Editor map must save successfully"
unreal.log("GUNYU_EDITOR_CHECK: PASS (editable map, scenario properties, ten material references, thirteen battle meshes, three portraits)")
