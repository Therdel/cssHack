//
// Created by therdel on 04.09.19.
//
#pragma once
#include "ForeignPointerCRTP.hpp"


template<typename BasePtr, typename U=uintptr_t>
class ForeignPtrBuilderOffset {
public:
	ForeignPtrBuilderOffset(BasePtr basePtr, ptrdiff_t offset, OffsetType type)
			: m_basePtr(basePtr)
			, m_offset(offset)
			, m_type(type){}

	ForeignPtrOffset<BasePtr, U> build() const {
		return {m_basePtr, m_offset, m_type};
	}

	template<typename T=uintptr_t>
	ForeignPtrBuilderOffset<ForeignPtrOffset<BasePtr, U>, T> add_offset(ptrdiff_t offset, OffsetType type) {
		return {m_basePtr, offset, type};
	}

private:

	BasePtr m_basePtr;
	ptrdiff_t m_offset;
	OffsetType m_type;
};

template<typename BasePtr>
class ForeignPtrBuilderBase {
public:
	ForeignPtrBuilderBase(BasePtr basePtr)
			: m_basePtr(std::move(basePtr)) {}

	BasePtr build() const {
		return m_basePtr;
	}

	template<typename T=uintptr_t>
	ForeignPtrBuilderOffset<BasePtr, T> add_offset(ptrdiff_t offset, OffsetType type) const {
		return {m_basePtr, offset, type};
	}

private:
	BasePtr m_basePtr;
};

namespace ForeignPtrBuilder {
	ForeignPtrBuilderBase<ForeignRawPtrBase> base(uintptr_t rawAddress);

	ForeignPtrBuilderBase<ForeignLibraryPtrBase> base(LibName const &libName);

	template<typename BasePtr>
	ForeignPtrBuilderBase<BasePtr> base(BasePtr &basePtr);
};