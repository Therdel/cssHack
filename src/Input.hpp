//
// Created by therdel on 15.06.19.
//
#pragma once

#ifdef __linux__

#include <map>
#include <functional>
#include <mutex>
#include <string>

#include <SDL_keyboard.h>

#include "Utility.hpp"

#else
#include <windows.h>
#endif

struct KeyStroke {
	constexpr explicit KeyStroke(SDL_Keycode keycode, uint16_t modifiers = KMOD_NONE)
			: m_keycode(keycode)
			, m_modifiers(modifiers) {}

	/// sorts key first, modifiers second.
	/// those without modifiers compare equally pairwise
	/// and sort after those with modifiers
	bool operator<(KeyStroke const &other) const {
		if(m_keycode != other.m_keycode) {
			return m_keycode < other.m_keycode;
		} else {
			if(m_modifiers == KMOD_NONE) {
				return false;
			} else {
				return m_modifiers < other.m_modifiers;
			}
		}
	}

	SDL_Keycode m_keycode;
	uint16_t m_modifiers;
};

#ifdef __linux__
constexpr KeyStroke key_inject(SDLK_F10);
constexpr KeyStroke key_eject(SDLK_F11);

constexpr KeyStroke key_aim(SDLK_LSHIFT);
constexpr KeyStroke key_trigger(SDLK_CAPSLOCK);
constexpr KeyStroke key_bhop(SDLK_SPACE);
constexpr KeyStroke key_gui(SDLK_INSERT);
#else
#define key_eject_comb_0    VK_LCONTROL
#define key_eject_comb_1    0x45

#define key_inject_comb_0    VK_LCONTROL
#define key_inject_comb_1    0x49

#define key_aim             VK_SHIFT
#define key_trigger         VK_CAPITAL
#define key_bhop            VK_LMENU
#endif

#ifdef __linux__

struct SDL_KeyboardEvent;
union SDL_Event;

struct _XDisplay;
typedef struct _XDisplay Display;

class Input {
public:
	/// receives a key event.
	/// return value determines if the event should be hidden from the game
	using keyHandler = std::function<bool(SDL_KeyboardEvent const &)>;

	/// receives mouse motion/button/wheel events
	/// return value determines if the event should be hidden from the game
	using mouseHandler = std::function<bool(SDL_Event const &)>;

	using eventHandler = std::function<void(SDL_Event const &)>;

	Input();

	~Input();

	bool isDown(SDL_Keycode key) const;

	void setKeyHandler(KeyStroke key, keyHandler callback);

	bool removeKeyHandler(KeyStroke key);

	void setMouseHandler(mouseHandler callback);

	bool removeMouseHandler();

	void setAllEventConsumer(eventHandler callback);

	void removeAllEventConsumer();

	static void injectEvent(SDL_Event *event);

private:
	Display *m_display;
	std::mutex m_keyHandlersMutex;
	std::map<KeyStroke, keyHandler> m_keyHandlers;

	std::mutex m_mouseHandlerMutex;
	std::optional<mouseHandler> m_mouseHandler;

	std::mutex m_allEventConsumerMutex;
	std::optional<eventHandler> m_allEventConsumer;

	static void installPollEventHook();

	static void removePollEventHook();

	std::optional<bool> callKeyHandlerIfExists(SDL_KeyboardEvent const &event);

	static int hook_SDL_PollEvent(SDL_Event *callerEvent);

	int detour_SDL_PollEvent(SDL_Event *callerEvent);
};

// TODO: This is ugly as fuck. Should be a singleton.
extern Input *g_keyboard;

#else

class Input {
public:
	static bool isDown(int virtualKey);

	bool isApplicationActivated();
};
#endif

class ScopedKeyHandler : public Utility::NonCopyable {
public:
	ScopedKeyHandler(Input &keyboard,
	                 KeyStroke key,
	                 Input::keyHandler handler) noexcept;

	ScopedKeyHandler(ScopedKeyHandler &&other) noexcept;

	ScopedKeyHandler &operator=(ScopedKeyHandler &&other) noexcept;

	~ScopedKeyHandler();

private:
	Input *m_input;
	KeyStroke m_key;
	bool m_valid;
};