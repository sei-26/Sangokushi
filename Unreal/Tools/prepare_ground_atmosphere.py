"""Versioned soft ground dust. Existing custom material graphs stay intact."""
import unreal

A=unreal.EditorAssetLibrary
E=unreal.MaterialEditingLibrary
path='/Game/Art/Materials/M_GroundDustV6'
if not A.does_asset_exist(path):
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_GroundDustV6','/Game/Art/Materials',unreal.Material,unreal.MaterialFactoryNew())
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property('two_sided',False)
    m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    def node(cls): return E.create_material_expression(m,cls)
    def scalar(name,value):
        n=node(unreal.MaterialExpressionScalarParameter)
        n.set_editor_property('parameter_name',name);n.set_editor_property('default_value',value);return n
    def link(a,b,name):
        names=E.get_material_expression_input_names(b)
        if name not in names:
            matches=[n for n in names if n.replace(' ','').lower()==name.removeprefix('In').lower()]
            name=matches[0] if matches else names[0] if len(names)==1 else name
        assert E.connect_material_expressions(a,'',b,name),(name,names)
    color=node(unreal.MaterialExpressionVectorParameter)
    color.set_editor_property('parameter_name','DustColor')
    color.set_editor_property('default_value',unreal.LinearColor(.38,.30,.195,1))
    assert E.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    fresnel=node(unreal.MaterialExpressionFresnel)
    fresnel.set_editor_property('exponent',2.2)
    fresnel.set_editor_property('base_reflect_fraction',0.)
    inverse=node(unreal.MaterialExpressionOneMinus);link(fresnel,inverse,'Input')
    density=node(unreal.MaterialExpressionMultiply);link(inverse,density,'A');link(scalar('Density',.38),density,'B')
    fade=node(unreal.MaterialExpressionDepthFade)
    link(density,fade,'InOpacity');link(scalar('GroundFade',18),fade,'FadeDistance')
    assert E.connect_material_property(fade,'',unreal.MaterialProperty.MP_OPACITY)
    E.set_material_usage(m,unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.layout_material_expressions(m);E.recompile_material(m);A.save_loaded_asset(m)

soft=A.load_asset(path)
assert soft and E.get_material_property_input_node(soft,unreal.MaterialProperty.MP_OPACITY)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/Game/Maps/TacticalTrial')
fields=[a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_class().get_name()=='TacticalField']
assert len(fields)==1
dust=[c for c in fields[0].get_components_by_class(unreal.InstancedStaticMeshComponent) if c.get_name()=='Dust']
assert len(dust)==1
old=dust[0].get_material(0)
if old is None or old.get_name() in ('M_Dust','M_GroundDustV6'):
    dust[0].set_material(0,soft)
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log('GUNYU_GROUND_ATMOSPHERE: PASS (versioned soft edges, ground fade, instancing, custom graphs retained)')
