#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>  // CloseHandle

// RAII wrapper for a windows handle, calling CloseHandle(HANDLE) on destruct
class RAIIHandle {
private:
  HANDLE m_handle;

  // closes wrapped handle and invalidifies its value
  void reset() {
    if (isValid()) {
      CloseHandle(m_handle);
      m_handle = INVALID_HANDLE_VALUE;
    }
  }

public:
  explicit RAIIHandle(HANDLE h) : m_handle(h) {}
  // no copying, only one RAIIHandle per HANDLE
  RAIIHandle(const RAIIHandle &other) = delete;
  RAIIHandle(RAIIHandle &&other) noexcept : m_handle(other.getRaw()) {
    other.m_handle = INVALID_HANDLE_VALUE;
  }
  // disallow copying from an already wrapped handle
  RAIIHandle &operator=(const RAIIHandle &other) = delete;
  // wraps a naked handle, resetting the already wrapped one
  RAIIHandle &operator=(const HANDLE &other) {
    reset();
    m_handle = other;
    return *this;
  }

  // moves another handle into this one
  RAIIHandle &operator=(RAIIHandle &&other) {
    m_handle = other.m_handle;
    other.m_handle = INVALID_HANDLE_VALUE;
    return *this;
  }

  HANDLE getRaw() const {
    return m_handle;
  }

	bool isValid() const {
		return m_handle != INVALID_HANDLE_VALUE &&
		       m_handle != nullptr;
  }

  ~RAIIHandle() {
    reset();
  }
};
