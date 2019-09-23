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

	address_type address() const {
		return derived().address();
	}

	void update() {
		return derived().update();
	}

	operator address_type() const {
		return address();
	}

	pointer_type pointer() const {
		return reinterpret_cast<pointer_type>(address());
	}

	reference_type operator*() const {
		return *pointer();
	}

	pointer_type operator->() const {
		return pointer();
	}

	operator pointer_type() const {
		return pointer();
	}

private:
	Derived &derived() {
		return static_cast<Derived &>(*this);
	}

	Derived const &derived() const {
		return static_cast<Derived const &>(*this);
	}
};