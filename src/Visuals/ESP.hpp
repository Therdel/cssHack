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
	constexpr static float FAR_PLANE = 10e9, NEAR_PLANE = 0;
	float m_linewidth = 1.7;
	bool m_enableAimbotTargetCross;
	bool m_enableBulletPredictionCross;
	bool m_enableDrawFov;
	bool m_enableBoxESP;
	bool m_enableLineESP;
	bool m_enableFlagESP;
	constexpr static SDL_Color colorT{255, 0, 0, 255};  // RED
	constexpr static SDL_Color colorCT{0, 0, 255, 255}; // BLUE
	constexpr static SDL_Color colorTflag{255, 255, 0, 255};
	constexpr static SDL_Color colorCTflag{0, 255, 0, 255};

	static glm::mat4 m_mat_unswizzle_game_coords;
	glm::mat4 m_mat_view;
	glm::mat4 m_mat_projection;

	auto calcMatView() -> glm::mat4;
	auto calcMatProjection() -> glm::mat4;

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

	auto drawAimbotTargetCross() const -> void;

	auto drawBulletPrediction() const -> void;
};