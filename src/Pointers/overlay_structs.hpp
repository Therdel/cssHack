#pragma once
#include <array>
#include <cstdint>

#include <glm/vec3.hpp>

#include "Offsets.hpp"

namespace overlay_structs {
    struct Player {
        enum class TEAM : int {
            SPECT = 1,
            T = 2,
            CT = 3
        };

        uint32_t m_arrayIndex;
        uint32_t m_valid;    // if this is >0, then the described player is currently on the server
        char m_unknown[8];

        char m_name[32];      // most likely encoded with "CodePage" - no idea what that is

        TEAM m_team;
        int m_health;
        glm::vec3 m_pos;
        glm::vec3 m_viewangles;
        int64_t m_footstep_history[30];  // 30x8 bytes - getting updated one after another after movement

        auto is_valid() const -> bool {
            return m_valid > 0;
        }

        auto isActive() const -> bool {
            return is_valid() &&
                m_team != Player::TEAM::SPECT &&
                m_health > 0;
        }
    };

    struct LocalPlayer {
        union {
            struct {
                uint8_t padding[Offsets::localplayer_off_team];
                Player::TEAM value;
            } team;

            struct {
                uint8_t padding[Offsets::localplayer_off_punch];
                glm::vec3 value;
            } punch_angles;

            struct {
                uint8_t padding[Offsets::localplayer_off_targetId];
                uint32_t value;
            } target_id;
        };
    };

    struct RadarStruct {
        uint8_t padding[Offsets::radarstruct_playerArray_off];
        std::array<Player, 64> players;
    };
}