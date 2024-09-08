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
#include "../Pointers/GameVars.hpp"
#include "../Pointers/Offsets.hpp"

DrawHook *g_DrawHook = nullptr;

DrawHook::DrawHook(const GameVars &gameVars)
		: m_subscribersMutex()
		, m_subscribers()
		, _op_sdl_swapWindow_detour{} {
	g_DrawHook = this;
	_op_sdl_swapWindow_detour.emplace(Util::Address(gameVars.op_sdl_swapWindow_caller),
                                      Util::Address((uintptr_t)(hook_SDL_GL_SwapWindow)));
}

DrawHook::~DrawHook() {
    // TODO: Wireframe mode!
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

auto DrawHook::attachSubscriber(DrawHookSubscriber *sub) -> void {
	std::scoped_lock l_lock(m_subscribersMutex);
	m_subscribers.push_back(sub);
}

auto DrawHook::detachSubscriber(DrawHookSubscriber *sub) -> void {
	std::scoped_lock l_lock(m_subscribersMutex);
	m_subscribers.erase(std::remove(m_subscribers.begin(),
	                                m_subscribers.end(),
	                                sub),
	                    m_subscribers.end());
}

auto DrawHook::callSubscribers(SDL_Window *window) -> void {
	std::scoped_lock l_lock(m_subscribersMutex);
	for (auto &subscriber : m_subscribers) {
		subscriber->onDraw(window);
	}
}

auto DrawHook::hook_SDL_GL_SwapWindow(SDL_Window *window) -> void {
	g_DrawHook->callSubscribers(window);

	SDL_GL_SwapWindow(window);

	// TODO: Wireframe mode!
//	glClearColor(0, 0, 0, 1);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
