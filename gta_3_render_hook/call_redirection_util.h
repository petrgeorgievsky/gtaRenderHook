//
// Created by peter on 30.10.2020.
//
#pragma once
#include <cstdint>

constexpr uint32_t Version_unknown  = 0xF;
constexpr uint32_t Version_1_0_en   = 0;
constexpr uint32_t Version_1_1_en   = 1;
constexpr uint32_t Version_Steam_en = 2;

uint32_t GetGameId() noexcept;

uint32_t GetAddressByGame( uint32_t v1_0, uint32_t v1_1,
                           uint32_t v_steam ) noexcept;