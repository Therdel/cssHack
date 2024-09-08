//
// Created by therdel on 07.06.19.
//
#include "Utility.hpp"

#include <cmath>
#include <vector>
#include <algorithm>    // std::find_first_of

#include <glm/geometric.hpp> // glm::dot

namespace Util {
	static const float PI = static_cast<float>(std::acos(-1));

// simple string split
// source: https://www.bfilipek.com/2018/07/string-view-perf-followup.html
// code: https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp
	auto split(std::string_view str, std::string_view delims) -> std::vector<std::string_view> {
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
	auto get_filename(std::string_view path) -> std::string_view {

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

	auto toRadians(float degrees) -> float {
		return (degrees * PI) / 180.0f;
	}

	auto toDegrees(float radians) -> float{
		return (radians * 180.0f) / PI;
	}


	auto rotateAroundZ(glm::vec3 const &vec, float yawDegrees) -> glm::vec3 {
		float yawRad = toRadians(yawDegrees);
		// rotate around z axis
		// source: https://de.wikipedia.org/wiki/Drehmatrix
		auto rotated = glm::vec3{
				vec.x * std::cos(yawRad) + -vec.y * std::sin(yawRad),
				vec.x * std::sin(yawRad) + vec.y * std::cos(yawRad),
				vec.z
		};
		return rotated;
	}

	auto viewAnglesToUnitvector(glm::vec3 const &angles) -> glm::vec3 {
		// determine crosshair unit vector
		// that is dependent on the players viewangles
		// TODO: Clarify angle conversions
		float l_yawRad = toRadians(angles.y + 180);
		float l_pitchRad = toRadians(angles.x - 90);
		const glm::vec3 l_unitVector{
				std::sin(l_pitchRad) * std::cos(l_yawRad),  // x
				std::sin(l_pitchRad) * std::sin(l_yawRad),  // y
				std::cos(l_pitchRad) *
				-1                   // z, has to be negated, somehow the axis is flipped using this calculation
		};

		return l_unitVector;
	}

	auto degreesBetweenVectors(const glm::vec3 &a, const glm::vec3 &b) -> float {
		// derive the angle using the dot product
		const float l_dotProduct = glm::dot(a, b);
		const float l_angleRad = std::acos(l_dotProduct / (glm::length(a) * glm::length(b)));
		// check if acos returned a nan value
		if (std::isnan(l_angleRad)) {
			// this means that both vectors directions were very similar - we'll return 0
			return 0;
		} else {
			return toDegrees(l_angleRad);
		}
	}
}
