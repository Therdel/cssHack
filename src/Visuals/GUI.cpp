//
// Created by therdel on 01.08.19.
//
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>

#include "Gui_imgui_impl_sdl.h"

// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)

#include <GL/gl3w.h>    // Initialize with gl3wInit()

#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include "GUI.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../Log.hpp"
#include "GuiElements.hpp"
#include <thread>

using namespace GuiElements;

GUI::GUI(DrawHook &drawHook, Input &keyboard)
		: m_drawHook(drawHook)
		, m_input(keyboard)
		, m_guiKeyHandler(m_input,
		                  key_gui,
		                  [&](SDL_KeyboardEvent const &event) {
			                  return onGuiKey(event);
		                  })
		, m_initImGuiMutex{}
		, m_didInitImGui(false)
		, m_showGuiMutex{}
		, m_showGui{false} {
	m_drawHook.attachSubscriber(this);
}

GUI::~GUI() {
	if (std::scoped_lock showGuiLock{m_showGuiMutex};
		m_showGui) {
		m_input.removeMouseHandler();
	}
	m_drawHook.detachSubscriber(this);
	shutdownImGui();
}

auto GUI::registerFloatSlider(FloatSlider slider) -> void {
	std::scoped_lock l_lock(m_debugFloatSlidersMutex);
	m_debugFloatSliders.push_back(std::move(slider));
}

auto GUI::registerCheckbox(Checkbox checkBox) -> void {
	std::scoped_lock l_lock(m_debugCheckboxesMutex);
	m_debugCheckboxes.push_back(std::move(checkBox));
}

auto GUI::registerIntSlider(IntSlider slider) -> void {
	std::scoped_lock l_lock(m_debugIntSlidersMutex);
	m_debugIntSliders.push_back(std::move(slider));
}

auto GUI::registerAngleRadSlider(AngleRadSlider slider) -> void {
	std::scoped_lock l_lock(m_debugAngleSlidersMutex);
	m_debugAngleSliders.push_back(std::move(slider));
}

auto GUI::registerComboBox(ComboBox comboBox) -> void {
	std::scoped_lock l_lock(m_debugComboBoxesMutex);
	m_debugComboBoxes.push_back(std::move(comboBox));
}

auto GUI::registerButton(Button button) -> void {
	std::scoped_lock l_lock(m_debugButtonsMutex);
	m_debugButtons.push_back(std::move(button));
}

auto GUI::onGuiKey(SDL_KeyboardEvent const &event) -> bool {
	if (event.type == SDL_KEYDOWN && event.repeat == 0) {
		std::scoped_lock showGuiLock{m_showGuiMutex};
		m_showGui = !m_showGui;
		if (m_showGui) {
			m_input.setAllEventConsumer([this](SDL_Event const &event) {
				const bool steal_mouseevent = ImGui_ImplSDL2_ProcessEvent(&event);
				return steal_mouseevent;
			});
		} else {
			m_input.removeAllEventConsumer();
		}
	}
	return true;
}

auto GUI::onDraw(SDL_Window *window) -> void {
	imGuiNewFrame(window);
}

auto GUI::initImGui(SDL_Window *window) -> void {
	const char *glsl_version = "#version 130";

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err) {
		std::string l_error("Failed to initialize OpenGL loader!");
		Log::log<Log::FLUSH>(l_error);
		throw std::runtime_error(l_error);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto &io = ImGui::GetIO();
	(void) io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
//	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, SDL_GL_GetCurrentContext());
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

auto GUI::shutdownImGui() -> void {
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

auto GUI::imGuiNewFrame(SDL_Window *window) -> void {
	std::scoped_lock initImGuiLock{m_initImGuiMutex};
	if (!m_didInitImGui) {
		initImGui(window);
		m_didInitImGui = true;
	}

	// Start the Dear ImGui frame
	auto &io = ImGui::GetIO();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	if (std::scoped_lock showGuiLock{m_showGuiMutex};
		m_showGui) {
		io.MouseDrawCursor = true;
		ImGui::Begin("Knerz");                          // Create a window called "Hello, world!" and append into it.
		{
			std::scoped_lock l_lock(m_debugFloatSlidersMutex);
			for (auto &slider : m_debugFloatSliders) {
				ImGui::SliderFloat(slider.m_description.c_str(),
				                   &slider.m_value,
				                   slider.m_rangeBegin,
				                   slider.m_rangeEnd);
			}
		}
		{
			std::scoped_lock l_lock(m_debugCheckboxesMutex);
			if (!m_debugCheckboxes.empty()) {
				ImGui::Text("Debug checkboxes:");
				for (auto &checkbox : m_debugCheckboxes) {
					ImGui::Checkbox(checkbox.m_description.c_str(),
					                &checkbox.m_value);
				}
			}
		}
		{
			std::scoped_lock l_lock(m_debugIntSlidersMutex);
			for (auto &slider : m_debugIntSliders) {
				ImGui::SliderInt(slider.m_description.c_str(),
				                 &slider.m_value,
				                 slider.m_rangeBegin,
				                 slider.m_rangeEnd);
			}
		}
		{
			std::scoped_lock l_lock(m_debugAngleSlidersMutex);
			for (auto &slider : m_debugAngleSliders) {
				ImGui::SliderAngle(slider.m_description.c_str(),
				                   &slider.m_value,
				                   slider.m_rangeBeginDegrees,
				                   slider.m_rangeEndDegrees);
			}
		}
		{
			std::scoped_lock l_lock(m_debugComboBoxesMutex);
			for (auto &comboBox : m_debugComboBoxes) {
				if (ImGui::BeginCombo(comboBox.m_description.c_str(),
				                      comboBox.m_currentSelection->c_str())) {
					for (auto option_it = comboBox.m_options.begin();
					     option_it != comboBox.m_options.end();
					     ++option_it) {
						bool selected = &(*comboBox.m_currentSelection) == &(*option_it);
						if (ImGui::Selectable(option_it->c_str(), selected)) {
							comboBox.m_currentSelection = option_it;
							auto selectedIndex = option_it - comboBox.m_options.begin();
							comboBox.m_onChangeHandler(selectedIndex);
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

			}
		}
		{
			std::scoped_lock l_lock(m_debugButtonsMutex);
			for (auto &button : m_debugButtons) {
				if (ImGui::Button(button.m_description.c_str())) {
					button.m_callback();
				}
			}
		}
		ImGui::End();
	} else {
		io.MouseDrawCursor = false;
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
