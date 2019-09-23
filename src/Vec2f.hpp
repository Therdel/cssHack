//
// Created by therdel on 30.06.19.
//
#pragma once

struct Vec2f {
	float m_x;
	float m_y;

	Vec2f operator*(float scalar) const {
		return {m_x * scalar, m_y * scalar};
	}

	Vec2f operator+(Vec2f const &other) const {
		return {m_x + other.m_x,
		        m_y + other.m_y};
	}

	Vec2f &operator+=(Vec2f const &other) {
		m_x += other.m_x;
		m_y += other.m_y;
		return *this;
	}

	Vec2f rotate(float radians) const {
		float sin = std::sin(radians);
		float cos = std::cos(radians);

		return {m_x * cos - m_y * sin,
		        m_x * sin + m_y * cos};
	}
};
