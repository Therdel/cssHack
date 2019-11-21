#pragma once

#include <cmath>    // std::sqrtf

struct Vec3f {
	float m_x;
	float m_y;
	float m_z;

	Vec3f(float x, float y, float z) noexcept {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	Vec3f() noexcept
			: Vec3f(0, 0, 0) {
	}

	Vec3f operator-(const Vec3f &other) const {
		return {m_x - other.m_x, m_y - other.m_y, m_z - other.m_z};
	}

	Vec3f &operator-=(const Vec3f &other) {
		m_x -= other.m_x;
		m_y -= other.m_y;
		m_z -= other.m_z;
		return *this;
	}

	Vec3f operator+(const Vec3f &other) const {
		return {m_x + other.m_x, m_y + other.m_y, m_z + other.m_z};
	}

	Vec3f &operator+=(const Vec3f &other) {
		m_x += other.m_x;
		m_y += other.m_y;
		m_z += other.m_z;
		return *this;
	}

	Vec3f operator*(float scalar) const {
		return {m_x * scalar, m_y * scalar, m_z * scalar};
	}

	Vec3f operator/(float scalar) const {
		return {m_x / scalar, m_y / scalar, m_z / scalar};
	}

	Vec3f &operator*=(float scalar) {
		m_x *= scalar;
		m_y *= scalar;
		m_z *= scalar;
		return *this;
	}

	// dot product
	float operator*(Vec3f const &other) const {
		return m_x * other.m_x +
		       m_y * other.m_y +
		       m_z * other.m_z;
	}

	bool operator==(Vec3f const &other) const {
		return m_x == other.m_x &&
		       m_y == other.m_y &&
		       m_z == other.m_z;
	}

	bool operator!=(Vec3f const &other) const {
		return !operator==(other);
	}

	float length() const {
		return std::sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
	}

	// calculates the distance to another vector (the point that other points to)
	float distanceTo(const Vec3f &other) const {
		Vec3f l_difference = other - *this;
		return l_difference.length();
	}

	// returns this vector resized to length 1
	Vec3f toUnitVector() const {
		float l_length = length();
		return {m_x / l_length, m_y / l_length, m_z / l_length};
	}
};
