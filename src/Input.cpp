//
// Created by therdel on 15.06.19.
//
#include <optional>

#include <SDL.h>

#include "Input.hpp"
#include "MemoryUtils.hpp"
#include "Pointers/GameVars.hpp"

Input *g_keyboard{nullptr};

Input::Input(const GameVars &gameVars)
		: m_keyHandlersMutex()
		, m_keyHandlers()
		, m_mouseHandlerMutex()
		, m_mouseHandler()
		, _op_sdl_pollEvent_detour() {
	g_keyboard = this;
	_op_sdl_pollEvent_detour.emplace(Util::Address(gameVars.op_sdl_pollEvent_caller),
                                     Util::Address((uintptr_t)(hook_SDL_PollEvent)));
}

auto Input::isDown(SDL_Keycode key) const -> bool {
	uint8_t const *keyState = SDL_GetKeyboardState(nullptr);
	SDL_Scancode scancode = SDL_GetScancodeFromKey(key);

	return keyState[scancode] == 1;
}

auto Input::setKeyHandler(KeyStroke key, keyHandler callback) -> void {
	std::scoped_lock lock(m_keyHandlersMutex);
	m_keyHandlers[key] = std::move(callback);
}

auto Input::removeKeyHandler(KeyStroke key) -> bool {
	std::scoped_lock lock(m_keyHandlersMutex);

	auto amountErased = m_keyHandlers.erase(key);
	return amountErased != 0;
}

auto Input::setMouseHandler(Input::mouseHandler callback) -> void {
	std::scoped_lock l_lock(m_mouseHandlerMutex);
	m_mouseHandler = std::move(callback);
}

auto Input::removeMouseHandler() -> bool {
	bool l_success = false;
	if (m_mouseHandler.has_value()) {
		m_mouseHandler.reset();
		l_success = true;
	}
	return l_success;
}

auto Input::callKeyHandlerIfExists(SDL_KeyboardEvent const &event) -> std::optional<bool> {
	KeyStroke keyStroke{event.keysym.sym, event.keysym.mod};

	std::scoped_lock l_lock(m_keyHandlersMutex);
	// check if handler exists
	auto handlerIt = g_keyboard->m_keyHandlers.find(keyStroke);
	if (handlerIt != g_keyboard->m_keyHandlers.end()) {
		// execute handler
		bool doSteal = handlerIt->second(event);
		return doSteal;
	}

	return std::nullopt;
}

auto Input::setAllEventConsumer(Input::eventHandler callback) -> void {
	std::scoped_lock l_lock(m_allEventConsumerMutex);
	m_allEventConsumer = std::move(callback);
}

auto Input::removeAllEventConsumer() -> void {
	std::scoped_lock l_lock(m_allEventConsumerMutex);
	m_allEventConsumer.reset();
}

auto Input::injectEvent(SDL_Event *event) -> void {
	SDL_PushEvent(event);
}

//static auto fakeMouseMotionEvent(Vec2f const &pos) -> SDL_Event {
//	SDL_Event result;
//	memset(&result, '\0', sizeof(result));
//
//	result.type = SDL_MOUSEMOTION;
//	result.motion.timestamp = std::numeric_limits<decltype(result.motion.timestamp)>::max();
//	result.motion.x = static_cast<int>(pos.m_x);
//	result.motion.y = static_cast<int>(pos.m_y);
//
//	return result;
//}

static auto getNopEvent() -> SDL_Event {
	SDL_Event result;
	memset(&result, '\0', sizeof(result));

//	result.type = SDL_MOUSEMOTION;
//	auto &motionEvent = result.motion;
//	motionEvent.state = SDL_GetMouseState(&motionEvent.x, &motionEvent.y);
//	motionEvent.xrel = 0;
//	motionEvent.yrel = 0;

//	result.type = SDL_MOUSEWHEEL;
//	result.wheel.timestamp =std::numeric_limits<decltype(result.wheel.timestamp)>::max();
//	result.wheel.x = result.wheel.y = 0;
//	result.wheel.direction = SDL_MOUSEWHEEL_NORMAL;

	result.type = SDL_KEYUP;
	result.key.timestamp = (std::numeric_limits<decltype(result.key.timestamp)>::max)();
	result.key.state = SDL_RELEASED;
	result.key.repeat = 0;
	result.key.keysym.sym = key_gui.m_keycode;

	return result;
}

auto SDLCALL Input::hook_SDL_PollEvent(SDL_Event *callerEvent) -> int {
	return g_keyboard->detour_SDL_PollEvent(callerEvent);
}

auto Input::detour_SDL_PollEvent(SDL_Event *callerEvent) -> int {
	if (callerEvent == nullptr) {
		return SDL_PollEvent(callerEvent);
	}

	bool steal = false;

	SDL_Event event;
	int eventExists = SDL_PollEvent(&event);
	if (eventExists == 1) {
		{
			std::scoped_lock l_lock(m_allEventConsumerMutex);
			if (m_allEventConsumer) {
				const KeyStroke keystroke{event.key.keysym.sym, event.key.keysym.mod};
				const bool is_unconsumable_special_key = 
					(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
					(keystroke == key_gui || keystroke == key_inject || keystroke == key_eject);
				steal = !is_unconsumable_special_key && (*m_allEventConsumer)(event);
			}
		}
		if (!steal) {
			switch (event.type) {
				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP: {
					std::scoped_lock lock(m_mouseHandlerMutex);
					if (m_mouseHandler.has_value()) {
						steal = (*m_mouseHandler)(event);
					}
					break;
				}
				case SDL_KEYDOWN:
				case SDL_KEYUP: {
					std::optional<bool> doSteal = callKeyHandlerIfExists(event.key);
					if (doSteal.has_value()) {
						steal = doSteal.value();
					}
					break;
				}
//			useful if trying to block textinput into input fields
//			case SDL_TEXTINPUT:
//				steal = false;
//				break;
				default:
					break;
			}
		}
	}

	if (steal) {
		// replace the original event with one of no effect to steal the original
		// to not break any calling loop on SDL_PollEvent
		*callerEvent = getNopEvent();
	} else {
		if (eventExists == 1) {
			// forward event to caller
			*callerEvent = event;
		}
	}

	return eventExists;
}

ScopedKeyHandler::ScopedKeyHandler(Input &keyboard,
                                   KeyStroke key,
                                   Input::keyHandler handler) noexcept
		: m_input(&keyboard)
		, m_key(key)
		, m_valid(true) {
	keyboard.setKeyHandler(key, std::move(handler));
}

ScopedKeyHandler::ScopedKeyHandler(ScopedKeyHandler &&other) noexcept
		: m_input(other.m_input)
		, m_key(other.m_key)
		, m_valid(other.m_valid) {
	other.m_valid = false;
}

ScopedKeyHandler::~ScopedKeyHandler() {
	if (m_valid) {
		m_input->removeKeyHandler(m_key);
	}
}

auto ScopedKeyHandler::operator=(ScopedKeyHandler &&other) noexcept -> ScopedKeyHandler& {
	m_input = other.m_input;
	m_key = other.m_key;
	m_valid = other.m_valid;
	other.m_valid = false;

	return *this;
}
