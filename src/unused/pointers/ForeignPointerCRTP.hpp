//
// Created by therdel on 04.09.19.
//
#pragma once

#include "../../Pointers/libNames.hpp"

// TODO Implement pull observer

// TODO Implementation alternative: Decorator pattern
//       with runtime polymorphism + caching

// TODO Implementation alternative: Different template design
//      libName client{"client.so"};
//      ForeignPointer<libNames::client, 0x352A> localplayer;
//      ForeignPointer<localplayer, 0x1, 0x02> teamIdx;

template<typename Derived, typename T=uintptr_t>
class ForeignPointer {
public:
	uintptr_t address() const {
		return derived().address();
	};

	T *pointer() {
		return reinterpret_cast<T *>(address());
	}

	T &operator*() {
		return *pointer();
	}

	T *operator->() {
		return pointer();
	}

	void update() {
		derived().update();
	}

private:

	/// CRTP convenience method
	Derived &derived() {
		return *static_cast<Derived *>(this);
	}

	Derived const &derived() const {
		return *static_cast<Derived const*>(this);
	}
};

class ForeignRawPtrBase : public ForeignPointer<ForeignRawPtrBase> {
public:
	explicit ForeignRawPtrBase(uintptr_t address)
			: m_address(address) {}

	uintptr_t address() const {
		return m_address;
	}

	static void update() {
		// nothing to update, constant base
	}

private:
	const uintptr_t m_address;
};

class ForeignLibraryPtrBase : public ForeignPointer<ForeignLibraryPtrBase> {
public:
	explicit ForeignLibraryPtrBase(LibName const &libName)
			: m_libName{libName}
			, m_cachedAddress(readLibraryBase()) {}

	// TODO: make lazy
	uintptr_t address() const {
		return m_cachedAddress;
	}

	void update() {
		m_cachedAddress = readLibraryBase();
	}

private:
	LibName const &m_libName;
	uintptr_t m_cachedAddress;

	uintptr_t readLibraryBase() const;
};

enum class OffsetType {
	DEREFERENCE,
	PLAIN_OFFSET
};

template<typename BasePtr,
		typename T=uintptr_t,
		typename U=uintptr_t>
class ForeignPtrOffset : public ForeignPointer<ForeignPtrOffset<BasePtr, T, U>, T> {
public:
	ForeignPtrOffset(ForeignPointer<BasePtr, U> &basePtr, ptrdiff_t offset, OffsetType type)
			: m_basePtr(basePtr)
			, m_offset(offset)
			, m_type(type)
			, m_cachedAddress(calculateAddress()) {}

	// TODO: make lazy
	uintptr_t address() const {
		return m_cachedAddress;
	}

	void update() {
		m_basePtr.update();
		m_cachedAddress = calculateAddress();
	}

private:
	ForeignPointer<BasePtr, U> &m_basePtr;
	ptrdiff_t m_offset;
	OffsetType m_type;
	uintptr_t m_cachedAddress;

	uintptr_t calculateAddress() const {
		auto *l_basePointer = reinterpret_cast<uintptr_t *>(m_basePtr.address() + m_offset);

		uintptr_t result;
		if (m_type == OffsetType::DEREFERENCE) {
			uintptr_t l_nextPointerAddress = *l_basePointer;
			result = l_nextPointerAddress;
		} else {
			result = reinterpret_cast<uintptr_t>(l_basePointer);
		}

		return result;
	}
};