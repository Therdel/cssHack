//
// Created by Therdel on 02/05/2020.
//
#pragma once
#include <functional>

#include <windows.h>

extern class WinProcHook *g_winProcHook;

class WinProcHook {
public:
	using Callback = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	explicit WinProcHook(Callback callback);

private:
	Callback _callback;
	WNDPROC _lastWndProc;

	static auto _findHwnd() -> HWND;
	auto _installWndProc() -> void;
	static auto CALLBACK _hookWindowProc(_In_ HWND   hwnd,
	                                     _In_ UINT   uMsg,
	                                     _In_ WPARAM wParam,
	                                     _In_ LPARAM lParam) -> LRESULT;
};