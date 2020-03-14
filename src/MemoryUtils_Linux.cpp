#include "MemoryUtils.hpp"

#include <vector>
#include <thread>   // std::this_thread::sleep_for
#include <functional>   // std::function

#include <link.h>       // dl_iterate_phdr, dl_phdr_info
#include <fstream>      // std::ifstream
#include <sys/mman.h>   // mprotect
#include <unistd.h>     // getpagesize
#include <dlfcn.h>      // dlopen, dlsym, dlclose, dladdr, Dl_info
#include <memory>       // std::unique_ptr
#include <iostream>

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"
#include "Utility.hpp"  // Utility::split, Utility::get_filename

using namespace std::chrono_literals; // std::chrono::seconds(1) == 1s

auto MemoryUtils::MemoryRange::noProtection() -> protection_t {
	return PROT_READ | PROT_WRITE | PROT_EXEC;
}

auto MemoryUtils::MemoryRange::isReadable() const -> bool { return protection & PF_R; }
auto MemoryUtils::MemoryRange::isWritable() const -> bool { return protection & PF_W; }
auto MemoryUtils::MemoryRange::isExecutable() const -> bool { return protection & PF_X; }

// upon construction creates a snapshot of all currently loaded shared objects
struct LibrarySnapshot {
	LibrarySnapshot()
			: phdr_infos() {
		dl_iterate_phdr(&dl_iterate_phdr_callback, this);
	}

	static auto dl_iterate_phdr_callback(struct dl_phdr_info *info,
										 size_t,
										 void *data) -> int {
		auto *phdr_infos = reinterpret_cast<LibrarySnapshot*>(data);
		phdr_infos->phdr_infos.emplace_back(*info);
		return 0;
	}

	std::vector<dl_phdr_info> phdr_infos;
};

auto find_library(std::function<bool(dl_phdr_info const &)> predicate) -> std::optional<dl_phdr_info> {
	std::optional<dl_phdr_info> result;

	LibrarySnapshot snapshot;
	for (auto &library : snapshot.phdr_infos) {
		if (predicate(library)) {
			result = library;
			break;
		}
	}

	return result;
}

auto MemoryUtils::lib_base_32_timeout(std::string_view libName,
									  std::chrono::milliseconds timeout) -> std::optional<uintptr_t> {
	std::optional<uintptr_t> base_addr;

	auto now = []() { return std::chrono::steady_clock::now(); };
	auto time_end = now() + timeout;

	do {
		auto predicate = [&](dl_phdr_info const &info) {
			return Utility::get_filename(info.dlpi_name) == libName;
		};
		auto library = find_library(std::move(predicate));
		if (library.has_value()) {
			base_addr = library->dlpi_addr;
			break;
		}

		std::this_thread::sleep_for(200ms);
	} while (now() < time_end);

	return base_addr;
}

auto MemoryUtils::this_lib_path() -> std::string {
	// get library path without its name
	// https://www.unknowncheats.me/forum/counterstrike-global-offensive/248039-linux-unloading-cheat.html
	Dl_info info;
	dladdr((void *) &this_lib_path, &info);
	return {info.dli_fname};
}

auto MemoryUtils::loadedLibPath(std::string_view libName) -> std::optional<std::string> {
	std::optional<std::string> l_libPath;

	auto predicate = [&](dl_phdr_info const &info) {
		return Utility::get_filename(info.dlpi_name) == libName;
	};
	auto library = find_library(std::move(predicate));
	if (library.has_value()) {
		l_libPath = library->dlpi_name;
	}

	return l_libPath;
}

// link with -ldl
auto MemoryUtils::getSymbolAddress(std::string_view libName, const std::string &symbol) -> std::optional<uintptr_t> {
	std::optional<uintptr_t> result;

	auto libPath = loadedLibPath(libName);

	// check if specified library is loaded
	if (libPath.has_value()) {
		// get lib handle (auto-closing)
		auto dlCloser = [](void *handle) { dlclose(handle); };
		std::unique_ptr<void, decltype(dlCloser) &> hLib(dlopen(libPath->c_str(), RTLD_NOW), dlCloser);

		// check if library could be opened
		if (hLib != nullptr) {
			// see man dlsym(3) on purpose of dlerror use
			dlerror(); // clear old error
			void *address = dlsym(hLib.get(), symbol.c_str());
			// check if symbol found
			if (dlerror() == nullptr) {
				// symbol found, yay!
				result = reinterpret_cast<uintptr_t>(address);
			}
		} else {
			// library couldn't be opened, so the handle needn't be closed
			hLib.release();
		}
		if (!result.has_value()) {
			Log::log("Symbol ", libName, "::", symbol, " could not be retrieved: ", dlerror());
		}
	} else {
		// no library loaded with that name
		Log::log("getSymbolAddress: No library \"", libName, "\" loaded");
	}

	return result;
}

auto MemoryUtils::lib_segment_ranges(std::string_view libName,
									 std::function<bool(const MemoryRange&)> predicate) -> std::vector<MemoryRange> {
	std::vector<MemoryRange> ranges;

	// find library
	dl_phdr_info library{};
	while (true) {
		auto predicate = [&](dl_phdr_info const &info) {
			return Utility::get_filename(info.dlpi_name) == libName;
		};
		auto optionalLib = find_library(std::move(predicate));
		if (optionalLib.has_value()) {
			library = *optionalLib;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	// read library segment ranges
	for (int i = 0; i < library.dlpi_phnum; ++i) {
		const auto &programHeader = library.dlpi_phdr[i];
		const auto protection = programHeader.p_flags;

		const auto libBase = library.dlpi_addr;
		const char *segmentBase = reinterpret_cast<const char *>(libBase + programHeader.p_vaddr);
		size_t segmentSize = programHeader.p_memsz;
		std::string_view segmentRange(segmentBase, segmentSize);
		MemoryRange range{segmentRange, protection};

		if (predicate(range)) {
			ranges.push_back(range);
		}
	}

	return ranges;
}

/// reads the memory protection of given range
/// \param address start address
/// \param length length of memory region
/// \return the protection (as defined in sys/mman.h),
/// 		or std::nullopt, if given region spans multiple mappings or isn't contained in any
auto read_protection(uintptr_t address, size_t length) -> std::optional<MemoryUtils::MemoryProtection> {
    using MemoryProtection = MemoryUtils::MemoryProtection;
	std::optional<MemoryProtection> protection;

	auto parse_prot_string =
			[](const std::string_view &protection_string) {
				MemoryProtection::protection_t protection = 0;
				if (protection_string[0] == 'r')
					protection |= PROT_READ;
				if (protection_string[1] == 'w')
					protection |= PROT_WRITE;
				if (protection_string[2] == 'x')
					protection |= PROT_EXEC;

				return protection;
			};

	auto map_range_contains =
			[address, length](const std::string_view &map_range) {
				// parse map range
				auto map_range_tokens = Utility::split(map_range, "-");
				auto address_start = std::stoull(std::string(map_range_tokens[0]), 0, 16);
				auto address_end = std::stoull(std::string(map_range_tokens[1]), 0, 16);

				return address >= address_start &&
				       address + length <= address_end;
			};

	std::ifstream mem_maps("/proc/self/maps");
	if (mem_maps.good()) {
		std::string line;
		// iterate over memory maps
		while (std::getline(mem_maps, line)) {
			auto mapping = Utility::split(line, " ");
			std::string_view &map_range = mapping[0];
			std::string_view &protection_string = mapping[1];

			if (map_range_contains(map_range)) {
			    const char *charAddress = reinterpret_cast<const char*>(address);
				protection = MemoryProtection{ { charAddress, length }, parse_prot_string(protection_string)};
				break;
			}
		}
	} else {
		auto error = "MemoryUtils::read_protection: /proc/self/maps doesn't exist";
		Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, error);
		throw std::runtime_error(error);
	}

	return protection;
}

auto MemoryUtils::set_memory_protection(MemoryProtection newProtection) -> std::optional<MemoryProtection> {
	auto address = newProtection.address();
	auto length = newProtection.memoryRange.size();

	// mprotect accepts page-aligned addresses
	// starting address of containing page can be optained by this important magic:
	// https://stackoverflow.com/questions/6387771/get-starting-address-of-a-memory-page-in-linux
	long pagesize = sysconf(_SC_PAGESIZE);
	uintptr_t addr_page_aligned = address & -pagesize;

	auto raw_page_address = reinterpret_cast<void *>(addr_page_aligned);

	auto formerProtection = read_protection(address, length);
	if (mprotect(raw_page_address, length, newProtection.protection) == 0) {
		if (formerProtection.has_value()) {
			return formerProtection;
		} else {
			std::string error{"  reading memory protection failed (address, length): "};
			error += address;
			error += ", ";
			error += length;
			Log::log<Log::FLUSH>(error);
			throw std::runtime_error(error);
		}
	} else {
		return std::nullopt;
	}
}
