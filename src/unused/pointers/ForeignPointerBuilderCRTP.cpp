//
// Created by therdel on 04.09.19.
//
#include "ForeignPointerBuilderCRTP.hpp"

namespace ForeignPtrBuilder {
	ForeignPtrBuilderBase<ForeignRawPtrBase> base(uintptr_t rawAddress) {
		return {ForeignRawPtrBase(rawAddress)};
	}

	ForeignPtrBuilderBase<ForeignLibraryPtrBase> base(LibName const &libName) {
		return {ForeignLibraryPtrBase(libName)};
	}

	template<typename BasePtr>
	ForeignPtrBuilderBase<BasePtr> base(BasePtr &basePtr) {
		return {basePtr};
	}
};