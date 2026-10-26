"""Editor entry point for rebuilding missing battle assets from original sources."""
from pathlib import Path
import runpy
import unreal

root=Path(__file__).resolve().parent
for filename in ('make_battle_meshes.py','make_battle_details.py','make_battlefield_props.py','import_battle_meshes.py','prepare_battle_art.py','prepare_battle_finish.py','prepare_battle_cinema.py','prepare_map_art.py','prepare_field_readability.py'):
    runpy.run_path(str(root/filename),run_name='__main__')
unreal.log('GUNYU_BATTLE_VISUALS: PASS')
