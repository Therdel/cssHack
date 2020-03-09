//
// Created by therdel on 24/01/2020.
//
#pragma once

#include "Utility.hpp"
#include "Pointers/Signatures.hpp"

class LinearSegmentScanner final {
public:
	explicit LinearSegmentScanner(const SignatureAOI &signature)
	: _signature(signature)
	{
	}

    void scanSegment(std::string_view haystack,
                     std::vector<uintptr_t> &matches) const {
        auto &pattern = _signature.signature.pattern();
		const char *lastCandidate = haystack.end() - pattern.size();
		for(const char &position : haystack) {
			if(&position > lastCandidate) {
				break;
			}
			if(signatureMatches(reinterpret_cast<const uint8_t *>(&position))) {
				auto *signatureAoi = &position + _signature.aoi_offset;
				auto signatureAoiAddress = reinterpret_cast<uintptr_t>(signatureAoi);

				matches.push_back(signatureAoiAddress);
			}
		}
	}

private:
	const SignatureAOI &_signature;

	bool signatureMatches(const uint8_t *candidate) const {
	    auto &mask = _signature.signature.mask();
	    auto &pattern = _signature.signature.pattern();
		for(size_t i=0; i<pattern.size(); ++i) {
			if(mask[i] == SignatureMask::CARE) {
				if(candidate[i] != pattern[i]) {
					return false;
				}
			} else {
				// ignore signature wildcards ("??")
				continue;
			}
		}
		return true;
	}
};