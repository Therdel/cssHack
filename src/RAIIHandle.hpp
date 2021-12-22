#pragma once
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>        // CloseHandle

// RAII wrapper for a windows handle, calling CloseHandle(HANDLE) on destruct
class RAIIHandle {
private:
    HANDLE m_handle;

    // closes wrapped handle and invalidifies its value
    auto reset() -> void {
        if (isValid()) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
    }

public:
    explicit RAIIHandle(HANDLE h) : m_handle(h) {}
    // no copying, only one RAIIHandle per HANDLE
    RAIIHandle(const RAIIHandle& other) = delete;
    RAIIHandle(RAIIHandle&& other) noexcept : m_handle(other.getRaw()) {
        other.m_handle = INVALID_HANDLE_VALUE;
    }
    ~RAIIHandle() {
        reset();
    }

    // disallow copying from an already wrapped handle
    auto operator=(const RAIIHandle& other) -> RAIIHandle& = delete;
    // wraps a naked handle, resetting the already wrapped one
    auto operator=(const HANDLE& other) -> RAIIHandle& {
        reset();
        m_handle = other;
        return *this;
    }

    // moves another handle into this one
    auto operator=(RAIIHandle&& other) -> RAIIHandle& {
        m_handle = other.m_handle;
        other.m_handle = INVALID_HANDLE_VALUE;
        return *this;
    }

    auto getRaw() const -> HANDLE {
        return m_handle;
    }

    auto isValid() const -> bool {
        return m_handle != INVALID_HANDLE_VALUE &&
            m_handle != nullptr;
    }
};
