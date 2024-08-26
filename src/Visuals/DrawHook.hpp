//
// Created by therdel on 29.06.19.
//
#pragma once

#include <mutex>
#include <vector>

#include "../Utility.hpp"
#include "../CallDetour.hpp"

struct GameVars;
struct SDL_Window;

class DrawHookSubscriber {
public:
	virtual auto onDraw(SDL_Window *window) -> void = 0;

	virtual ~DrawHookSubscriber() = default;
};

class DrawHook : public Util::NonCopyable, public Util::NonMovable {
public:
	DrawHook(const GameVars&);
	~DrawHook();

	auto attachSubscriber(DrawHookSubscriber *sub) -> void;
	auto detachSubscriber(DrawHookSubscriber *sub) -> void;

private:
	std::mutex m_subscribersMutex;
	std::vector<DrawHookSubscriber *> m_subscribers;
    CallDetour _op_sdl_swapWindow_detour;

	auto callSubscribers(SDL_Window *window) -> void;

	static auto hook_SDL_GL_SwapWindow(SDL_Window *window) -> void;
};

// FIXME: race condition on use when drawhook is being destroyed
extern DrawHook *g_DrawHook;