//
// Created by Therdel on 02/05/2020.
//
#include "WinProcHook.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"

WinProcHook *g_winProcHook{nullptr};

WinProcHook::WinProcHook(Callback callback)
: _callback{std::move(callback)}
, _lastWndProc{nullptr} {
	_installWndProc();
}

auto WinProcHook::_findHwnd() -> HWND {
	struct Parameters {
		DWORD pId = GetCurrentProcessId();
		HWND hWnd = nullptr;
	} parameters;
	// source: https://stackoverflow.com/questions/4232381/handle-to-window-handle
	auto iterate_func = [](HWND hwnd, LPARAM lp) -> BOOL {
		auto *parameters = (Parameters*)(lp);
		DWORD processId;
		GetWindowThreadProcessId(hwnd, &processId);
		if (processId == parameters->pId) {
			parameters->hWnd = hwnd;
			return FALSE;
		}
		return TRUE;
	};
	BOOL (*iterate_func_ptr)(HWND, LPARAM) = iterate_func;
	EnumWindows((WNDENUMPROC)(iterate_func_ptr), (LPARAM)&parameters);

	if(parameters.hWnd == nullptr) {
		// TODO: can (almost) not happen
	}
	return parameters.hWnd;
}

auto WinProcHook::_installWndProc() -> void {
	HWND hWnd = _findHwnd();
	SetLastError(0);
	auto result = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (long)_hookWindowProc);
	auto lastError = GetLastError();
	if (result == 0 && lastError != 0) {
		// TODO: handle error, should not happen
		Log::log("Error: WndProc install failed");
	} else {
		_lastWndProc = (WNDPROC)result;
	}
}

auto CALLBACK WinProcHook::_hookWindowProc(_In_ HWND   hwnd,
                                           _In_ UINT   uMsg,
                                           _In_ WPARAM wParam,
                                           _In_ LPARAM lParam) -> LRESULT {
	auto keyDown = GetAsyncKeyState(VK_SPACE) < 0;
	if (not keyDown) {
		// forward message to application
		return CallWindowProc(g_winProcHook->_lastWndProc,
		                      hwnd,
		                      uMsg,
		                      wParam,
		                      lParam);
	}
	return 0;
}