#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include <glm/fwd.hpp>

namespace overlay_structs {
    class LocalPlayer;
    class RadarStruct;
}

// credit: Aixxe
class CBaseHandle;
class IClientEntity;
class IClientNetworkable;
class IClientUnknown;
class IClientEntityList {
	public:
		virtual IClientNetworkable* GetClientNetworkable(int entindex) = 0;
		virtual IClientNetworkable* GetClientNetworkableFromHandle(CBaseHandle handle) = 0;
		virtual IClientUnknown* GetClientUnknownFromHandle(CBaseHandle handle) = 0;
		virtual IClientEntity* GetClientEntity(int entindex) = 0;
		virtual IClientEntity* GetClientEntityFromHandle(CBaseHandle handle) = 0;
		virtual int NumberOfEntities(bool include_non_networkable) = 0;
		virtual int GetHighestEntityIndex(void) = 0;
		virtual void SetMaxEntities(int max_entities) = 0;
		virtual int GetMaxEntities() = 0;
};

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
    std::pair<int, int>& screen_dimensions;

    IClientEntityList& clientEntityList;

private:
    uintptr_t localplayer_base;
};