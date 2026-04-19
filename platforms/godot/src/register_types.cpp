#include "register_types.h"
#include "LintalkerSynth.h"
#include "LintalkerVoice.h"

#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void initialize_lintalker_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    ClassDB::register_class<LintalkerVoice>();
    ClassDB::register_class<LintalkerSynth>();
}

void uninitialize_lintalker_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {

GDExtensionBool GDE_EXPORT lintalker_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr   p_library,
    GDExtensionInitialization         *r_initialization)
{
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_lintalker_module);
    init_obj.register_terminator(uninitialize_lintalker_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
}

} // extern "C"
