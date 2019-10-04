//
// Created by therdel on 05.09.19.
//
#pragma once

#include <cstdint>      // uintptr_t
#include <memory>       // std::enable_shared_from_this, std::shared_ptr
#include <type_traits>  // std::enable_if
#include <functional>

#include "GamePointerInterface.hpp"
#include "../MemoryUtils.hpp"
#include "libNames.hpp"

class GamePointerUntyped {
public:
	GamePointerUntyped(uintptr_t address)
			: m_cachedAddress(address)
			, m_dependentPointers() {}

	virtual ~GamePointerUntyped() = default;

	uintptr_t address() const {
		return m_cachedAddress;
	}

	/// recalculate the address and propagate the change to dependent pointers
	void update() {
		m_cachedAddress = calculateAddress();
		for (auto &dependent : m_dependentPointers) {
			if (auto sharedDependent = dependent.lock()) {
				sharedDependent->update();
			}
		}
	}

	void addDependent(std::weak_ptr<GamePointerUntyped> dependent) {
		m_dependentPointers.emplace_back(std::move(dependent));
	}

protected:
	virtual uintptr_t calculateAddress() const = 0;

private:
	uintptr_t m_cachedAddress;
	std::vector<std::weak_ptr<GamePointerUntyped>> m_dependentPointers;
};

template<typename T>
class GamePointer : public GamePointerUntyped,
                    public GamePointerInterface<T, GamePointer<T>> {
public:
	GamePointer(uintptr_t address)
			: GamePointerUntyped(address) {}

	uintptr_t address() const {
		return GamePointerUntyped::address();
	}

	void update() {
		GamePointerUntyped::update();
	}
};

class GamePointerBaseLibrary : public GamePointerUntyped {
public:
	explicit GamePointerBaseLibrary(std::string_view libName)
			: GamePointerUntyped(MemoryUtils::lib_base_32(libName))
			, m_libName{libName} {}

protected:
	uintptr_t calculateAddress() const override {
		return MemoryUtils::lib_base_32(m_libName);
	};

private:
	std::string_view m_libName;
};

class GamePointerBaseRaw : public GamePointerUntyped {
public:
	explicit GamePointerBaseRaw(uintptr_t address)
			: GamePointerUntyped(address)
			, m_address(address) {}

protected:
	uintptr_t calculateAddress() const override {
		return m_address;
	};

private:
	const uintptr_t m_address;
};

enum class OffsetType {
	DEREFERENCE,
	PLAIN_OFFSET
};

template<typename T>
class GamePointerOffset : public GamePointer<T> {
public:
	static std::shared_ptr<GamePointerOffset<T>> create(std::shared_ptr<GamePointerUntyped> basePtr,
	                                                    ptrdiff_t offset,
	                                                    OffsetType type) {
		auto ptr = std::shared_ptr<GamePointerOffset<T>>(new GamePointerOffset<T>(basePtr, offset, type));
		basePtr->addDependent(ptr);

		return ptr;
	}

protected:
	uintptr_t calculateAddress() const override {
		return calculateAddress(m_basePtr.get(), m_offset, m_type);
	}

private:
	std::shared_ptr<GamePointerUntyped> m_basePtr;
	ptrdiff_t m_offset;
	OffsetType m_type;

	GamePointerOffset(std::shared_ptr<GamePointerUntyped> basePtr, ptrdiff_t offset, OffsetType type) noexcept
			: GamePointer<T>(calculateAddress(basePtr.get(), offset, type))
			, m_basePtr(std::move(basePtr))
			, m_offset(offset)
			, m_type(type) {}

	static uintptr_t calculateAddress(GamePointerUntyped const *basePtr, ptrdiff_t offset, OffsetType type) {
		auto *l_basePointer = reinterpret_cast<uintptr_t *>(basePtr->address() + offset);

		uintptr_t result;
		if (type == OffsetType::DEREFERENCE) {
			uintptr_t l_nextPointerAddress = *l_basePointer;
			result = l_nextPointerAddress;
		} else {
			result = reinterpret_cast<uintptr_t>(l_basePointer);
		}

		return result;
	}
};