//
// Created by therdel on 19.10.19.
//
#pragma once
#include <regex>
#include <iomanip>

#include "Utility.hpp"
#include "Pointers/Signatures.hpp"

class RegexSegmentScanner final {
public:
	explicit RegexSegmentScanner(const SignatureAOI &signature)
	: _signature(signature)
	, _regex(BinaryRegex::from(signature))
	{
	}

	void scanSegment(std::string_view haystack,
                     std::vector<uintptr_t> &matches) const {
		std::cmatch results;
		std::regex_search(haystack.begin(), haystack.end(), results, _regex);

		for (size_t i = 0; i < results.size(); ++i) {
			const auto offset = results.position(i);
			auto *matchInSegment = haystack.data() + offset;
			auto *signatureAoiInSegment = matchInSegment + _signature.aoi_offset;
			auto signatureAoiInSegmentRaw = reinterpret_cast<uintptr_t>(signatureAoiInSegment);

			matches.push_back(signatureAoiInSegmentRaw);
		}
	}

private:
	const SignatureAOI &_signature;
	const std::regex _regex;

	struct BinaryRegex {
	public:
		static std::regex from(const SignatureAOI &signature) {
			std::stringstream regex;
			auto &pattern = signature.signature.pattern();
            auto &mask = signature.signature.mask();
            for (size_t i = 0; i < pattern.size(); ++i) {
                if (mask[i] == SignatureMask::DONT_CARE) {
                    appendUnknownByte(regex);
                } else {
                    appendKnownByte(regex, pattern[i]);
                }
            }
            return std::regex(regex.str(), std::regex_constants::ECMAScript | std::regex_constants::optimize);
        }

    private:
        static void appendUnknownByte(std::stringstream &regex) {
            // "[\\S\\s]" matches any character/byte
            regex << "[\\S\\s]";
        }

        static void appendKnownByte(std::stringstream &regex, uint8_t byte) {
            // "\xHH" matches a raw hex value HH
            regex << "\\x";

            if(byte < 16) {
                regex << '0';
            }

            regex << std::hex << std::uppercase;
            regex << static_cast<unsigned>(byte);
            regex << std::dec << std::nouppercase;
		}
	};
};