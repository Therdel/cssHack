#pragma once

#include <optional>
#include <chrono>
#include <string>
#include <functional>

#include "Utility.hpp"

class MemoryUtils {
public:
	/// describes a memory area with its protection
	struct MemoryRange {
		using protection_t = uintmax_t;

		std::string_view memoryRange;
		protection_t protection;

		auto address() const -> uintptr_t;

		auto isReadable() const -> bool;
		auto isWritable() const -> bool;
		auto isExecutable() const -> bool;
		static auto noProtection() -> protection_t;
	};
	using MemoryProtection = MemoryRange;

	/// temporary memory protection change, RAII auto-restores the former protection.
	class ScopedReProtect : private Util::NonCopyable {
	public:
		ScopedReProtect(ScopedReProtect &&other) noexcept;
		auto operator=(ScopedReProtect &&other) noexcept -> ScopedReProtect&;

		/// calls restore() for auto-cleanup
		~ScopedReProtect();

		/// restores the formerly active memory protection
		/// \return true, if the old protection is now active again
		auto restore() -> bool;

	private:
		friend class MemoryUtils;

		bool _active;
		MemoryProtection _oldProtection;

		explicit ScopedReProtect(MemoryProtection oldProtection);
	};

	/// find base address of a loaded shared library.
	/// Blocks until either the library's loaded
	/// or until the timeout is reached.
	/// \param libName filename of wanted library
	/// \param timeout maximum time to wait for library load
	/// \return library base address
	static auto lib_base_32_timeout(std::string_view libName,
									std::chrono::milliseconds timeout) -> std::optional<uintptr_t>;

	/// find base address of a loaded shared library.
	/// if the library can't be found, this blocks until it's found.
	/// \param libName filename of wanted library
	/// \return library base address
	static auto lib_base_32(std::string_view libName) -> uintptr_t;

	/// find absolute filepath of this loaded shared library
	static auto this_lib_path() -> std::string;

	/// find absolute filepath of a loaded shared library
	/// \param libName filename of wanted library
	/// \return absolute filepath to library
	static auto loadedLibPath(std::string_view libName) -> std::optional<std::string>;

	/// alters the protection of a memory area
	/// \param address start address
	/// \param length length of memory area
	/// \return ScopedReProtect handle to the former protection, or std::nullopt if operation failed
	static auto scoped_remove_memory_protection(uintptr_t address, size_t length) -> std::optional<ScopedReProtect>;

	/// retrieves symbol address exported by loaded dynamic library
	/// \param libName filename of library exporting symbol
	/// \param symbol symbol name
	/// \return address of libName::symbol or std::nullopt if library wasn't
	/// 		loaded or symbol wasn't found in its exported symbols
	static auto getSymbolAddress(std::string_view libName, const std::string &symbol) -> std::optional<uintptr_t>;

	/// reads allocated memory ranges and their respective protections
	/// of a loaded dynamic library
	/// \param libName library of interest's filename
	/// \param predicate user-defined filter, for protection checks for example.
	/// \return allocated memory ranges and their respective protections of specified library
	static auto lib_segment_ranges(std::string_view libName,
								   std::function<bool(const MemoryRange&)> predicate = [](auto&) { return true; }) -> std::vector<MemoryRange>;

private:

	/// alters the protection of all pages the specified area spans
	/// \param newProtection area and protection to change to
	/// \return the former protection - or std::nullopt if failed
	static auto set_memory_protection(MemoryProtection newProtection) -> std::optional<MemoryProtection>;
};
