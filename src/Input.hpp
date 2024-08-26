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
#include "CallDetour.hpp"

struct KeyStroke {
	constexpr explicit KeyStroke(SDL_Keycode keycode, uint16_t modifiers = KMOD_NONE)
			: m_keycode(keycode)
			, m_modifiers(modifiers) {}

	/// sorts key first, modifiers second.
	/// those without modifiers compare equally pairwise
	/// and sort after those with modifiers
	auto  operator<(KeyStroke const &other) const -> bool {
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

struct GameVars;
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

	Input(const GameVars&);

	auto isDown(SDL_Keycode key) const -> bool;

	auto setKeyHandler(KeyStroke key, keyHandler callback) -> void;

	auto removeKeyHandler(KeyStroke key) -> bool;

	auto setMouseHandler(mouseHandler callback) -> void;

	auto removeMouseHandler() -> bool;

	auto setAllEventConsumer(eventHandler callback) -> void;

	auto removeAllEventConsumer() -> void;

	static auto injectEvent(SDL_Event *event) -> void;

private:
	std::mutex m_keyHandlersMutex;
	std::map<KeyStroke, keyHandler> m_keyHandlers;

	std::mutex m_mouseHandlerMutex;
	std::optional<mouseHandler> m_mouseHandler;

	std::mutex m_allEventConsumerMutex;
	std::optional<eventHandler> m_allEventConsumer;

	CallDetour _op_sdl_pollEvent_detour;

	std::optional<bool> callKeyHandlerIfExists(SDL_KeyboardEvent const &event);

	static auto hook_SDL_PollEvent(SDL_Event *callerEvent) -> int;

	auto detour_SDL_PollEvent(SDL_Event *callerEvent) -> int;
};

// TODO: This is ugly as fuck. Should be a singleton.
extern Input *g_keyboard;

class ScopedKeyHandler : public Util::NonCopyable {
public:
	ScopedKeyHandler(Input &keyboard,
	                 KeyStroke key,
	                 Input::keyHandler handler) noexcept;

	ScopedKeyHandler(ScopedKeyHandler &&other) noexcept;

	~ScopedKeyHandler();

	auto operator=(ScopedKeyHandler &&other) noexcept -> ScopedKeyHandler&;

private:
	Input *m_input;
	KeyStroke m_key;
	bool m_valid;
};
