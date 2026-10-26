"""Create original procedural cue audio and missing presentation materials.
Existing art/material edits are preserved. Run via Unreal Python commandlet.
"""
import math
import random
import struct
import wave
from pathlib import Path
import unreal

assets = unreal.EditorAssetLibrary
editing = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

def material(name, color=None, effect=False):
    path = "/Game/Art/Materials/" + name
    if assets.does_asset_exist(path):
        return
    m = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())
    node = editing.create_material_expression(m, unreal.MaterialExpressionVertexColor if effect else unreal.MaterialExpressionConstant3Vector)
    if effect:
        m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        m.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        editing.connect_material_property(node, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        editing.connect_material_property(node, "A", unreal.MaterialProperty.MP_OPACITY)
    else:
        node.set_editor_property("constant", unreal.LinearColor(*color))
        editing.connect_material_property(node, "", unreal.MaterialProperty.MP_BASE_COLOR)
        rough = editing.create_material_expression(m, unreal.MaterialExpressionConstant)
        rough.set_editor_property("r", .72)
        editing.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    m.set_editor_property("two_sided", True)
    editing.set_material_usage(m, unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    editing.recompile_material(m)
    assets.save_loaded_asset(m)

material("M_Iron", (.065, .08, .085, 1))
material("M_Skin", (.43, .25, .13, 1))
material("M_Dust", (.25, .21, .12, 1))
material("M_BattleFX", effect=True)

root = Path(unreal.Paths.project_dir()).resolve() / "ArtSource" / "Audio"
root.mkdir(parents=True, exist_ok=True)
rate = 22050
for name, kind in (("S_March", 0), ("S_Command", 1), ("S_Impact", 2)):
    path = root / (name + ".wav")
    # Deterministic original percussion/air/impact cues, no third-party samples.
    rng = random.Random(711 + kind)
    samples = []
    duration = 1.4 if kind == 0 else .95
    for i in range(int(rate * duration)):
        t = i / rate
        noise = rng.uniform(-1, 1)
        if kind == 0:
            phase = t % .22
            value = math.exp(-phase * 35) * (.5 * math.sin(2 * math.pi * 85 * phase) + .13 * noise)
        elif kind == 1:
            value = math.exp(-t * 6) * (.6 * math.sin(2 * math.pi * (95 * t - 20 * t * t)) + .18 * noise)
            value += .09 * noise * math.sin(math.pi * min(t / duration, 1)) ** 2
        else:
            value = math.exp(-t * 9) * (.55 * math.sin(2 * math.pi * 61 * t) + .3 * noise + .12 * math.sin(2 * math.pi * 1253 * t))
        value *= min(t * 500, 1) * min((duration - t) * 100, 1)
        samples.append(int(max(-.85, min(.85, value)) * 32767))
    if not path.exists():
        with wave.open(str(path), "wb") as out:
            out.setnchannels(1); out.setsampwidth(2); out.setframerate(rate)
            out.writeframes(struct.pack("<" + "h" * len(samples), *samples))
    if not assets.does_asset_exist("/Game/Audio/" + name):
        task = unreal.AssetImportTask()
        task.filename = str(path); task.destination_path = "/Game/Audio"; task.destination_name = name
        task.automated = True; task.save = True; task.factory = unreal.SoundFactory()
        tools.import_asset_tasks([task])
    sound = assets.load_asset("/Game/Audio/" + name)
    assert isinstance(sound, unreal.SoundWave), name
    sound.set_sound_asset_compression_type(unreal.SoundAssetCompressionType.PCM)
    assets.save_loaded_asset(sound)
unreal.log("GUNYU_PRESENTATION_ASSETS: PASS")
