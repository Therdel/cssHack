//
// Created by therdel on 01.08.19.
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <queue>

#include "../Utility.hpp"
#include "DrawHook.hpp"
#include "../Input.hpp"
#include "GuiElements.hpp"

struct SDL_KeyboardEvent;
union SDL_Event;
struct SDL_Window;

class GUI : protected DrawHookSubscriber, public Util::NonCopyable, public Util::NonMovable {
public:
	GUI(DrawHook &drawHook, Input &keyboard);

	~GUI() override;

	auto registerFloatSlider(GuiElements::FloatSlider slider) -> void;

	auto registerCheckbox(GuiElements::Checkbox checkBox) -> void;

	auto registerIntSlider(GuiElements::IntSlider slider) -> void;

	auto registerAngleRadSlider(GuiElements::AngleRadSlider slider) -> void;

	auto registerComboBox(GuiElements::ComboBox comboBox) -> void;

	auto registerButton(GuiElements::Button button) -> void;

protected:

	auto onDraw(SDL_Window *window) -> void override;

private:
	DrawHook &m_drawHook;
	Input &m_input;
	ScopedKeyHandler m_guiKeyHandler;
	std::mutex m_debugFloatSlidersMutex;
	std::vector<GuiElements::FloatSlider> m_debugFloatSliders;

	std::mutex m_debugCheckboxesMutex;
	std::vector<GuiElements::Checkbox> m_debugCheckboxes;
	std::mutex m_debugIntSlidersMutex;
	std::vector<GuiElements::IntSlider> m_debugIntSliders;

	std::mutex m_debugAngleSlidersMutex;
	std::vector<GuiElements::AngleRadSlider> m_debugAngleSliders;

	std::mutex m_debugComboBoxesMutex;
	std::vector<GuiElements::ComboBox> m_debugComboBoxes;

	std::mutex m_debugButtonsMutex;
	std::vector<GuiElements::Button> m_debugButtons;

	enum class ImGuiLifecycle { UNINITIALIZED, ACTIVE, SHUTDOWN_PENDING, SHUTDOWN_DONE };
	// transitions:
	//   - UNINITIALIZED→ACTIVE (render thread on first draw),
	//   - ACTIVE→SHUTDOWN_PENDING (~GUI)
	//   - SHUTDOWN_PENDING→SHUTDOWN_DONE (render thread)
	std::mutex m_imguiMutex;
	ImGuiLifecycle m_imguiState;
	std::condition_variable m_imguiCv;

	std::mutex m_showGuiMutex;
	bool m_showGui;

	auto onGuiKey(SDL_KeyboardEvent const &event) -> bool;

	auto initImGui(SDL_Window *window) -> void;

	auto shutdownImGui() -> void;

	auto imGuiNewFrame(SDL_Window *window) -> void;

};
