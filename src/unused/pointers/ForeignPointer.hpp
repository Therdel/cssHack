//
// Created by therdel on 29.06.19.
//

#ifndef CSSHACK_FOREIGNPOINTER_HPP
#define CSSHACK_FOREIGNPOINTER_HPP

#include <cstdint>  // uintptr_t
#include <cstddef> // ptrdiff_t
#include <initializer_list>
#include <type_traits>
#include <optional>

#include "../../MemoryUtils.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../../Log.hpp"
#include "../../Pointers/libNames.hpp"

template<LibName &module>
class ForeignPointerBase {
private:
	uintptr_t m_value;

	void update() {
		std::optional<uintptr_t> l_module_base;
		l_module_base = MemoryUtils::get_lib_base_32(module);
		if (!l_module_base.has_value()) {
			Log::log(module, " base addr not found");
			throw std::runtime_error(module.name() + " base addr not found");
		}
		m_value = l_module_base.value();
	}

	uintptr_t value() const {
		return m_value;
	}

public:
	ForeignPointerBase()
			: m_value() {
		update();
	}
};

template<typename T>
class ForeignPointerBase {
	uintptr_t value() const {
		return T.value();
	}
};

template<typename T, ForeignPointerBase Base, ptrdiff_t ... offsets>
class ForeignPointer {
	static_assert(std::is_same_v<Base, const char *> ||
	              std::is_same_v<Base, ForeignPointer>);
private:
	T *m_value;
public:
	ForeignPointer()
			: m_value(nullptr) {
		update();
	}

	void update() {
		uintptr_t l_basePtr;
		if constexpr (std::is_same_v<Base, const char *>) {
		} else if constexpr (std::is_same_v<Base, ForeignPointer>) {
			Base base;
			l_basePtr = static_cast<uintptr_t>(base.value());
		}
		T *pointer = static_cast<uintptr_t>;

		std::initializer_list<ptrdiff_t> l_offsets;
		for (ptrdiff_t offset : l_offsets) {
			pointer = *static_cast<T *>(base + offset);
		}

	}

	T *value() const {
		return m_value;
	}

	T &operator*() const {
		return *m_value;
	}

	T *operator->() const {
		return m_value;
	}
};

#endif //CSSHACK_FOREIGNPOINTER_HPP
