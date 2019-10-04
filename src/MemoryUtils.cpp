#include "MemoryUtils.hpp"

#include <string>
#include <optional>
#include <chrono>   // system_clock
#include <vector>
#include <thread>   // std::this_thread::sleep_for
#include <functional>   // std::function

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

#ifdef __linux__
#include <link.h>       // dl_iterate_phdr, dl_phdr_info
#include <fstream>      // std::ifstream
#include <sys/mman.h>   // mprotect
#include <unistd.h>     // getpagesize
#include <dlfcn.h>      // dlopen, dlsym, dlclose, dladdr, Dl_info
#include <memory>       // std::unique_ptr

#include "Utility.hpp"  // Utility::split, Utility::get_filename

#else // windows
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
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

static int get_no_protection() {
	return PROT_READ | PROT_WRITE | PROT_EXEC;
}

#else
static uintptr_t get_no_protection() {
  return PAGE_EXECUTE_READWRITE;
}
#endif

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

static std::optional<dl_phdr_info> find_library(std::function<bool(dl_phdr_info const &)> predicate) {
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

#else
static std::optional<MODULEENTRY32> find_library(std::function<bool(MODULEENTRY32 const&)> predicate) {
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
  while (!predicate(l_me32.value())) {
	if (!Module32Next(l_snapshot_h.getRaw(), &l_me32.value())) {
	  // last module looked at, module not found
	  return std::nullopt;
	}
  }

  return l_me32;
}
#endif

std::optional<uintptr_t> MemoryUtils::lib_base_32_timeout(std::string_view libName,
                                                          std::chrono::milliseconds timeout) {
	std::optional<uintptr_t> base_addr;

	auto now = []() { return std::chrono::steady_clock::now(); };
	auto time_end = now() + timeout;

	do {
#ifdef __linux__
		auto predicate = [&](dl_phdr_info const &info) {
			return Utility::get_filename(info.dlpi_name) == libName;
		};
		auto library = find_library(std::move(predicate));
		if (library.has_value()) {
			base_addr = library->dlpi_addr;
			break;
		}
#else
		auto predicate = [&](const MODULEENTRY32& module) { return libName == module.szModule; };
		std::optional<MODULEENTRY32> l_me32 = find_library(std::move(predicate));
		if (l_me32.has_value()) {
		  base_addr = reinterpret_cast<uintptr_t>(l_me32->modBaseAddr);
		  break;
		}
#endif

		std::this_thread::sleep_for(200ms);
	} while (now() < time_end);

	return base_addr;
}

uintptr_t MemoryUtils::lib_base_32(std::string_view libName) {
	if(&libName == nullptr)
		Log::log<Log::FLUSH>("FUCKUP");
	auto l_libBase = lib_base_32_timeout(libName, (std::chrono::milliseconds::max) ());
	if (!l_libBase) {
		std::string error{libName};
		error += " base addr not found";
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}
	return *l_libBase;
}

#ifdef __linux__

std::string MemoryUtils::this_lib_path() {
	// get library path without its name
	// https://www.unknowncheats.me/forum/counterstrike-global-offensive/248039-linux-unloading-cheat.html
	Dl_info info;
	dladdr((void *) &this_lib_path, &info);
	return {info.dli_fname};
}

#else
std::string MemoryUtils::this_lib_path() {
  HMODULE l_hModule;
  DWORD dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
	| GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
  BOOL success = GetModuleHandleEx(dwFlags,
	reinterpret_cast<LPCSTR>(&this_lib_path),
	&l_hModule);
  if (success == 0) {
	std::string error{ "MemoryUtils::this_lib_path failed" };
	Log::log<Log::FLUSH>(error);
	throw std::runtime_error(error);
  }

  auto predicate = [&](const MODULEENTRY32& module) { return module.hModule == l_hModule; };
  auto library = find_library(std::move(predicate));
  if (!library.has_value()) {
	std::string error{ "MemoryUtils::this_lib_path failed" };
	Log::log<Log::FLUSH>(error);
	throw std::runtime_error(error);
  }

  return library->szExePath;
}
#endif

#ifdef __linux__

std::optional<std::string> MemoryUtils::loadedLibPath(std::string_view libName) {
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

#else
std::optional<std::string> MemoryUtils::loadedLibPath(LibName const& libName) {
  std::optional<std::string> l_libPath;

  auto predicate = [name = libName.name()](const MODULEENTRY32& module) {
	return Utility::get_filename(module.szModule) == name;
  };
  auto library = find_library(std::move(predicate));
  if (library.has_value()) {
	l_libPath = library->szModule;
  }

  return l_libPath;
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

#ifdef __linux__

// link with -ldl
std::optional<uintptr_t> MemoryUtils::getSymbolAddress(std::string_view libName, const std::string &symbol) {
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

#else
std::optional<uintptr_t> MemoryUtils::getSymbolAddress(LibName const& libName, std::string const& symbol) {
  std::optional<uintptr_t> result;

  HMODULE l_hModule = GetModuleHandle(libName);
  if (l_hModule == nullptr) {
	// no library loaded with that name
	Log::log("getSymbolAddress: No library \"", libName, "\" loaded");
  }
  else {
	auto symbolAddress = GetProcAddress(l_hModule, symbol.c_str());
	if (symbolAddress == nullptr) {
	  // TODO: print GetLastError()
	  Log::log("Symbol ", libName, "::", symbol, " could not be retrieved: ");
	}
	else {
	  result = reinterpret_cast<uintptr_t>(symbolAddress);
	}
  }

  return result;
}
#endif

#ifdef __linux__

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
	} else {
		auto error = "MemoryUtils::read_protection: /proc/self/maps doesn't exist";
		Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, error);
		throw std::runtime_error(error);
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

#else
std::optional<MemoryUtils::MemoryProtection>
MemoryUtils::set_memory_protection(MemoryUtils::MemoryProtection newProtection) {
  uintptr_t old_protection;
  uintptr_t new_protection = newProtection.protection;
  if (VirtualProtect(
	reinterpret_cast<void*>(newProtection.address),
	newProtection.length,
	new_protection,
	reinterpret_cast<DWORD*>(&old_protection)) == 0) {
	// unable to change memory protection of code segment
	return std::nullopt;
  }
  else {
	return MemoryProtection{ newProtection.address, newProtection.length, old_protection };
  }
}
#endif
