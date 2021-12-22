#pragma once

#include "Vec3f.hpp"

struct Player {
	enum class TEAM : int {
		SPECT = 1,
		T = 2,
		CT = 3
	};

	unsigned m_arrayIndex;
	unsigned m_valid;    // if this is >0, then the described player is currently on the server
	char m_unknown[8];

	char m_name[32];      // most likely encoded with "CodePage" - no idea what that is

	TEAM m_team;
	int m_health;
	Vec3f m_pos;
	Vec3f m_viewangles;
	int m_unknown_3[60];  // 30x8 bytes - getting updated one after another after movement

	auto is_valid() const -> bool {
		return m_valid > 0;
	}

	auto isActive() const -> bool {
		return is_valid() &&
		       m_team != Player::TEAM::SPECT &&
		       m_health > 0;
	}
};