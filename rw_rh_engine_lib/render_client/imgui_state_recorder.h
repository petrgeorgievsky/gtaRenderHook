//
// Created by peter on 23.02.2021.
//
#pragma once

namespace rh::rw::engine
{
struct ImGuiInputState;
void InstallWinProcHook();
void UpdateStateClient( ImGuiInputState &state );
void RemoveWinProcHook();
} // namespace rh::rw::engine