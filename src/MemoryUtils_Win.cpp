#include "MemoryUtils.hpp"

#include <vector>
#include <thread>       // std::this_thread::sleep_for
#include <functional>   // std::function

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <TlHelp32.h> // MODULEENTRY32, CreateToolhelp32Snapshot

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"
#include "RAIIHandle.hpp"

using namespace std::chrono_literals; // std::chrono::seconds(1) == 1s

auto MemoryUtils::MemoryRange::noProtection() -> protection_t {
	return PAGE_EXECUTE_READWRITE;
}

auto MemoryUtils::MemoryRange::isReadable() const -> bool {
	bool readable = protection & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_READONLY | PAGE_READWRITE);
	bool accessible = !(protection & PAGE_NOACCESS);
	return accessible && readable;
}
auto MemoryUtils::MemoryRange::isWritable() const -> bool {
	bool writable = protection & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_READWRITE | PAGE_WRITECOPY);
	bool accessible = !(protection & PAGE_NOACCESS);
	return accessible && writable;
}
auto MemoryUtils::MemoryRange::isExecutable() const -> bool {
	bool executable = protection & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
	bool accessible = !(protection & PAGE_NOACCESS);
	return accessible && executable;
}

static auto find_library(std::function< bool(MODULEENTRY32 const &)> predicate) -> std::optional<MODULEENTRY32> {
	// get snapshot of all modules of current process
	auto l_snapshot_h = RAIIHandle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, 0));

	if (!l_snapshot_h.isValid()) {
		return
		std::nullopt;
	}

	// read first module from list
	std::optional<MODULEENTRY32> l_me32 = MODULEENTRY32();
	l_me32->dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(l_snapshot_h.getRaw(), &l_me32.value())) {
		// module list empty
		return
		std::nullopt;
	}

	// iterate through all modules, looking the one with the specified name
	while (!predicate(l_me32.value())) {
		if (!Module32Next(l_snapshot_h.getRaw(), &l_me32.value())) {
			// last module looked at, module not found
			return
			std::nullopt;
		}
	}

	return
	l_me32;
}

auto MemoryUtils::lib_base_32_timeout(std::string_view libName,
                                      std::chrono::milliseconds timeout) -> std::optional<uintptr_t> {
	std::optional<uintptr_t> base_addr;

	auto now = []() { return std::chrono::steady_clock::now(); };
	auto time_end = now() + timeout;

	do {
		auto predicate = [&](const MODULEENTRY32 &module) { return libName == module.szModule; };
		std::optional<MODULEENTRY32> l_me32 = find_library(std::move(predicate));
		if (l_me32.has_value()) {
			base_addr = reinterpret_cast<uintptr_t>(l_me32->modBaseAddr);
			break;
		}

		std::this_thread::sleep_for(200ms);
	} while (now() < time_end);

	return base_addr;
}

auto MemoryUtils::this_lib_path() -> std::string {
	HMODULE l_hModule;
	DWORD dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
	                | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	BOOL success = GetModuleHandleEx(dwFlags,
	                                 reinterpret_cast<LPCSTR>(&this_lib_path),
	                                 &l_hModule);
	if (success == 0) {
		std::string error{"MemoryUtils::this_lib_path failed"};
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}

	auto predicate = [&](const MODULEENTRY32 &module) { return module.hModule == l_hModule; };
	auto library = find_library(std::move(predicate));
	if (!library.has_value()) {
		std::string error{"MemoryUtils::this_lib_path failed"};
		Log::log<Log::FLUSH>(error);
		throw std::runtime_error(error);
	}

	return library->szExePath;
}

auto MemoryUtils::loadedLibPath(std::string_view libName) -> std::optional<std::string> {
	std::optional<std::string> l_libPath;


    auto predicate = [&libName](const MODULEENTRY32& module) {
        return Util::get_filename(module.szModule) == libName;
	};
	auto library = find_library(std::move(predicate));
	if (library.has_value()) {
		l_libPath = library->szModule;
	}

	return l_libPath;
}

auto MemoryUtils::getSymbolAddress(std::string_view libName, std::string const& symbol) -> std::optional<uintptr_t> {
	std::optional<uintptr_t> result;

    HMODULE l_hModule = GetModuleHandle(libName.data());
	if (l_hModule == nullptr) {
		// no library loaded with that name
		Log::log("getSymbolAddress: No library \"", libName, "\" loaded");
	} else {
		auto symbolAddress = GetProcAddress(l_hModule, symbol.c_str());
		if (symbolAddress == nullptr) {
			// TODO: print GetLastError()
			Log::log("Symbol ", libName, "::", symbol, " could not be retrieved: ");
		} else {
			result = reinterpret_cast<uintptr_t>(symbolAddress);
		}
	}

	return result;
}

auto MemoryUtils::lib_segment_ranges(std::string_view libName,
									 std::function<bool(const MemoryRange&)> predicate) -> std::vector<MemoryRange> {
	std::vector<MemoryRange> ranges;

	auto libraryPredicate = [&libName](const MODULEENTRY32& module) {
		return Util::get_filename(module.szModule) == libName;
	};
	auto library = find_library(std::move(libraryPredicate));
	if (!library.has_value()) {
		auto message = "MemoryUtils::lib_segment_ranges library not found";
		Log::log<Log::FLUSH>(message);
		throw std::runtime_error(message);
	}

	// walk regions inside dll mapping
	auto *modBaseAddr = reinterpret_cast<const char*>(library->modBaseAddr);
	auto modSize = library->modBaseSize;

	MEMORY_BASIC_INFORMATION currentRange;

	for (const char *rangeBaseAddr = modBaseAddr; rangeBaseAddr < (modBaseAddr + modSize);) {
		bool success = VirtualQuery(rangeBaseAddr, &currentRange, sizeof(currentRange)) != 0;
		if (!success) {
			auto message = "MemoryUtils::lib_segment_ranges memory range couldn't be retrieved";
			Log::log<Log::FLUSH>(message);
			throw std::runtime_error(message);
		}

		auto protection = currentRange.Protect;
		auto regionSize = currentRange.RegionSize;
		auto range = MemoryRange{ {rangeBaseAddr, regionSize}, protection};

		if (predicate(range)) {
			ranges.push_back(range);
		}

		rangeBaseAddr += regionSize;
	}

	return ranges;
}

auto MemoryUtils::set_memory_protection(MemoryProtection newProtection) -> std::optional<MemoryProtection> {
	DWORD old_protection;
	DWORD new_protection = newProtection.protection;
	const void* rawConstAddress = reinterpret_cast<const void*>(newProtection.memoryRange.data());
	void* rawAddress = const_cast<void*>(rawConstAddress);
	if (VirtualProtect(
			rawAddress,
			newProtection.memoryRange.size(),
			new_protection,
			&old_protection) == 0) {
		// unable to change memory protection of code segment
		return std::nullopt;
	} else {
		return MemoryProtection{ newProtection.memoryRange, old_protection };
	}
}
