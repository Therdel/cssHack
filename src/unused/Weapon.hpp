#pragma once
#include "../Vec3f.hpp"

struct Weapon {
	uint8_t unknown_0[0xA0];
	int id;
	uint8_t unknown_1[0x804];
	int ammo;
};

struct WeaponEntry {
	uintptr_t unknown_0;
	Weapon *weapon;
	uintptr_t unknown_1[2];

	bool isPastEndEntry() const {
		return unknown_0 == 0;
	}
};