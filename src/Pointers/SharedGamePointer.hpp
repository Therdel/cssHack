//
// Created by therdel on 05.09.19.
//
#pragma once

#include "GamePointer.hpp"

template<typename T>
/// Provides a GamePointerInterface to a shared GamePointer
class SharedGamePointer : public GamePointerInterface<T, SharedGamePointer<T>> {
public:
	SharedGamePointer(std::shared_ptr<GamePointer<T>> gamePointer)
			: m_gamePointer(std::move(gamePointer)) {}

	uintptr_t address() const {
		return m_gamePointer->address();
	}

	void update() {
		m_gamePointer->update();
	}

	std::shared_ptr<GamePointer<T>> const &getGamePointer() const {
		return m_gamePointer;
	}

private:
	const std::shared_ptr<GamePointer<T>> m_gamePointer;
};