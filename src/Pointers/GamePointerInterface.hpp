//
// Created by therdel on 05.09.19.
//
#pragma once

template<typename T, typename Derived>
class GamePointerInterface {
public:
	using address_type = uintptr_t;
	using value_type = T;
	using reference_type = T &;
	using pointer_type = T *;

	auto address() const -> address_type {
		return derived().address();
	}

	auto update() -> void {
		return derived().update();
	}

	operator address_type() const {
		return address();
	}

	auto pointer() const -> pointer_type {
		return reinterpret_cast<pointer_type>(address());
	}

	auto operator*() const -> reference_type {
		return *pointer();
	}

	auto operator->() const -> pointer_type {
		return pointer();
	}

	operator pointer_type() const {
		return pointer();
	}

private:
	auto derived() -> Derived& {
		return static_cast<Derived &>(*this);
	}

	auto derived() const -> Derived const& {
		return static_cast<Derived const &>(*this);
	}
};