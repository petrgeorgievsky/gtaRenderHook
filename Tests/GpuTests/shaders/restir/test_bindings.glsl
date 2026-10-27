#ifndef RH_TEST_BINDINGS_GLSL
#define RH_TEST_BINDINGS_GLSL

#include "../../vulkan/engine/raycommon.glsl"
#include "../../vulkan/engine/restir/restir_structs.glsl"
#include "../../vulkan/engine/restir/restir_common.glsl"

// Globals expected by evaluate_pdf.glsl. Test kernels assign them in main().
vec4        SunDir;
int         LightsCount;
int         TriLightsCount;
PackedLight SceneLights[8];
PackedLight SceneTriLights[8];

// evaluate_pdf.glsl accesses scnDesc.i[instanceId].transfo, so the test side
// mirrors the production interface block with a plain global struct.
struct SceneDescTable
{
    sceneDesc i[8];
};
SceneDescTable scnDesc;

#endif
