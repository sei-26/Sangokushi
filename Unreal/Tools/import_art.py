"""Import manifest-listed art in the editor, preserving existing assets by default.

Set GUNYU_REIMPORT_ART=1 explicitly to replace existing imported textures.
This is an editor production pipeline, never a runtime dependency.
"""
import json
import os
from pathlib import Path
import unreal

repo = Path(__file__).resolve().parents[2]
manifest = json.loads((repo / "Unreal/ArtSource/manifest.json").read_text(encoding="utf-8-sig"))
replace = os.environ.get("GUNYU_REIMPORT_ART") == "1"
tasks = []
for item in manifest["assets"]:
    source = (repo / item["source"]).resolve()
    if not source.is_file() or not source.is_relative_to(repo):
        raise ValueError("Missing or out-of-repository asset source: " + item["source"])
    destination = item["destination"]
    if not destination.startswith("/Game/Art/"):
        raise ValueError("Art must be imported below /Game/Art/")
    if not item.get("provenance"):
        raise ValueError("Record the source or generation provenance for each asset")
    if not replace and unreal.EditorAssetLibrary.does_asset_exist(destination + "/" + item["name"]):
        continue
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", item["name"])
    task.set_editor_property("replace_existing", replace)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    # Explicit texture factory also supports commandlets without a Slate app.
    # UE5.7's generic Interchange path can try to show its completion notification.
    task.set_editor_property("factory", unreal.TextureFactory())
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    if not task.get_editor_property("imported_object_paths"):
        raise RuntimeError("Import failed: " + task.get_editor_property("filename"))
unreal.log("GUNYU_ART_IMPORT: PASS (%d tasks)" % len(tasks))
