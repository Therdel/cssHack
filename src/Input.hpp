//
// Created by therdel on 15.06.19.
//
#pragma once

#include <functional>
#include <mutex>
#include <map>
#include <optional>

#include <SDL_keyboard.h>

#include "Utility.hpp"
#include "Pointers/SharedGamePointer.hpp"

struct KeyStroke {
	constexpr explicit KeyStroke(SDL_Keycode keycode, uint16_t modifiers = KMOD_NONE)
			: m_keycode(keycode)
			, m_modifiers(modifiers) {}

	/// sorts key first, modifiers second.
	/// those without modifiers compare equally pairwise
	/// and sort after those with modifiers
	bool operator<(KeyStroke const &other) const {
		if (m_keycode != other.m_keycode) {
			return m_keycode < other.m_keycode;
		} else {
			if (m_modifiers == KMOD_NONE) {
				return false;
			} else {
				return m_modifiers < other.m_modifiers;
			}
		}
	}

	SDL_Keycode m_keycode;
	uint16_t m_modifiers;
};

constexpr KeyStroke key_inject(SDLK_F10);
constexpr KeyStroke key_eject(SDLK_F11);

constexpr KeyStroke key_aim(SDLK_LSHIFT);
constexpr KeyStroke key_trigger(SDLK_CAPSLOCK);
constexpr KeyStroke key_bhop(SDLK_SPACE);
constexpr KeyStroke key_gui(SDLK_INSERT);

struct SDL_KeyboardEvent;
union SDL_Event;

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
	SharedGamePointer<uintptr_t> m_op_sdl_pollEvent_call;
	std::mutex m_keyHandlersMutex;
	std::map<KeyStroke, keyHandler> m_keyHandlers;

	std::mutex m_mouseHandlerMutex;
	std::optional<mouseHandler> m_mouseHandler;

	std::mutex m_allEventConsumerMutex;
	std::optional<eventHandler> m_allEventConsumer;

	void installPollEventHook();

	void removePollEventHook();

	std::optional<bool> callKeyHandlerIfExists(SDL_KeyboardEvent const &event);

	static int hook_SDL_PollEvent(SDL_Event *callerEvent);

	int detour_SDL_PollEvent(SDL_Event *callerEvent);
};

// TODO: This is ugly as fuck. Should be a singleton.
extern Input *g_keyboard;

class ScopedKeyHandler : public Util::NonCopyable {
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
