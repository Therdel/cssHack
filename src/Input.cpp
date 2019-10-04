//
// Created by therdel on 15.06.19.
//
#include <optional>

#include <SDL.h>

#include "Input.hpp"
#include "MemoryUtils.hpp"
#include "Pointers/GamePointerFactory.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "Log.hpp"

using SDL_PollEvent_t = decltype(&SDL_PollEvent);
static SDL_PollEvent_t g_pOrigSDL_PollEvent;

Input *g_keyboard{nullptr};

Input::Input()
		: m_op_sdl_pollEvent_call(GamePointerFactory::get(GamePointerDef::op_sdl_pollEvent_call()))
		, m_keyHandlersMutex()
		, m_keyHandlers()
		, m_mouseHandlerMutex()
		, m_mouseHandler() {
	g_keyboard = this;
	installPollEventHook();
}

Input::~Input() {
	removePollEventHook();
	g_keyboard = nullptr;
}

bool Input::isDown(SDL_Keycode key) const {
	uint8_t const *keyState = SDL_GetKeyboardState(nullptr);
	SDL_Scancode scancode = SDL_GetScancodeFromKey(key);

	return keyState[scancode] == 1;
}

void Input::setKeyHandler(KeyStroke key, keyHandler callback) {
	std::lock_guard<std::mutex> lock(m_keyHandlersMutex);
	m_keyHandlers[key] = std::move(callback);
}

bool Input::removeKeyHandler(KeyStroke key) {
	std::lock_guard<std::mutex> lock(m_keyHandlersMutex);

	auto amountErased = m_keyHandlers.erase(key);
	return amountErased != 0;
}

void Input::setMouseHandler(Input::mouseHandler callback) {
	std::lock_guard<std::mutex> l_lock(m_mouseHandlerMutex);
	m_mouseHandler = std::move(callback);
}

bool Input::removeMouseHandler() {
	bool l_success = false;
	if (m_mouseHandler.has_value()) {
		m_mouseHandler.reset();
		l_success = true;
	}
	return l_success;
}

void Input::installPollEventHook() {

	// calculate call-relative hook address
	uintptr_t addr_call_pollEvent = m_op_sdl_pollEvent_call;
	uintptr_t *p_call_dest_relative = (uintptr_t *) (addr_call_pollEvent + 1);
	uintptr_t addr_after_call = addr_call_pollEvent + 0x05;
	uintptr_t addr_hook_relative = (uintptr_t) hook_SDL_PollEvent - addr_after_call;

	g_pOrigSDL_PollEvent = reinterpret_cast<SDL_PollEvent_t>(*p_call_dest_relative + addr_after_call);
	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_pollEvent + 1, 4);
		// patch this shit
		*(uintptr_t *) (addr_call_pollEvent + 1) = addr_hook_relative;
	}
}

void Input::removePollEventHook() {
	// calculate call-relative address to original function
	uintptr_t addr_call_pollEvent = m_op_sdl_pollEvent_call;
	uintptr_t addr_after_call = addr_call_pollEvent + 0x05;
	//uintptr_t addr_orig_relative = (uintptr_t) SDL_PollEvent - addr_after_call;
	uintptr_t addr_orig_relative = (uintptr_t) g_pOrigSDL_PollEvent - addr_after_call;

	{
		auto scoped_reprotect = MemoryUtils::scoped_remove_memory_protection(addr_call_pollEvent + 1, 4);
		// unpatch this shit
		*(uintptr_t *) (addr_call_pollEvent + 1) = addr_orig_relative;
	}
}

std::optional<bool> Input::callKeyHandlerIfExists(SDL_KeyboardEvent const &event) {
	KeyStroke keyStroke{event.keysym.sym, event.keysym.mod};

	std::lock_guard<std::mutex> l_lock(m_keyHandlersMutex);
	// check if handler exists
	auto handlerIt = g_keyboard->m_keyHandlers.find(keyStroke);
	if (handlerIt != g_keyboard->m_keyHandlers.end()) {
		// execute handler
		bool doSteal = handlerIt->second(event);
		return doSteal;
	}

	return std::nullopt;
}

void Input::setAllEventConsumer(Input::eventHandler callback) {
	std::lock_guard<std::mutex> l_lock(m_allEventConsumerMutex);
	m_allEventConsumer = std::move(callback);
}

void Input::removeAllEventConsumer() {
	std::lock_guard<std::mutex> l_lock(m_allEventConsumerMutex);
	m_allEventConsumer.reset();
}

void Input::injectEvent(SDL_Event *event) {
	SDL_PushEvent(event);
}

//static SDL_Event fakeMouseMotionEvent(Vec2f const &pos) {
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

static SDL_Event getNopEvent() {
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

int SDLCALL Input::hook_SDL_PollEvent(SDL_Event *callerEvent) {
	return g_keyboard->detour_SDL_PollEvent(callerEvent);
}

int Input::detour_SDL_PollEvent(SDL_Event *callerEvent) {
	if (callerEvent == nullptr) {
		return SDL_PollEvent(callerEvent);
	}

	bool steal = false;

	SDL_Event event;
	//int eventExists = SDL_PollEvent(&event);
	int eventExists = g_pOrigSDL_PollEvent(&event);
	if (eventExists == 1) {
		{
			std::lock_guard<std::mutex> l_lock(m_allEventConsumerMutex);
			if (m_allEventConsumer) {
				(*m_allEventConsumer)(event);
				steal = true;
			}
		}
		if (!steal) {
			switch (event.type) {
				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP: {
					std::lock_guard<std::mutex> lock(m_mouseHandlerMutex);
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

ScopedKeyHandler &ScopedKeyHandler::operator=(ScopedKeyHandler &&other) noexcept {
	m_input = other.m_input;
	m_key = other.m_key;
	m_valid = other.m_valid;
	other.m_valid = false;

	return *this;
}

ScopedKeyHandler::~ScopedKeyHandler() {
	if (m_valid) {
		m_input->removeKeyHandler(m_key);
	}
}
