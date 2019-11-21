
/*
void Bunnyhop::sendFakeKeyboardTap(int virtualKeyCode)
{
//send down event
keybd_event(
virtualKeyCode,
MapVirtualKey(virtualKeyCode, MAPVK_VK_TO_VSC),
0,
0);

//send up event
keybd_event(
virtualKeyCode,
MapVirtualKey(virtualKeyCode, MAPVK_VK_TO_VSC),
KEYEVENTF_KEYUP,
0);
}
*/

/*
// pointer dereference with a register:
// see http://www.winasm.net/forum/index.php?showtopic=813
void __declspec(naked) Bunnyhop::hook_on_on_ground_inc()
{
  __asm {
    // save register that we'll going to use for pointer deref
    push eax

    // execute operation that we overwrote
    mov eax, [m_on_ground]  // load pointer
    inc [eax]             // use pointer to change pointed-to value

    // cause the jump action
    mov eax, [m_jump]       // load pointer
    mov [eax],5           // use pointer to change pointed-to value

    // restore register
    pop eax

    // jump back to where we came from
    jmp [m_client_addr_on_ground_inc_op_jmp_back]
  }
}
*/

/*
// pointer dereference with a register:
// see http://www.winasm.net/forum/index.php?showtopic=813
void __declspec(naked) Bunnyhop::hook_on_on_ground_dec() {
  __asm {
    // save register that we'll going to use for pointer deref
    push eax

    // execute operation that we overwrote
    mov eax, [m_on_ground]  // load pointer
    dec[eax]              // use pointer to change pointed-to value

    // stop causing the jump action
    mov eax, [m_jump]       // load pointer
    mov[eax], 4           // use pointer to change pointed-to value

    pop eax               // restore register

    // jump back to where we came from
    jmp[m_client_addr_on_ground_dec_op_jmp_back]
  }
}

// blatantly copied
// source: https://www.unknowncheats.me/forum/c-and-c-/183880-linux-detecting-key-held-simulating-getasynckeystate.html
bool Keyboard::isDown(KeySym keySym) const {
	char szKey[32];
	KeyCode keyCode = XKeysymToKeycode(m_display, keySym);

	XQueryKeymap(m_display, szKey);

	return szKey[keyCode / 8] & (1 << (keyCode % 8));
}

*/

/*

void DrawHook::drawDebugClockLine(SDL_Window *window) {
	static float angle = 0;
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	auto radians = [](float degrees) { return (degrees * M_PI) / 180.0; };

	angle += 1;
	angle = std::fmod(angle, 360);
	float cos = std::cos(radians(angle));
	float sin = std::sin(radians(angle));

	// 100px wide, 100px high
	Vec2f circleDimensions{480, 480};
	normalizeScreenCoordinates(circleDimensions, width, height);

	Vec2f lineEnd{cos * circleDimensions.m_x, sin * circleDimensions.m_y};

	glBegin(GL_LINES);
	glColor3f(sin * 0.5 + 0.5,
	          std::sin(radians(angle + 120)) * 0.5 + 0.5,
	          std::sin(radians(angle + 240)) * 0.5 + 0.5); // RGB value
	glVertex2f(0, 0); // Line Origin (screen center)
	glVertex2f(lineEnd.m_x, lineEnd.m_y); // Line end
	glEnd();
}
 */

// CMakeLists.txt
/*
	# make library un-loadable
	# source: https://stackoverflow.com/questions/24467404/dlclose-doesnt-really-unload-shared-object-no-matter-how-many-times-it-is-call
	#COMPILE_FLAGS "-fno-gnu-unique"
 */

// DetourToMethod.hpp
/*  // Windows MSVC x86
	// save ecx register
	// 51             - push ecx
	codeBuf[nextIdx++] = 0x51;

	// put object-pointer in ecx for object method call
	// B9 EFBEADDE    - mov ecx, [object] (DEADBEEF examplifies an actual address)
	codeBuf[nextIdx++] = 0xB9;
	*(uintptr_t *) &codeBuf[nextIdx] = m_object_ptr;
	nextIdx += 4;

	// call method by address relative operation after call instruction
	// E8 EFBEADDE    - call[DEADBEEF] call relative to next instruction
	codeBuf[nextIdx++] = 0xE8;
	uintptr_t method_relative_address = m_method_ptr - (uintptr_t) &codeBuf[nextIdx + 4];
	*(uintptr_t *) &codeBuf[nextIdx] = method_relative_address;
	nextIdx += 4;

	// 59             - pop ecx
	codeBuf[nextIdx++] = 0x59;
#endif
 */

// ForeignPointer.hpp
/*
template<typename T=uintptr_t>
class ForeignPointerBuilderBase {
public:
	ForeignPointerBuilderBase(SharedForeignPointer<T> basePtr)
			: m_basePtr(std::move(basePtr)) {}

	SharedForeignPointer<T> build() const {
		return m_basePtr;
	}

	template<typename U=uintptr_t>
	ForeignPointerBuilderBase<U> offset(ptrdiff_t offset, OffsetType type) {
		return {std::make_shared<ForeignPointerOffset<U>>(m_basePtr, offset, type)};
	}

private:
	SharedForeignPointer<T> m_basePtr;
};

namespace ForeignPointerBuilder {
	ForeignPointerBuilderBase<> base(LibName const &libName);

	template<typename T>
	ForeignPointerBuilderBase<T> base(SharedForeignPointer<T> basePtr) {
		return {std::move(basePtr)};
	}
}

ForeignPointerBuilderBase<> ForeignPointerBuilder::base(LibName const &libName) {
	return {std::make_shared<ForeignPointerBaseLibrary>(libName)};
}
 */

// Input.cpp
/*
bool Input::windowFocused() const {
	// TODO? doesn't work at all
	Window w;
	int revert_to;
	XGetInputFocus(m_display, &w, &revert_to); // see man
	return w != None && w != PointerRoot;
}
 */
/*
// windows includes
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

bool Input::isDown(int virtualKey)
{
  return GetAsyncKeyState(virtualKey) & 0x1;
}

bool Input::isApplicationActivated() {
  HWND activatedHandle = GetForegroundWindow();
  if (activatedHandle == nullptr) {
    return false;       // No window is currently activated
  }

  DWORD procId = GetCurrentProcessId();
  DWORD activeProcId;
  GetWindowThreadProcessId(activatedHandle, &activeProcId);

  return activeProcId == procId;
}
*/
