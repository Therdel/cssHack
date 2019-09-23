//
// Created by therdel on 04.09.19.
//
#include "ForeignPointerCRTP.hpp"

#include "../../MemoryUtils.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../../Log.hpp"

uintptr_t ForeignLibraryPtrBase::readLibraryBase() const {
	const auto libBase = MemoryUtils::get_lib_base_32(m_libName);
	if (!libBase.has_value()) {
		std::string error{m_libName.name() + " base addr not found"};
		Log::log(error);
		throw std::runtime_error(error);
	}

	return *libBase;
}