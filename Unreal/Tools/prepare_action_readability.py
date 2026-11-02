"""Versioned lit banner material with vertex opacity. Keep editable art intact."""
import unreal
A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
path='/Game/Art/Materials/M_ArmyStandardReadableV8'
if not A.does_asset_exist(path):
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_ArmyStandardReadableV8','/Game/Art/Materials',unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property('two_sided',True)
    m.set_editor_property('translucency_lighting_mode',unreal.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
    color=E.create_material_expression(m,unreal.MaterialExpressionVertexColor)
    rough=E.create_material_expression(m,unreal.MaterialExpressionConstant)
    rough.set_editor_property('r',.83)
    assert E.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    assert E.connect_material_property(color,'A',unreal.MaterialProperty.MP_OPACITY)
    assert E.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    E.layout_material_expressions(m);E.recompile_material(m);A.save_loaded_asset(m)
m=A.load_asset(path)
assert E.get_material_property_input_node(m,unreal.MaterialProperty.MP_OPACITY)
unreal.log('GUNYU_ACTION_MATERIAL: PASS (lit two-sided banner, vertex opacity, existing assets retained)')
