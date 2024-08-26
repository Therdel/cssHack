#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include <glm/fwd.hpp>

namespace overlay_structs {
    class LocalPlayer;
    class RadarStruct;
}

class GameVars {
private:
    GameVars();

public:
    using code_region_t = std::span<uint8_t>;

    static auto scan() -> GameVars;

    uint32_t& on_ground;
    // TODO: explain values - 4, 5, other.. enum?
    uint32_t& do_jump;
    uint32_t& do_attack_1;
    glm::vec3& player_pos;
    glm::vec3& angles;
    glm::vec3& angles_visual;
    uint8_t& is_ingame;
    uint32_t& is_inmenu;
    auto localplayer() -> std::optional<overlay_structs::LocalPlayer*>;
    auto localplayer() const -> std::optional<const overlay_structs::LocalPlayer*>;
    overlay_structs::RadarStruct& radar_struct;
    uintptr_t op_sdl_swapWindow_caller;
    uintptr_t op_sdl_pollEvent_caller;

    float& fov_horizontal_degrees;
    glm::mat4& mat_viewmodel;
    std::pair<int, int>& screen_dimensions;

private:
    uintptr_t& localplayer_base;
};