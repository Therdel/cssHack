//
// Created by therdel on 24/01/2020.
//
#pragma once

#include "Utility.hpp"
#include "BoyerMooreDontCare/BoyerMooreDontCare.hpp"
#include "Pointers/Signatures.hpp"

class BoyerMooreSegmentScanner final {
public:
	explicit BoyerMooreSegmentScanner(SignatureAOI signatureAOI)
	: _signatureAOI(std::move(signatureAOI))
	, _searcher(_signatureAOI.signature)
	{
	}

    auto scanSegment(std::string_view haystack,
                     std::vector<uintptr_t> &matches) const -> void {
		auto &pattern = _signatureAOI.signature.pattern();
		if(haystack.size() < pattern.size()) {
			return;
		}
		std::basic_string_view<uint8_t> rawSegmentRange((uint8_t*)haystack.data(), haystack.size());
		auto uint8Matches = _searcher.search(rawSegmentRange);

		std::transform(uint8Matches.begin(), uint8Matches.end(), std::back_inserter(matches), [this](const uint8_t *match) {
            auto *signatureAoi = match + _signatureAOI.aoi_offset;
            auto signatureAoiAddress = reinterpret_cast<uintptr_t>(signatureAoi);

            return signatureAoiAddress;
		});
	}

private:
    const SignatureAOI &_signatureAOI;
	BoyerMooreDontCare _searcher;
};