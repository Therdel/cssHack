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

	float colMulRow(Vec4f const &other) const {
		return m_x * other.m_x +
		       m_y * other.m_y +
		       m_z * other.m_z +
		       m_w * other.m_w;
	}

	Vec4f operator*(float scalar) const {
		return {m_x * scalar,
		        m_y * scalar,
		        m_z * scalar,
		        m_w * scalar};
	}

	Vec3f homogenous_to_cartesian() const {
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

	Vec4f matMulCol(Vec4f const &col) const {
		Vec4f result{
				columns[0].colMulRow(col),
				columns[1].colMulRow(col),
				columns[2].colMulRow(col),
				columns[3].colMulRow(col)
		};

		return result;
	}
};