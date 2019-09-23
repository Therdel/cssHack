//
// Created by therdel on 01.08.19.
//
#pragma once

#include <atomic>
#include <queue>

#include "../Utility.hpp"
#include "DrawHook.hpp"
#include "../Input.hpp"
#include "../Vec2f.hpp"
#include "GuiElements.hpp"

struct SDL_KeyboardEvent;
union SDL_Event;
struct SDL_Window;

class GUI : protected DrawHookSubscriber, public Utility::NonCopyable, public Utility::NonMovable {
public:
	GUI(DrawHook &drawHook, Input &keyboard);

	~GUI() override;

	void registerFloatSlider(GuiElements::FloatSlider slider);

	void registerCheckbox(GuiElements::Checkbox checkBox);

	void registerIntSlider(GuiElements::IntSlider slider);

	void registerAngleRadSlider(GuiElements::AngleRadSlider slider);

	void registerComboBox(GuiElements::ComboBox comboBox);

	void registerButton(GuiElements::Button button);

protected:

	void onDraw(SDL_Window *window) override;

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
	bool m_didInit;
	bool m_show;

	bool onGuiKey(SDL_KeyboardEvent const &event);

	bool onInputEvent(SDL_Event const &event);

	void initImGui(SDL_Window *window);

	void shutdownImGui();

	void imGuiNewFrame(SDL_Window *window);

};
