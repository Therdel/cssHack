//
// Created by therdel on 24/01/2020.
//
#pragma once

#include "Utility.hpp"
#include "MemoryUtils.hpp"
#include "Pointers/Signatures.hpp"
#include "RegexSegmentScanner.hpp"

class LinearSegmentScanner final {
public:
	explicit LinearSegmentScanner(const Signature &signature)
	: _signature(signature)
	, _rawSignature(parseSignature(_signature))
	{
	}

	void scanSegment(const MemoryUtils::LibrarySegmentRange &segment,
	                        std::vector<uintptr_t> &matches) const {
		auto &memoryRange = segment.memoryRange;
		if(memoryRange.size() < _signature.sig.size()) {
			return;
		}
		const char *lastCandidate = memoryRange.end() - _signature.sig.size();
		for(const char &position : memoryRange) {
			if(&position > lastCandidate) {
				break;
			}
			if(signatureMatches(&position)) {
				auto *signatureAoi = &position + _signature.aoi_offset;
				auto signatureAoiAddress = reinterpret_cast<uintptr_t>(signatureAoi);

				matches.push_back(signatureAoiAddress);
			}
		}
	}

private:
	using RawSignature = std::vector<std::optional<char>>;

	const Signature &_signature;
	RawSignature _rawSignature;

	static RawSignature parseSignature(const Signature &signature) {
		RawSignature rawSignature;
		auto bytes = Utility::split(signature.sig, " ");
		for (auto &byteDesc : bytes) {
			if (byteDesc.size() != 2) {
				throw std::invalid_argument("Wrong signature byte description size");
			}
			if (byteDesc == "??") {
				rawSignature.emplace_back(std::nullopt);
			} else {
				rawSignature.emplace_back(std::stoi(std::string(byteDesc), nullptr, 16));
			}
		}
		return rawSignature;
	}

	bool signatureMatches(const char *candidate) const {
		for(size_t i=0; i<_rawSignature.size(); ++i) {
			auto &signaturePos = _rawSignature[i];
			if(signaturePos.has_value()) {
				if(candidate[i] != *signaturePos) {
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