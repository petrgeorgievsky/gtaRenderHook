#pragma once
#include <string>
#include <vector>
namespace rh::engine
{
enum class InputElementType : uint32_t;
/*
    Vertex shader input data layout info, used for InputLayouts allocation
*/
struct InputElementInfo
{
    std::string      semantic;
    InputElementType type;
};

/*
    Vertex shader input data layout info, used for InputLayouts allocation
*/
struct InputLayoutInfo
{
    std::vector<InputElementInfo> inputElements;
    void *                        shaderPtr = nullptr;
};
} // namespace rh::engine
