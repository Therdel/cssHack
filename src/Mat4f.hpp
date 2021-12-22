//
// Created by therdel on 29.06.19.
//
#pragma once

#include <array>

struct Vec4f {
	float m_x;
	float m_y;
	float m_z;
	float m_w;

	auto colMulRow(Vec4f const &other) const -> float {
		return m_x * other.m_x +
		       m_y * other.m_y +
		       m_z * other.m_z +
		       m_w * other.m_w;
	}

	auto operator*(float scalar) const -> Vec4f {
		return {m_x * scalar,
		        m_y * scalar,
		        m_z * scalar,
		        m_w * scalar};
	}

	auto  homogenous_to_cartesian() const -> Vec3f {
		return {m_x / m_w,
		        m_y / m_w,
		        m_z / m_w};
	}
};

struct Mat4f {
	union {
		std::array<float, 16> values;
		std::array<Vec4f, 4> columns;
	};

	auto matMulCol(Vec4f const &col) const -> Vec4f {
		Vec4f result{
				columns[0].colMulRow(col),
				columns[1].colMulRow(col),
				columns[2].colMulRow(col),
				columns[3].colMulRow(col)
		};

		return result;
	}
};