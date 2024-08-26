//
// Created by therdel on 29.06.19.
//
#pragma once

#include <array>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "../Utility.hpp"
#include "DrawHook.hpp"
#include "../Pointers/GameVars.hpp"
#include "../Pointers/overlay_structs.hpp"

struct SDL_Window;

class GUI;

class Aimbot;

class ESP : public DrawHookSubscriber, public Util::NonCopyable, public Util::NonMovable {
public:
	explicit ESP(GameVars gameVars, DrawHook &drawHook, GUI &gui, Aimbot &aimbot);

	~ESP() override;

	auto onDraw(SDL_Window *window) -> void override;

private:
	GameVars gameVars;
	DrawHook &m_drawHook;
	GUI &m_gui;
	Aimbot &m_aimbot;

	// configuration
	float LINEWIDTH = 1.7;
	bool m_enableDrawFov;
	bool m_enableBoxESP;
	bool m_enableLineESP;
	bool m_enableFlagESP;
	const SDL_Color colorT{255, 0, 0, 255};  // RED
	const SDL_Color colorCT{0, 0, 255, 255}; // BLUE
	const SDL_Color colorTflag{255, 255, 0, 255};
	const SDL_Color colorCTflag{0, 255, 0, 255};

	template<typename T, size_t rows, size_t cols>
	struct MatrixRowMajor {
		using row_t = std::array<T, cols>;
		std::array<row_t, rows> m_rows;
	};
	using Mat3x4fRowMajor = MatrixRowMajor<float, 3, 4>;
	glm::mat4 m_mat_perspective;
	glm::mat4 m_mat_normalization;

	// game fields ESP operates on
	float *m_fovHorizDegrees;
	float m_f = 1000000, m_n = 1;
	constexpr static size_t MAX_PLAYERS = 64;
	glm::mat4 *m_mat_viewModel;
	std::pair<int, int> *m_screen_dimensions;
	Mat3x4fRowMajor *m_boneMatrices1;
	Mat3x4fRowMajor *m_boneMatrices2;
	int m_boneMatrices1Amount = 1000;
	int m_boneMatrices2Amount = 1000;

	auto calcMatPerspective() -> glm::mat4;

	auto calcMatNormalization() -> glm::mat4;

	/**
	 * Returns given position in screen coordinates, if visible
	 * @param worldPos position in world space
	 * @return screen x/y coordinates. std::nullopt if not on screen
	 */
	auto world_to_screen(glm::vec3 const &worldPos) const -> std::optional<glm::vec2>;


	auto drawCircleScreen(float cx, float cy, float r, int num_segments, const SDL_Color &color) const -> void;

	auto drawBox(glm::vec3 position,
	             SDL_Color const &color,
	             float height = 100.0f,
	             float orientationYaw = 0.0f,
	             float width = 100.0f) const -> void;

	auto drawScreenCross(glm::vec2 crossPos, float radius, const SDL_Color &color,
	                     bool diagonal = true, float lineToCenterRatio = 0.4) const -> void;

	auto drawLineESP() const -> void;

	auto drawBoxESP() const -> void;

	auto drawFlagESP() const -> void;

	auto drawAimFov() const -> void;

	auto drawAimTargetCross() const -> void;

	auto drawBulletPrediction() const -> void;

	auto drawBoneBoxes() const -> void;
};