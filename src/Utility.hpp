//
// Created by therdel on 07.06.19.
//
#pragma once

#include <vector>
#include <string_view>
#include <cstdint>      // uintptr_t, ptrdiff_t

struct Vec3f;

namespace Util {
	class NonCopyable {
	public:
		NonCopyable() = default;
		virtual ~NonCopyable() noexcept = default;

		NonCopyable(const NonCopyable&) = delete;
		NonCopyable(NonCopyable&&) = default;

		NonCopyable& operator=(const NonCopyable&) = delete;
		NonCopyable& operator=(NonCopyable&&) = default;
	};

	class NonMovable {
	public:
		NonMovable() = default;
		virtual ~NonMovable() noexcept = default;

		NonMovable(const NonMovable&) = default;
		NonMovable(NonMovable&&) = delete;

		NonMovable &operator=(const NonMovable&) = default;
		NonMovable &operator=(NonMovable&&) = delete;
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

	class Offset final {
	public:
		constexpr explicit Offset(ptrdiff_t offset) : _offset{ offset } {}
		constexpr operator ptrdiff_t() const { return _offset; }

		constexpr auto operator+(ptrdiff_t offset) const -> Offset { return Offset{ _offset + offset }; }
		constexpr auto operator-(ptrdiff_t offset) const -> Offset { return Offset{ _offset - offset }; }
		auto operator+=(ptrdiff_t offset) -> Offset& { _offset += offset; return *this; }
		auto operator-=(ptrdiff_t offset) -> Offset& { _offset -= offset; return *this; }

	private:
		ptrdiff_t _offset;
	};

	class Address final {
	public:
		constexpr explicit Address(uintptr_t address) : _address{ address } {}
		constexpr operator uintptr_t const& () const { return _address; }

		constexpr auto operator+(Offset offset) const -> Address { return Address{ _address + offset }; }
		constexpr auto operator-(Offset offset) const -> Address { return Address{ _address - offset }; }
		auto operator+=(Offset offset) -> Address& { _address += offset; return *this; }
		auto operator-=(Offset offset) -> Address& { _address -= offset; return *this; }

		auto operator-(Address address) const -> Offset { return Offset{ static_cast<ptrdiff_t>(_address - address) }; }

		template<typename T>
		auto as_pointer() const -> T* { return reinterpret_cast<T*>(_address); }

	private:
		uintptr_t _address;
	};
}
