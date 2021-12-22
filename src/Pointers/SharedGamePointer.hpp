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

	auto address() const -> uintptr_t {
		return m_gamePointer->address();
	}

	auto update() -> void {
		m_gamePointer->update();
	}

	auto getGamePointer() const -> std::shared_ptr<GamePointer<T>> const& {
		return m_gamePointer;
	}

private:
	const std::shared_ptr<GamePointer<T>> m_gamePointer;
};