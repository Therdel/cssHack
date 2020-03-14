//
// Created by therdel on 29.06.19.
//

#include <algorithm>    // std::remove

#include <SDL.h>    // SDL_Window, SDL_GL_SwapWindow
//#include <SDL_opengl.h>
#include <GL/gl.h>

#include "DrawHook.hpp"
#include "../MemoryUtils.hpp"
#include "../Pointers/libNames.hpp"
#include "../Pointers/Offsets.hpp"

DrawHook *g_DrawHook = nullptr;

DrawHook::DrawHook()
		: m_subscribersMutex()
		, m_subscribers()
		, m_launcherBase{MemoryUtils::lib_base_32(libNames::launcher)} {

	g_DrawHook = this;
	installSwapWindowHook();
}

DrawHook::~DrawHook() {
	removeSwapWindowHook();
	g_DrawHook = nullptr;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DrawHook::attachSubscriber(DrawHookSubscriber *sub) {
	std::scoped_lock l_lock(m_subscribersMutex);
	m_subscribers.push_back(sub);
}

void DrawHook::detachSubscriber(DrawHookSubscriber *sub) {
	std::scoped_lock l_lock(m_subscribersMutex);
	m_subscribers.erase(std::remove(m_subscribers.begin(),
	                                m_subscribers.end(),
	                                sub),
	                    m_subscribers.end());
}

void DrawHook::installSwapWindowHook() {
	// calculate call-relative hook address
	uintptr_t addr_call_swapWindow = m_launcherBase + Offsets::launcher_sdl_swapWindow_caller;
	uintptr_t addr_after_call = addr_call_swapWindow + 0x05;
	uintptr_t addr_hook_relative = (uintptr_t) hook_SDL_GL_SwapWindow - addr_after_call;

	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_swapWindow + 1, 4);
		// patch this shit
		*(uintptr_t *) (addr_call_swapWindow + 1) = addr_hook_relative;
	}
}

void DrawHook::removeSwapWindowHook() {
	// calculate call-relative address to original function
	uintptr_t addr_call_swapWindow = m_launcherBase + Offsets::launcher_sdl_swapWindow_caller;
	uintptr_t addr_after_call = addr_call_swapWindow + 0x05;
	uintptr_t addr_orig_relative = (uintptr_t) SDL_GL_SwapWindow - addr_after_call;

	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_swapWindow + 1, 4);
		// unpatch this shit
		*(uintptr_t *) (addr_call_swapWindow + 1) = addr_orig_relative;
	}

}

void DrawHook::callSubscribers(SDL_Window *window) {
	std::scoped_lock l_lock(m_subscribersMutex);
	for (auto &subscriber : m_subscribers) {
		subscriber->onDraw(window);
	}
}

void DrawHook::hook_SDL_GL_SwapWindow(SDL_Window *window) {
	g_DrawHook->callSubscribers(window);

	SDL_GL_SwapWindow(window);

	// TODO: Wireframe mode!
//	glClearColor(0, 0, 0, 1);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
