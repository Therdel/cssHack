//
// Created by therdel on 19.10.19.
//
#pragma once
#include <regex>

#include "Utility.hpp"
#include "MemoryUtils.hpp"
#include "Pointers/Signatures.hpp"

class RegexSegmentScanner final {
public:
	explicit RegexSegmentScanner(const Signature &signature)
	: _signature(signature)
	, _regex(BinaryRegex::from(signature))
	{
	}

	void scanSegment(const MemoryUtils::LibrarySegmentRange &segment,
	                        std::vector<uintptr_t> &matches) const {
		auto &memoryRange = segment.memoryRange;
		std::cmatch results;
		std::regex_search(memoryRange.begin(), memoryRange.end(), results, _regex);

		for (size_t i = 0; i < results.size(); ++i) {
			const auto offset = results.position(i);
			auto *matchInSegment = memoryRange.data() + offset;
			auto *signatureAoiInSegment = matchInSegment + _signature.aoi_offset;
			auto signatureAoiInSegmentRaw = reinterpret_cast<uintptr_t>(signatureAoiInSegment);

			matches.push_back(signatureAoiInSegmentRaw);
		}
	}

private:
	const Signature &_signature;
	const std::regex _regex;

	struct BinaryRegex {
	public:
		static std::regex from(const Signature &signature) {
			std::stringstream regex;
			auto bytes = Utility::split(signature.sig, " ");
			for (auto &byteDesc : bytes) {
				if (byteDesc.size() != 2) {
					throw std::invalid_argument("Wrong signature byte description size");
				}
				if (byteDesc == "??") {
					appendUnknownByte(regex);
				} else {
					appendKnownByte(regex, byteDesc);
				}
			}
			return std::regex(regex.str(), std::regex_constants::ECMAScript | std::regex_constants::optimize);
		}
	private:
		static void appendUnknownByte(std::stringstream &regex) {
			// "[\\S\\s]" matches any character/byte
			regex << "[\\S\\s]";
		}

		static void appendKnownByte(std::stringstream &regex, std::string_view representation) {
			// TODO: ensure passed representation is a valid hex number (throws instead)

			// "\xHH" matches a raw hex value HH
			regex << "\\x" << representation;
		}
	};
};