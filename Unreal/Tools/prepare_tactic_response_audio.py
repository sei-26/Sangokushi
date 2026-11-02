"""Import missing original stings, preserving any existing edited SoundWaves."""
from pathlib import Path
import runpy
import unreal

generator = runpy.run_path(str(Path(__file__).with_name("make_tactic_response_audio.py")))
root = Path(unreal.Paths.project_dir()).resolve() / "ArtSource" / "Audio"
generator["generate"](root)
assets = unreal.EditorAssetLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
for name in generator["SOUNDS"]:
    path = "/Game/Audio/" + name
    if not assets.does_asset_exist(path):
        task = unreal.AssetImportTask()
        task.filename = str(root / (name + ".wav"))
        task.destination_path = "/Game/Audio"
        task.destination_name = name
        task.automated = True
        task.save = True
        task.factory = unreal.SoundFactory()
        tools.import_asset_tasks([task])
        sound = assets.load_asset(path)
        assert isinstance(sound, unreal.SoundWave), name
        sound.set_sound_asset_compression_type(unreal.SoundAssetCompressionType.PCM)
        assets.save_loaded_asset(sound)
    assert isinstance(assets.load_asset(path), unreal.SoundWave), name
unreal.log("GUNYU_RESPONSE_AUDIO: PASS (five original, editable, short response cues)")
