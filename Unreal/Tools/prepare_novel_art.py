"""Import only the original visual-novel assets. Existing artwork is preserved."""
from pathlib import Path
import json
import unreal
repo=Path(__file__).resolve().parents[2]
manifest=json.loads((repo/'Unreal/ArtSource/manifest.json').read_text(encoding='utf-8-sig'))
for item in manifest['assets']:
    if item.get('kind')!='visual-novel': continue
    source=(repo/item['source']).resolve()
    assert source.is_relative_to(repo) and source.is_file()
    destination=item['destination']+'/'+item['name']
    assert destination.startswith('/Game/Art/Novel/')
    if unreal.EditorAssetLibrary.does_asset_exist(destination): continue
    task=unreal.AssetImportTask()
    task.filename=str(source);task.destination_path=item['destination'];task.destination_name=item['name']
    task.automated=True;task.save=False;task.replace_existing=False;task.factory=unreal.TextureFactory()
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture=unreal.EditorAssetLibrary.load_asset(destination)
    assert isinstance(texture,unreal.Texture2D),destination
    texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property('never_stream',True);texture.set_editor_property('srgb',True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('GUNYU_NOVEL_ART: PASS (two full-bleed backgrounds, protagonist and civilian alpha sprites)')
