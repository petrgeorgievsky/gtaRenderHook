#pragma once
#include <cstdint>

namespace rh::engine {

enum class BlendOp : uint8_t {
    Zero = 0,
    One = 1,
    SrcColor = 2,
    InvSrcColor = 3,
    SrcAlpha = 4,
    InvSrcAlpha = 5,
    DestAlpha = 6,
    InvDestAlpha = 7,
    DestColor = 8,
    InvDestColor = 9,
    SrcAlphaSat = 10,
    BlendFactor = 11,
    Src1Color = 12,
    InvSrc1Color = 13,
    Src1Alpha = 14,
    InvSrc1Alpha = 15
};

enum class BlendCombineOp : uint8_t {
    Add = 0,
    Substract = 1,
    RevSubstract = 2,
    Min = 3,
    Max = 4
};

}
