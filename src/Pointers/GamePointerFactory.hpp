//
// Created by therdel on 05.09.19.
//
#pragma once

#include <map>

#include "SharedGamePointer.hpp"
#include "GamePointerDef.hpp"

namespace GamePointerFactory {
	template<typename T=uintptr_t>
	auto create(std::shared_ptr<GamePointerUntyped> basePtr,
	            std::vector<ptrdiff_t> const &offsets,
	            OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) -> std::shared_ptr<GamePointer<T>> {
		std::shared_ptr<GamePointer<T>> result;

		if(offsets.empty()) {
			result = GamePointerOffset<T>::create(std::move(basePtr), 0, lastOffsetType);
		} else {
			for (size_t i = 0; i < offsets.size(); ++i) {
				bool firstOffset = i == 0;
				bool lastOffset = i == offsets.size() - 1;
				ptrdiff_t offset = offsets[i];

				OffsetType type = lastOffset ?
				                  lastOffsetType :
				                  OffsetType::DEREFERENCE;
				if (firstOffset) {
					result = GamePointerOffset<T>::create(std::move(basePtr), offset, type);
				} else {
					result = GamePointerOffset<T>::create(std::move(result), offset, type);
				}
			}
		}

		return result;
	}

	template<typename T=uintptr_t>
	auto create(std::string_view libName,
				std::vector<ptrdiff_t> const &offsets,
	            OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) -> std::shared_ptr<GamePointer<T>> {
		auto libraryPointer = std::make_shared<GamePointerBaseLibrary>(libName);
		return create<T>(std::move(libraryPointer),
		                            offsets,
		                            lastOffsetType);
	}

	template<typename T=uintptr_t>
	auto create(uintptr_t address,
				std::vector<ptrdiff_t> const& offsets = {},
				OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) -> std::shared_ptr<GamePointer<T>> {
		auto rawGamePointer = std::make_shared<GamePointerBaseRaw>(address);
		return create<T>(std::move(rawGamePointer),
		                            offsets,
		                            lastOffsetType);
	}
	
	template<typename T>
	auto get(GamePointerDef::RawPointer<T> const& rawAddress) -> SharedGamePointer<T> {
		using RawPointerStorage = std::map<GamePointerDef::RawPointer<T> const*, std::shared_ptr<GamePointer<T>>>;

		std::shared_ptr<GamePointer<T>> result;

		static RawPointerStorage s_rawPointerStorage;
		auto* key = &rawAddress;
		// check if pointer exists
		auto iterator = s_rawPointerStorage.find(key);
		if (iterator != s_rawPointerStorage.end()) {
			// return existing pointer
			result = iterator->second;
		}
		else {
			// create pointer and return
			result = create<T>(rawAddress._address);
			s_rawPointerStorage[key] = result;
		}

		return result;
	}

	template<typename T>
	auto get(GamePointerDef::Base<T> const &base) -> SharedGamePointer<T> {
		using BasePointerStorage = std::map<GamePointerDef::Base<T> const *, std::shared_ptr<GamePointer<T>>>;

		std::shared_ptr<GamePointer<T>> result;

		static BasePointerStorage s_basePointerStorage;
		// check if pointer exists
		auto base_it = s_basePointerStorage.find(&base);
		if (base_it != s_basePointerStorage.end()) {
			// return existing pointer
			result = base_it->second;
		} else {
			// create base pointer and return
			result = create<T>(base.libName, base.offsets, base.lastOffsetType);
			s_basePointerStorage[&base] = result;
		}

		return result;
	}

	template<typename T, typename BaseDef>
	auto get(GamePointerDef::Composite<T, BaseDef> const &composite) -> SharedGamePointer<T> {
		using CompositePointerStorage = std::map<GamePointerDef::Composite<T, BaseDef> const *, std::shared_ptr<GamePointer<T>>>;

		std::shared_ptr<GamePointer<T>> result;

		static CompositePointerStorage s_compositePointerStorage;
		// check if pointer exists
		auto composite_it = s_compositePointerStorage.find(&composite);
		if(composite_it != s_compositePointerStorage.end()) {
			// return existing pointer
			result = composite_it->second;
		} else {
			// get base pointer
			auto basePointer = get(composite.base).getGamePointer();

			// create pointer and return
			result = create<T>(basePointer, composite.offsets, composite.lastOffsetType);
			s_compositePointerStorage[&composite] = result;
		}

		return result;
	}
}