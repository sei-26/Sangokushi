"""Import original transparent tactic cut-ins; leave existing artwork intact."""
from pathlib import Path
import runpy
import unreal
runpy.run_path(str(Path(__file__).resolve().parent/'import_art.py'),run_name='__main__')
for name in ('T_LiuCutinV1','T_GuanCutinV1','T_ZhangCutinV1'):
    texture=unreal.EditorAssetLibrary.load_asset('/Game/Art/Officers/'+name)
    assert isinstance(texture,unreal.Texture2D),name
    texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property('never_stream',True)
    texture.set_editor_property('srgb',True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(texture),name
unreal.log('GUNYU_TACTIC_CUTINS: PASS (three original alpha portraits, editable textures, UI import)')
