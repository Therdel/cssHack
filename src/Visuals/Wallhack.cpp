//
// Created by therdel on 29.06.19.
//
#include <iostream>
#include <GL/gl.h>

#include "Wallhack.hpp"
#include "../MemoryUtils.hpp"
#include <dlfcn.h>      // dlopen, dlsym
#include "../Pointers/libNames.hpp"
#include "../Pointers/Offsets.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../Log.hpp"

Wallhack *g_Wallhack = nullptr;

Wallhack::DrawIndexedPrimitive_t Wallhack::getDrawIndexedPrimitive() {
	const char *symbolName = "_ZN16IDirect3DDevice920DrawIndexedPrimitiveE17_D3DPRIMITIVETYPEijjjj";

	auto drawIndexedPrimitiveRaw = MemoryUtils::getSymbolAddress(libNames::libtogl, symbolName);
	return reinterpret_cast<DrawIndexedPrimitive_t>(*drawIndexedPrimitiveRaw);
}

Wallhack::Wallhack()
		: m_indexedPrimitives()
		, m_shaderapidx9Base(MemoryUtils::lib_base_32(libNames::shaderapidx9))
		, m_orig_DrawIndexedPrimitive(getDrawIndexedPrimitive()) {

	g_Wallhack = this;
	installDrawIndexedPrimitiveHook();
}

Wallhack::~Wallhack() {
	removeDrawIndexedPrimitiveHook();
	g_Wallhack = nullptr;
}

void Wallhack::installDrawIndexedPrimitiveHook() {
	// calculate call-relative hook address
	uintptr_t addr_call_drawIndexedPrimitive = m_shaderapidx9Base + Offsets::shaderapidx9_drawIndexedPrimitive_caller;
	uintptr_t addr_after_call = addr_call_drawIndexedPrimitive + 0x05;
	uintptr_t addr_hook_relative = (uintptr_t) hook_DrawIndexedPrimitive - addr_after_call;

	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_drawIndexedPrimitive + 1, 4);
		// patch this shit
		*(uintptr_t *) (addr_call_drawIndexedPrimitive + 1) = addr_hook_relative;
	}
}

void Wallhack::removeDrawIndexedPrimitiveHook() {
	// calculate call-relative address to original function
	uintptr_t addr_call_drawIndexedPrimitive = m_shaderapidx9Base + Offsets::shaderapidx9_drawIndexedPrimitive_caller;
	uintptr_t addr_after_call = addr_call_drawIndexedPrimitive + 0x05;
	uintptr_t addr_orig_relative = (uintptr_t) m_orig_DrawIndexedPrimitive - addr_after_call;

	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_drawIndexedPrimitive + 1, 4);
		// unpatch this shit
		*(uintptr_t *) (addr_call_drawIndexedPrimitive + 1) = addr_orig_relative;
	}

}

Wallhack::HRESULT Wallhack::hook_DrawIndexedPrimitive(
		uintptr_t thisptr,
		D3DPRIMITIVETYPE d3Dprimitivetype,
		int BaseVertexIndex,
		unsigned MinVertexIndex,
		unsigned NumVertices,
		unsigned startIndex,
		unsigned primCount) {
	auto insertion = g_Wallhack->m_indexedPrimitives.emplace(d3Dprimitivetype,
	                                                         BaseVertexIndex,
	                                                         MinVertexIndex,
	                                                         NumVertices,
	                                                         startIndex,
	                                                         primCount);
	if (insertion.second) {
		std::cout << "Indexed Primitives count: "
		          << g_Wallhack->m_indexedPrimitives.size() << std::endl;
	}
//	glDepthMask(false);
	HRESULT result = g_Wallhack->m_orig_DrawIndexedPrimitive(thisptr,
		                                                         d3Dprimitivetype,
		                                                         BaseVertexIndex,
		                                                         MinVertexIndex,
		                                                         NumVertices,
		                                                         startIndex,
		                                                         primCount);
//	glDepthMask(true);
	return result;
}
