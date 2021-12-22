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

		auto operator=(const NonCopyable&) -> NonCopyable& = delete;
		auto operator=(NonCopyable&&) -> NonCopyable& = default;
	};

	class NonMovable {
	public:
		NonMovable() = default;
		virtual ~NonMovable() noexcept = default;

		NonMovable(const NonMovable&) = default;
		NonMovable(NonMovable&&) = delete;

		auto operator=(const NonMovable&) -> NonMovable& = default;
		auto operator=(NonMovable&&) -> NonMovable& = delete;
	};

// simple string split
// source: https://www.bfilipek.com/2018/07/string-view-perf-followup.html
// code: https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp
	auto split(std::string_view str, std::string_view delims = " ") -> std::vector<std::string_view>;

	// returns the filename of given path
	// taken from https://www.oreilly.com/library/view/c-cookbook/0596007612/ch10s15.html
	auto get_filename(std::string_view path) -> std::string_view;

	auto toRadians(float degrees) -> float;

	auto toDegrees(float radians) -> float;

	auto rotateAroundZ(Vec3f const &vec, float yawDegrees) -> Vec3f;

	auto viewAnglesToUnitvector(Vec3f const &angles) -> Vec3f;

	// returns the angle between given vectors in degrees
	auto degreesBetweenVectors(const Vec3f &a, const Vec3f &b) -> float;

	class Offset final {
	public:
		constexpr explicit Offset(ptrdiff_t offset) : _offset{ offset } {}
		constexpr operator ptrdiff_t() const { return _offset; }

		constexpr auto operator+(ptrdiff_t offset) const -> Offset { return Offset{ _offset + offset }; }
		constexpr auto operator-(ptrdiff_t offset) const -> Offset { return Offset{ _offset - offset }; }
		constexpr auto operator+=(ptrdiff_t offset) -> Offset& { _offset += offset; return *this; }
		constexpr auto operator-=(ptrdiff_t offset) -> Offset& { _offset -= offset; return *this; }

	private:
		ptrdiff_t _offset;
	};

	class Address final {
	public:
		constexpr explicit Address(uintptr_t address) : _address{ address } {}
		constexpr operator uintptr_t const& () const { return _address; }

		constexpr auto operator+(Offset offset) const -> Address { return Address{ _address + offset }; }
		constexpr auto operator-(Offset offset) const -> Address { return Address{ _address - offset }; }
		constexpr auto operator+=(Offset offset) -> Address& { _address += offset; return *this; }
		constexpr auto operator-=(Offset offset) -> Address& { _address -= offset; return *this; }

		constexpr auto operator-(Address address) const -> Offset { return Offset{ static_cast<ptrdiff_t>(_address - address) }; }

		template<typename T>
		constexpr auto as_pointer() const -> T* { return reinterpret_cast<T*>(_address); }

	private:
		uintptr_t _address;
	};
}
