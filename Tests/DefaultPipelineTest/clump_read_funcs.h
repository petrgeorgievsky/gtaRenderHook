#pragma once
#include <RWUtils/RwAPI.h>
#include <filesystem>

namespace RH_RWAPI
{

bool LoadClump( RpClump* &clump, const std::experimental::filesystem::path& dff_path );

}