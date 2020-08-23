//
// Created by therdel on 29.06.19.
//
#pragma once

#include <mutex>
#include <vector>
#include "../Utility.hpp"

struct SDL_Window;

class DrawHookSubscriber {
public:
	virtual void onDraw(SDL_Window *window) = 0;

	virtual ~DrawHookSubscriber() = default;
};

class DrawHook : public Util::NonCopyable, public Util::NonMovable {
public:
	DrawHook();

	~DrawHook();

	void attachSubscriber(DrawHookSubscriber *sub);

	void detachSubscriber(DrawHookSubscriber *sub);

private:
	std::mutex m_subscribersMutex;
	std::vector<DrawHookSubscriber *> m_subscribers;
	const uintptr_t m_launcherBase;

	// hooking stuff
	void installSwapWindowHook();

	void removeSwapWindowHook();

	void callSubscribers(SDL_Window *window);

	static void hook_SDL_GL_SwapWindow(SDL_Window *window);
};

// FIXME: race condition on use when drawhook is being destroyed
extern DrawHook *g_DrawHook;