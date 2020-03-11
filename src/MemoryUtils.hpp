#pragma once

#include <optional>
#include <chrono>
#include <string>
#include <functional>

#include "Utility.hpp"

class MemoryUtils {
public:
/// describes a memory area protection
	struct MemoryProtection {
		using protection_t = uintmax_t;

		uintptr_t address;
		size_t length;
		protection_t protection;

		static protection_t noProtection();
	};

	struct LibrarySegmentRange {
		uintmax_t protection;
		std::string_view memoryRange;
		bool isReadable() const;
		bool isWritable() const;
		bool isExecutable() const;
	};

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
	/// Blocks until either the library's loaded
	/// or until the timeout is reached.
	/// \param libName filename of wanted library
	/// \param timeout maximum time to wait for library load
	/// \return library base address
	static std::optional<uintptr_t> lib_base_32_timeout(std::string_view libName,
	                                                    std::chrono::milliseconds timeout);

	/// find base address of a loaded shared library.
	/// if the library can't be found, this blocks until it's found.
	/// \param libName filename of wanted library
	/// \return library base address
	static uintptr_t lib_base_32(std::string_view libName);

	/// find absolute filepath of this loaded shared library
	static std::string this_lib_path();

	/// find absolute filepath of a loaded shared library
	/// \param libName filename of wanted library
	/// \return absolute filepath to library
	static std::optional<std::string> loadedLibPath(std::string_view libName);

	/// alters the protection of a memory area
	/// \param address start address
	/// \param length length of memory area
	/// \return ScopedReProtect handle to the former protection, or std::nullopt if operation failed
	static std::optional<ScopedReProtect> scoped_remove_memory_protection(uintptr_t address, size_t length);

	/// retrieves symbol address exported by loaded dynamic library
	/// \param libName filename of library exporting symbol
	/// \param symbol symbol name
	/// \return address of libName::symbol or std::nullopt if library wasn't
	/// 		loaded or symbol wasn't found in its exported symbols
	static std::optional<uintptr_t> getSymbolAddress(std::string_view libName, const std::string &symbol);

	/// reads allocated memory ranges and their respective protections
	/// of a loaded dynamic library
	/// \param libName library of interest's filename
	/// \param predicate user-defined filter, for protection checks for example.
	/// \return allocated memory ranges and their respective protections of specified library
	static std::vector<LibrarySegmentRange> lib_segment_ranges(std::string_view libName,
                                                               std::function<bool(const LibrarySegmentRange&)> predicate = [](auto&) { return true; });

private:

	/// alters the protection of all pages the specified area spans
	/// \param newProtection area and protection to change to
	/// \return the former protection - or std::nullopt if failed
	static std::optional<MemoryProtection> set_memory_protection(MemoryProtection newProtection);
};
