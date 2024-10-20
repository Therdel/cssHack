//
// Created by therdel on 01.08.19.
//
#pragma once

#include <atomic>
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

	// imgui stuff
	std::mutex m_initImGuiMutex;
	bool m_didInitImGui;
	std::mutex m_showGuiMutex;
	bool m_showGui;

	auto onGuiKey(SDL_KeyboardEvent const &event) -> bool;

	auto initImGui(SDL_Window *window) -> void;

	auto shutdownImGui() -> void;

	auto imGuiNewFrame(SDL_Window *window) -> void;

};
