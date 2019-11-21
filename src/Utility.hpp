//
// Created by therdel on 07.06.19.
//
#pragma once

#include <vector>
#include <string_view>

struct Vec3f;

namespace Utility {
	class NonCopyable {
	public:
		NonCopyable() = default;

		NonCopyable(NonCopyable const &) = delete;

		NonCopyable &operator=(NonCopyable const &) = delete;
	};

	class NonMovable {
	public:
		NonMovable() = default;

		NonMovable(NonMovable &&) = delete;

		NonMovable &operator=(NonMovable &&) = delete;
	};

// simple string split
// source: https://www.bfilipek.com/2018/07/string-view-perf-followup.html
// code: https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp
	std::vector<std::string_view> split(std::string_view str, std::string_view delims = " ");

	// returns the filename of given path
	// taken from https://www.oreilly.com/library/view/c-cookbook/0596007612/ch10s15.html
	std::string_view get_filename(std::string_view path);

	float toRadians(float degrees);

	float toDegrees(float radians);

	Vec3f rotateAroundZ(Vec3f const &vec, float yawDegrees);

	Vec3f viewAnglesToUnitvector(Vec3f const &angles);

	// returns the angle between given vectors in degrees
	float degreesBetweenVectors(const Vec3f &a, const Vec3f &b);
}
