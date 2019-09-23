#include <string>
#include <optional>
#include <chrono>   // system_clock
#include <vector>
#include <thread>   // std::this_thread::sleep_for

#include "MemoryUtils.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

#ifdef __linux__

#include <cstring>      // std::strcmp
#include <link.h>       // dl_iterate_phdr
#include <fstream>      // std::ifstream
#include <sys/mman.h>   // mprotect
#include <unistd.h>     // getpagesize
#include <dlfcn.h>      // dlopen, dlsym, dlclose
#include <memory>       // std::unique_ptr
#include <functional>   // std::function

#include "Utility.hpp"  // Utility::split, Utility::get_filename

#else // windows

#include <windows.h>
#include <TlHelp32.h> // MODULEENTRY32, CreateToolhelp32Snapshot

#include "RAIIHandle.hpp"
#endif

using namespace std::chrono_literals; // std::chrono::seconds(1) == 1s

MemoryUtils::ScopedReProtect::ScopedReProtect(MemoryUtils::MemoryProtection oldProtection)
		: m_active(true)
		, m_formerProtection(oldProtection) {}

MemoryUtils::ScopedReProtect::ScopedReProtect(MemoryUtils::ScopedReProtect &&other) noexcept
		: m_active(other.m_active)
		, m_formerProtection(other.m_formerProtection) {
	other.m_active = false;
}

MemoryUtils::ScopedReProtect &MemoryUtils::ScopedReProtect::operator=(MemoryUtils::ScopedReProtect &&other) noexcept {
	m_active = other.m_active;
	other.m_active = false;
	m_formerProtection = other.m_formerProtection;
	return *this;
}

bool MemoryUtils::ScopedReProtect::restore() {
	if (m_active) {
		auto result = set_memory_protection(m_formerProtection);
		if (result.has_value()) {
			m_active = false;
		}
	}

	return !m_active;
}

#ifdef __linux__

// upon construction creates a snapshot of all currently loaded shared objects
struct LibrarySnapshot {
	LibrarySnapshot()
			: phdr_infos() {
		dl_iterate_phdr(&dl_iterate_phdr_callback, this);
	}

	static int dl_iterate_phdr_callback(struct dl_phdr_info *info,
	                                    size_t,
	                                    void *data) {
		auto *phdr_infos = reinterpret_cast<LibrarySnapshot *>(data);
		phdr_infos->phdr_infos.emplace_back(*info);
		return 0;
	}

	std::vector<dl_phdr_info> phdr_infos;
};

std::optional<dl_phdr_info> findFirstLoadedObject(std::function<bool(dl_phdr_info const &)> predicate) {
	LibrarySnapshot snapshot;
	for (auto &library : snapshot.phdr_infos) {
		if (predicate(library)) {
			return library;
		}
	}

	return std::nullopt;
}

std::optional<uintptr_t> MemoryUtils::lib_base_32_timeout(const std::string &libName,
                                                          std::chrono::milliseconds timeout) {
	std::optional<uintptr_t> base_addr;

	auto now = []() { return std::chrono::steady_clock::now(); };
	auto time_end = now() + timeout;

	do {
		auto library = findFirstLoadedObject([&libName](dl_phdr_info const &info) {
			return Utility::get_filename(info.dlpi_name) == libName;
		});
		if (library.has_value()) {
			base_addr = library->dlpi_addr;
			break;
		}

		// TODO this timing comparison surely doesn't work
		std::this_thread::sleep_for(200ms);
	} while (now() < time_end);

	return base_addr;
}

uintptr_t MemoryUtils::lib_base_32(const std::string &libName) {
	auto l_libBase = lib_base_32_timeout(libName, std::chrono::milliseconds::max());
	if (!l_libBase) {
		std::string error{libName + " base addr not found"};
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}
	return *l_libBase;
}

std::optional<std::string> MemoryUtils::loadedLibPath(LibName const &libName) {
	std::optional<std::string> l_libPath;

	auto library = findFirstLoadedObject(
			[name = libName.name()]
					(dl_phdr_info const &info) {
				return Utility::get_filename(info.dlpi_name) == name;
			});
	if (library.has_value()) {
		l_libPath = library->dlpi_name;
	}

	return l_libPath;
}

std::optional<int> MemoryUtils::read_protection(uintptr_t address, size_t length) {
	std::optional<int> protection;

	auto parse_prot_string =
			[](const std::string_view &protection_string) {
				int protection = 0;
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
				protection = parse_prot_string(protection_string);
				break;
			}
		}

	}

	return protection;
}

std::optional<MemoryUtils::MemoryProtection>
MemoryUtils::set_memory_protection(MemoryUtils::MemoryProtection newProtection) {
	auto &address = newProtection.address;
	auto &length = newProtection.length;

	// mprotect accepts page-aligned addresses
	// starting address of containing page can be optained by this important magic:
	// https://stackoverflow.com/questions/6387771/get-starting-address-of-a-memory-page-in-linux
	long pagesize = sysconf(_SC_PAGESIZE);
	uintptr_t addr_page_aligned = address & -pagesize;

	auto raw_page_address = reinterpret_cast<void *>(addr_page_aligned);

	auto formerProtection = read_protection(address, length);
	if (mprotect(raw_page_address, length, newProtection.protection) == 0) {
		if (formerProtection.has_value()) {
			return MemoryProtection{address, length, *formerProtection};
		} else {
			std::string error{"reading memory protection failed (address, length): "};
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

int get_no_protection() {
	return PROT_READ | PROT_WRITE | PROT_EXEC;
}

#else // windows

std::optional<MODULEENTRY32> win_get_moduleentry(const std::string &dll_name) {
	// get snapshot of all modules of current process
	auto l_snapshot_h = RAIIHandle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, 0));

	if (!l_snapshot_h.isValid()) {
		return std::nullopt;
	}

	// read first module from list
	std::optional<MODULEENTRY32> l_me32 = MODULEENTRY32();
	l_me32->dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(l_snapshot_h.getRaw(), &l_me32.value())) {
		// module list empty
		return std::nullopt;
	}

	// iterate through all modules, looking the one with the specified name
	while (dll_name != l_me32->szModule) {
		if (!Module32Next(l_snapshot_h.getRaw(), &l_me32.value())) {
			// last module looked at, module not found
			return std::nullopt;
		}
	}

	return l_me32;
}

std::optional<uintptr_t> MemoryUtils::get_lib_base_32(const std::string &name,
													  std::optional<std::chrono::milliseconds> timeout) {
	std::optional<uintptr_t> base_addr;
	auto now = []() { return std::chrono::steady_clock::now(); };
	auto time_end = now();
	if (timeout.has_value()) {
		time_end += timeout.value();
	}

	bool found = false;
	do {
		std::optional<MODULEENTRY32> l_me32 = win_get_moduleentry(name);
		if(l_me32.has_value()) {
			base_addr = (uintptr_t)l_me32->modBaseAddr;
			found = true;
			break;
		}

		// TODO this timing comparison surely doesn't work
		if (timeout.has_value() && now() < time_end) {
			std::this_thread::sleep_for(500ms);
		}
	} while (timeout.has_value() && !found);

	return base_addr;
}

std::optional<MemoryUtils::MemoryProtection>
MemoryUtils::set_memory_protection(MemoryUtils::MemoryProtection newProtection) {
	uintptr_t old_protection;
	uintptr_t new_protection = newProtection.protection;
	if (VirtualProtect(
			(void *) newProtection.address,
			newProtection.length,
			new_protection,
			(DWORD*)&old_protection) == 0) {
		// unable to change memory protection of code segment
		return std::nullopt;
	} else {
		return MemoryProtection{newProtection.address, newProtection.length, old_protection};
	}
}

uintptr_t get_no_protection() {
	return PAGE_EXECUTE_READWRITE;
}
#endif

std::optional<MemoryUtils::ScopedReProtect>
MemoryUtils::scoped_remove_memory_protection(uintptr_t address, size_t length) {
	MemoryProtection newProtection{address, length, get_no_protection()};
	auto formerProtection = set_memory_protection(newProtection);

	if (formerProtection.has_value()) {
		return ScopedReProtect(*formerProtection);
	} else {
		return std::nullopt;
	}
}

// link with -ldl
std::optional<uintptr_t> MemoryUtils::getSymbolAddress(std::string const &libName, std::string const &symbol) {
	std::optional<uintptr_t> result;

	auto library = findFirstLoadedObject([&libName](dl_phdr_info const &info) {
		return Utility::get_filename(info.dlpi_name) == libName;
	});

	// check if specified library is loaded
	if (library.has_value()) {
		// loaded library found
		const char *libPath = library->dlpi_name;

		// get lib handle (auto-closing)
		auto dlCloser = [](void *handle) { dlclose(handle); };
		std::unique_ptr<void, decltype(dlCloser) &> hLib(dlopen(libPath, RTLD_NOW), dlCloser);

		// check if library was opened
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
			// library couldn't be opened, thus needn't be closed
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