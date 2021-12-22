//
// Created by therdel on 30.06.19.
//
#pragma once

struct Vec2f {
	float m_x;
	float m_y;

	auto operator*(float scalar) const -> Vec2f {
		return {m_x * scalar, m_y * scalar};
	}

	auto operator+(Vec2f const &other) const -> Vec2f {
		return {m_x + other.m_x,
		        m_y + other.m_y};
	}

	auto operator+=(Vec2f const &other) -> Vec2f& {
		m_x += other.m_x;
		m_y += other.m_y;
		return *this;
	}

	auto rotate(float radians) const -> Vec2f {
		float sin = std::sin(radians);
		float cos = std::cos(radians);

		return {m_x * cos - m_y * sin,
		        m_x * sin + m_y * cos};
	}
};
