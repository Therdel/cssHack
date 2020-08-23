//
// Created by therdel on 07.06.19.
//
#include "Utility.hpp"

#include <cmath>
#include <vector>
#include <algorithm>    // std::find_first_of

#include "Vec3f.hpp"

namespace Util {
	static const float PI = static_cast<float>(std::acos(-1));

// simple string split
// source: https://www.bfilipek.com/2018/07/string-view-perf-followup.html
// code: https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp
	std::vector<std::string_view> split(std::string_view str, std::string_view delims) {
		std::vector<std::string_view> output;
		//output.reserve(str.size() / 2);

		for (const char *first = str.data(), *second = str.data(), *last = first + str.size();
		     second != last && first != last; first = second + 1) {
			second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

			if (first != second) {
				output.emplace_back(first, second - first);
			}
		}

		return output;
	}


	// returns the filename of given path
	// taken from https://www.oreilly.com/library/view/c-cookbook/0596007612/ch10s15.html
	std::string_view get_filename(std::string_view path) {

#ifdef __linux__
		char sep = '/';
#else
		char sep = '\\';
#endif

		size_t i = path.rfind(sep, path.length());
		if (i != std::string_view::npos) {
			return path.substr(i + 1, path.length() - i);
		} else {
			return path;
		}
	}

	float toRadians(float degrees) {
		return (degrees * PI) / 180.0f;
	}

	float toDegrees(float radians) {
		return (radians * 180.0f) / PI;
	}


	Vec3f rotateAroundZ(Vec3f const &vec, float yawDegrees) {
		float yawRad = toRadians(yawDegrees);
		// rotate around z axis
		// source: https://de.wikipedia.org/wiki/Drehmatrix
		Vec3f rotated{
				vec.m_x * std::cos(yawRad) + -vec.m_y * std::sin(yawRad),
				vec.m_x * std::sin(yawRad) + vec.m_y * std::cos(yawRad),
				vec.m_z
		};
		return rotated;
	}

	Vec3f viewAnglesToUnitvector(Vec3f const &angles) {
		// determine crosshair unit vector
		// that is dependent on the players viewangles
		// TODO: Clarify angle conversions
		float l_yawRad = toRadians(angles.m_y + 180);
		float l_pitchRad = toRadians(angles.m_x - 90);
		const Vec3f l_unitVector{
				std::sin(l_pitchRad) * std::cos(l_yawRad),  // x
				std::sin(l_pitchRad) * std::sin(l_yawRad),  // y
				std::cos(l_pitchRad) *
				-1                   // z, has to be negated, somehow the axis is flipped using this calculation
		};

		return l_unitVector;
	}

	float degreesBetweenVectors(const Vec3f &a, const Vec3f &b) {
		// derive the angle using the dot product
		const float l_dotProduct = a * b;
		const float l_angleRad = std::acos(l_dotProduct / (a.length() * b.length()));
		// check if acos returned a nan value
		if (std::isnan(l_angleRad)) {
			// this means that both vectors directions were very similar - we'll return 0
			return 0;
		} else {
			return toDegrees(l_angleRad);
		}
	}
}
