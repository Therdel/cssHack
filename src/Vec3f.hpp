#pragma once

#include <cmath>    // std::sqrtf

struct Vec3ff {
	float m_x;
	float m_y;
	float m_z;

	constexpr Vec3ff(float x, float y, float z)
	: m_x(x)
	, m_y(y)
	, m_z(z) {
	}

	constexpr Vec3ff()
	: Vec3ff(0, 0, 0) {
	}

	auto operator-(const Vec3ff &other) const -> Vec3ff {
		return {m_x - other.m_x, m_y - other.m_y, m_z - other.m_z};
	}

	auto operator-=(const Vec3ff &other) -> Vec3ff& {
		m_x -= other.m_x;
		m_y -= other.m_y;
		m_z -= other.m_z;
		return *this;
	}

	auto operator+(const Vec3ff &other) const -> Vec3ff {
		return {m_x + other.m_x, m_y + other.m_y, m_z + other.m_z};
	}

	auto operator+=(const Vec3ff &other) -> Vec3ff& {
		m_x += other.m_x;
		m_y += other.m_y;
		m_z += other.m_z;
		return *this;
	}

	auto operator*(float scalar) const -> Vec3ff {
		return {m_x * scalar, m_y * scalar, m_z * scalar};
	}

	auto operator/(float scalar) const -> Vec3ff {
		return {m_x / scalar, m_y / scalar, m_z / scalar};
	}

	auto operator*=(float scalar) -> Vec3ff& {
		m_x *= scalar;
		m_y *= scalar;
		m_z *= scalar;
		return *this;
	}

	// dot product
	auto operator*(Vec3ff const &other) const -> float {
		return m_x * other.m_x +
		       m_y * other.m_y +
		       m_z * other.m_z;
	}

	auto operator==(Vec3ff const &other) const -> bool {
		return m_x == other.m_x &&
		       m_y == other.m_y &&
		       m_z == other.m_z;
	}

	auto operator!=(Vec3ff const &other) const -> bool {
		return !operator==(other);
	}

	auto length() const -> float {
		return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
	}

	// calculates the distance to another vector (the point that other points to)
	auto distanceTo(const Vec3ff &other) const -> float {
		Vec3ff l_difference = other - *this;
		return l_difference.length();
	}

	// returns this vector resized to length 1
	auto toUnitVector() const -> Vec3ff {
		float l_length = length();
		return {m_x / l_length, m_y / l_length, m_z / l_length};
	}
};
