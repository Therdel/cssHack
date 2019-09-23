//
// Created by therdel on 05.09.19.
//
#pragma once

#include "ForeignPointer.hpp"

namespace ForeignPointerBuilder {
	template<typename T=uintptr_t, typename U, ptrdiff_t ...offsets>
	SharedForeignPointer<T> create(SharedForeignPointer<U> basePtr,
	                               OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) {
		static_assert(sizeof...(offsets) > 0, "offset list is empty");

		constexpr size_t N = sizeof...(offsets);
		std::array<ptrdiff_t, N> l_offsets = {offsets...};

		auto offset = l_offsets[0];
		bool onlyOneOffset = N == 1;
		OffsetType type = onlyOneOffset ?
		                  lastOffsetType :
		                  OffsetType::DEREFERENCE;
		SharedForeignPointer<T> result
				= std::make_shared<ForeignPointerOffset<T, U>>(std::move(basePtr), offset, type);

		for (size_t i = 1; i < N; ++i) {

			ptrdiff_t offset = l_offsets[i];
			bool lastOffset = i == N - 1;
			OffsetType type = lastOffset ?
			                  lastOffsetType :
			                  OffsetType::DEREFERENCE;


			result = std::make_shared<ForeignPointerOffset<T, T>>(std::move(result), offset, type);
		}

		return result;
	}

	template<typename T=uintptr_t, ptrdiff_t ...offsets>
	SharedForeignPointer<T> create(SharedForeignPointer<uintptr_t> basePtr,
	                               OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) {
		return create<T, uintptr_t, offsets...>(std::move(basePtr), lastOffsetType);
	}

	template<typename T=uintptr_t, ptrdiff_t ...offsets>
	SharedForeignPointer<T> create(LibName const &libName,
	                               OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) {
		static_assert(sizeof...(offsets) > 0, "offset list is empty");
		auto libraryPointer = std::make_shared<ForeignPointerBaseLibrary>(libName);
		return create<T, uintptr_t, offsets...>(std::move(libraryPointer),
		                                        lastOffsetType);
	}

	template<ptrdiff_t ...offsets>
	SharedForeignPointer<> create(LibName const &libName,
	                              OffsetType lastOffsetType = OffsetType::PLAIN_OFFSET) {
		static_assert(sizeof...(offsets) > 0, "offset list is empty");
		auto libraryPointer = std::make_shared<ForeignPointerBaseLibrary>(libName);
		return create<uintptr_t, uintptr_t, offsets...>(std::move(libraryPointer),
		                                                lastOffsetType);
	}
};