//
// Created by therdel on 29.06.19.
//
#pragma once

#include <array>
#include <optional>

#include "../Utility.hpp"
#include "DrawHook.hpp"
#include "../Player.hpp"
#include "../Mat4f.hpp"
#include "../Vec2f.hpp"

struct SDL_Window;

class GUI;

class Aimbot;

class ESP : public DrawHookSubscriber, public Utility::NonCopyable, public Utility::NonMovable {
public:
	explicit ESP(DrawHook &drawHook, GUI &gui, Aimbot &aimbot);

	~ESP() override;

	void onDraw(SDL_Window *window) override;

private:
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
	struct Matrix {
		using row_t = std::array<T, cols>;
		std::array<row_t, rows> m_rows;
	};
	using Mat3x4f = Matrix<float, 3, 4>;
	Mat4f m_mat_perspective;
	Mat4f m_mat_normalization;

	// game fields ESP operates on
	float *m_fovHorizDegrees;
	float m_f = 1000000, m_n = 1;
	constexpr static size_t MAX_PLAYERS = 64;
	Mat4f *m_mat_viewModel;
	std::array<Player, MAX_PLAYERS> *m_players;
	Vec3f *m_player_pos;
	Vec3f *m_player_angles_vis;
	std::pair<int, int> *m_screen_dimensions;
	Mat3x4f *m_boneMatrices1;
	Mat3x4f *m_boneMatrices2;
	int m_boneMatrices1Amount = 1000;
	int m_boneMatrices2Amount = 1000;

	void calcMatPerspective();

	void calcMatNormalization();

	/**
	 * Returns given position in screen coordinates, if visible
	 * @param worldPos position in world space
	 * @return screen x/y coordinates. std::nullopt if not on screen
	 */
	std::optional<Vec2f> world_to_screen(Vec3f const &worldPos) const;


	void drawCircleScreen(float cx, float cy, float r, int num_segments, const SDL_Color &color) const;

	void drawBox(Vec3f position,
	             SDL_Color const &color,
	             float height = 100.0f,
	             float orientationYaw = 0.0f,
	             float width = 100.0f) const;

	void drawScreenCross(Vec2f crossPos, float radius, const SDL_Color &color,
	                     bool diagonal = true, float lineToCenterRatio = 0.4) const;

	void drawLineESP() const;

	void drawBoxESP() const;

	void drawFlagESP() const;

	void drawAimFov() const;

	void drawAimTargetCross() const;

	void drawBulletPrediction() const;

	void drawBoneBoxes() const;
};