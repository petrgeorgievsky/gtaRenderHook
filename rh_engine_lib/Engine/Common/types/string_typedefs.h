#pragma once
#include <iosfwd>
#include <string>
namespace rh::engine
{

#ifndef UNICODE
using String        = std::string;
using StringStream  = std::stringstream;
using OutFileStream = std::ofstream;
#else
using String        = std::wstring;
using StringStream  = std::wstringstream;
using OutFileStream = std::wofstream;
#endif

} // namespace rh::engine
