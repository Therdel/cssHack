//
// Created by therdel on 19.10.19.
//
#pragma once

#include "Utility.hpp"
#include "MemoryUtils.hpp"
#include "Pointers/Signatures.hpp"
#include "RegexSegmentScanner.hpp"
#include "LinearSegmentScanner.hpp"

namespace MemoryScanner {
	/// find occurances of a signature in the address space
	/// \param signature signature of wanted memory location
	/// \returns matching locations after applying the signatures offset
	template<typename Scanner=LinearSegmentScanner>
	static std::vector<uintptr_t> scanSignature(const Signature &signature) {
		constexpr auto now = [] { return std::chrono::system_clock::now(); };
		Scanner scanner(signature);
		auto start = now();

		std::vector<uintptr_t> matches;
		auto librarySegments = MemoryUtils::lib_segment_ranges(signature.libName);

		for (auto &segment : librarySegments) {
			if(segment.isReadable() && segment.isExecutable()) {
				scanner.scanSegment(segment, matches);
			}
		}

		auto duration = now() - start;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		Log::log<Log::FLUSH>(Log::Channel::MESSAGE_BOX, "scan duration: ", ms, "ms");

		return matches;
	}
};