#pragma once

#include <optional>
#include <chrono>
#include <string>

#include "Utility.hpp"
#include "Pointers/libNames.hpp"

class MemoryUtils {
private:
	/// describes a memory area protection
	struct MemoryProtection {
		uintptr_t address;
		size_t length;
#ifdef __linux__
		int protection;
#else // windows
		uintptr_t protection;
#endif
	};// TODO? remove unused

//
//public:
//	enum class PROT : uint8_t {
//		NONE = 0x0,
//		READ = 0x1,
//		WRITE = 0x2,
//		EXEC = 0x4,
//		READ_WRITE_EXEC = READ | WRITE | EXEC
//	};
public:

	/// temporary memory protection change, RAII auto-restores the former protection.
	class ScopedReProtect : public Utility::NonCopyable {
	public:
		ScopedReProtect(ScopedReProtect &&other) noexcept;

		ScopedReProtect &operator=(ScopedReProtect &&other) noexcept;

		/// calls restore() for auto-cleanup
		~ScopedReProtect() {
			restore();
		}

		/// restores the formerly active memory protection
		/// \return true, if the old protection is now active again
		bool restore();

	private:
		friend class MemoryUtils;

		bool m_active;
		MemoryProtection m_formerProtection;

		explicit ScopedReProtect(MemoryProtection oldProtection);
	};

	/// find base address of a loaded shared library.
	/// if a timeout is specified, this blocks either until the library's loaded
	/// or until the timeout is reached.
	/// \param libName filename of wanted library
	/// \param timeout maximum time to wait for library load
	/// \return library base address
	static std::optional<uintptr_t> lib_base_32_timeout(const std::string &libName,
	                                                   std::chrono::milliseconds timeout);

	/// find base address of a loaded shared library.
	/// if the library can't be found, this blocks until it's found.
	/// \param libName filename of wanted library
	/// \param timeout maximum time to wait for library load
	/// \return library base address
	static uintptr_t lib_base_32(const std::string &libName);

#ifdef __linux__
	static std::optional<std::string> loadedLibPath(LibName const &libName);

	/// retrieves the path of this loaded library
	static std::string this_lib_path();

	/// reads the memory protection of given range
	/// \param address start address
	/// \param length length of memory region
	/// \return the protection (as defined in sys/mman.h),
	/// 		or std::nullopt, if given region spans multiple mappings or isn't contained in any
	static std::optional<int> read_protection(uintptr_t address, size_t length);
#endif

	/// alters the protection of a memory area
	/// \param address start address
	/// \param length length of memory area
	/// \param newProtection protection to change into
	/// \return ScopedReProtect handle to the former protection, or std::nullopt if operation failed
	static std::optional<ScopedReProtect> scoped_remove_memory_protection(uintptr_t address, size_t length);

	/// retrieves symbol address exported by loaded dynamic library
	/// \param libName filename of library exporting symbol
	/// \param symbol symbol name
	/// \return address of libName::symbol or std::nullopt if library wasn't
	/// 		loaded or symbol wasn't found in its exported symbols
	static std::optional<uintptr_t> getSymbolAddress(LibName const &libName, std::string const &symbol);

private:

	/// alters the protection of all pages the specified area spans
	/// \param newProtection area and protection to change to
	/// \return the former protection - or std::nullopt if failed
	static std::optional<MemoryProtection> set_memory_protection(MemoryProtection newProtection);
};