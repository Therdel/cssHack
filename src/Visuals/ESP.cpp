//
// Created by therdel on 29.06.19.
//

#include <SDL.h>    // SDL_Window
#include <GL/gl.h>

#include "ESP.hpp"
#include "../MemoryUtils.hpp"
#include "../Pointers/libNames.hpp"
#include "../Pointers/Offsets.hpp"
#include "GUI.hpp"
#include "../Aimbot.hpp"
#include "../Utility.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../Log.hpp"
#include "../Mat4f.hpp"

using namespace Util;

ESP::ESP(DrawHook &drawHook, GUI &gui, Aimbot &aimbot)
		: m_drawHook(drawHook)
		, m_gui(gui)
		, m_aimbot(aimbot)
		, m_enableDrawFov(true)
		, m_enableBoxESP(true)
		, m_enableLineESP(false)
		, m_enableFlagESP(true) {
	// read module base addresses
	const uintptr_t l_client_base = MemoryUtils::lib_base_32(libNames::client);
	const uintptr_t l_engine_base = MemoryUtils::lib_base_32(libNames::engine);
	const uintptr_t l_matsystem_base = MemoryUtils::lib_base_32(libNames::materialsystem);

	m_gui.registerCheckbox({m_enableDrawFov, "ESP Fov"});
	m_gui.registerCheckbox({m_enableBoxESP, "ESP Boxes"});
	m_gui.registerCheckbox({m_enableLineESP, "ESP Lines"});
	m_gui.registerCheckbox({m_enableFlagESP, "ESP Flags"});
	m_gui.registerFloatSlider({0, 4, LINEWIDTH, "ESP Linewidth"});

	m_fovHorizDegrees = (float *) (l_engine_base + Offsets::engine_fov_horizontal);
	m_mat_viewModel = (Mat4f *) (l_client_base + Offsets::client_matViewModel);

	uintptr_t l_client_player_base_addr = *(uintptr_t *) (l_client_base + Offsets::client_player_p_base);
	m_players = (decltype(m_players)) (l_client_player_base_addr + Offsets::client_player_p_off);

	m_player_pos = (Vec3f *) (l_engine_base + Offsets::engine_player_pos);
	m_player_angles_vis = (Vec3f *) (l_client_base + Offsets::client_viewAngleVis);
	m_screen_dimensions = (std::pair<int, int> *) (l_engine_base + Offsets::engine_screenDimensions);
	m_drawHook.attachSubscriber(this);

	auto **l_boneMatrices1_base = (Mat3x4f **) (l_matsystem_base + 0x16016C);
	m_boneMatrices1 = *l_boneMatrices1_base;

	auto **l_boneMatrices2_base = (Mat3x4f **) (l_matsystem_base + 0x160164);
	m_boneMatrices2 = *l_boneMatrices2_base;
}

ESP::~ESP() {
	m_drawHook.detachSubscriber(this);
}

auto ESP::onDraw(SDL_Window *) -> void {
	// update transformation matrices
	calcMatPerspective();
	calcMatNormalization();

	// enable line antialiasing, setting linewidth and  "normal" alpha channel
	GLboolean lastBlendEnabled = glIsEnabled(GL_BLEND);
	GLfloat lastLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lastLineWidth);
	GLboolean lastLineSmoothEnabled = glIsEnabled(GL_LINE_SMOOTH);
	GLenum lastSmoothHint;
	glGetIntegerv(GL_LINE_SMOOTH_HINT, (GLint *) &lastSmoothHint);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	{
//		drawBox(Vec3f{0, 0, 0}, {255, 0, 255, 180});
		if (m_enableBoxESP) {
			drawBoxESP();
		}
		if (m_enableLineESP) {
			drawLineESP();
		}
		//	drawBoneBoxes();
		if (m_enableFlagESP) {
			drawFlagESP();
		}
		if (m_enableDrawFov) {
			drawAimFov();
		}
		drawAimTargetCross();
		drawBulletPrediction();
	}

	// restore previous opengl state
	glLineWidth(lastLineWidth);
	if (!lastLineSmoothEnabled) {
		glDisable(GL_LINE_SMOOTH);
	}
	glHint(GL_LINE_SMOOTH_HINT, lastSmoothHint);
	if (!lastBlendEnabled) {
		glDisable(GL_BLEND);
	}
}

auto ESP::calcMatPerspective() -> void {
	m_mat_perspective.columns = {Vec4f{1, 0, 0, 0},
	                             Vec4f{0, 1, 0, 0},
	                             Vec4f{0, 0, 1 + (m_f / m_n), m_f},
	                             Vec4f{0, 0, -1 / m_n, 0}};
}

auto ESP::calcMatNormalization() -> void {
	auto &screenW = m_screen_dimensions->first;
	auto &screenH = m_screen_dimensions->second;

	// source: https://www.khronos.org/opengl/wiki/GluPerspective_code
	float ymax, xmax;
	xmax = m_n * tanf(toRadians(*m_fovHorizDegrees) / 2.0f);
	ymax = xmax * screenH / screenW;
	float m_r = xmax, m_l = -xmax, m_t = ymax, m_b = -ymax;

	// explanation:
	// m_r = right  clip
	// m_l = left   clip
	// m_t = top    clip
	// m_b = bottom clip
	// m_n = near   clipping plane
	// m_f = far    clipping plane

	m_mat_normalization.columns = {Vec4f{2 / (m_r - m_l), 0, 0, -(m_r + m_l) / (m_r - m_l)},
	           Vec4f{0, 2 / (m_t - m_b), 0, -(m_t + m_b) / (m_t - m_b)},
	           Vec4f{0, 0, -2 * (m_f - m_n), -(m_f + m_n) / (m_f - m_n)},
	           Vec4f{0, 0, 0, 1}};
}

// source: Youtube HazardEdit - Gamehacking#13 ESP Overlay
// https://www.youtube.com/watch?v=GgTQod8Kp0k
auto ESP::world_to_screen(Vec3f const &worldPos) const -> std::optional<Vec2f>{
	// as in https://stackoverflow.com/questions/8491247/c-opengl-convert-world-coords-to-screen2d-coords
	// also see: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
	// TODO if the modelViewMatrix is unavailable:
	// https://gamedev.stackexchange.com/questions/168542/camera-view-matrix-from-position-yaw-pitch-worldup
	std::optional<Vec2f> l_result(std::nullopt);
	Vec4f world_homogenous{worldPos.m_x, worldPos.m_y, worldPos.m_z, 1};
	Vec4f view_homogenous = m_mat_viewModel->matMulCol(world_homogenous);

	auto projected_homogenous = m_mat_perspective.matMulCol(view_homogenous);
	// check if vertice is in front of screen
	if (projected_homogenous.m_w > 0.1f) {
		auto normalized_homogenous = m_mat_normalization.matMulCol(projected_homogenous);
		auto normalized_cartesian = normalized_homogenous.homogenous_to_cartesian();

		l_result = Vec2f{normalized_cartesian.m_x, normalized_cartesian.m_y};
	}
	return l_result;
}

auto ESP::drawBox(Vec3f position, SDL_Color const &color, float height, float orientationYaw, float width) const -> void {
	constexpr size_t squareVertices = 4;
	// describes a square of sidelength 1 centered at origin in the x/y plane
	static std::array<Vec3f, squareVertices> square{
			Vec3f{-0.5, -0.5, 0},
			Vec3f{0.5, -0.5, 0},
			Vec3f{0.5, 0.5, 0},
			Vec3f{-0.5, 0.5, 0}
	};

	glLineWidth(LINEWIDTH);
	glBegin(GL_LINES);
	glColor4ubv((GLubyte *) &color);
	// draw bottom & top planes
	for (size_t idxStart = 0; idxStart < squareVertices; ++idxStart) {
		size_t idxEnd = idxStart + 1;
		if (idxEnd >= squareVertices) {
			idxEnd = 0;
		}

		auto verticeStart = rotateAroundZ(square[idxStart], orientationYaw);
		auto verticeEnd = rotateAroundZ(square[idxEnd], orientationYaw);

		auto bottomBegVert = position + verticeStart * width;
		auto bottomEndVert = position + verticeEnd * width;
		auto topBegVert = position + verticeStart * width + Vec3f{0, 0, height};
		auto topEndVert = position + verticeEnd * width + Vec3f{0, 0, height};

		auto bottomBegWorld = world_to_screen(bottomBegVert);
		auto bottomEndWorld = world_to_screen(bottomEndVert);
		auto topBegWorld = world_to_screen(topBegVert);
		auto topEndWorld = world_to_screen(topEndVert);

		if (bottomBegWorld.has_value() &&
		    bottomEndWorld.has_value()) {
			// draw bottom square
			glVertex2f(bottomBegWorld->m_x, bottomBegWorld->m_y);
			glVertex2f(bottomEndWorld->m_x, bottomEndWorld->m_y);
		}
		if (topBegWorld.has_value() &&
		    topEndWorld.has_value()) {
			// draw top square
			glVertex2f(topBegWorld->m_x, topBegWorld->m_y);
			glVertex2f(topEndWorld->m_x, topEndWorld->m_y);
		}
		if (bottomBegWorld.has_value() &&
		    topBegWorld.has_value()) {
			// draw lines connecting top/bottom (side faces)
			glVertex2f(bottomBegWorld->m_x, bottomBegWorld->m_y);
			glVertex2f(topBegWorld->m_x, topBegWorld->m_y);
		}
	}
	glEnd();
}

auto ESP::drawLineESP() const -> void {
	glLineWidth(LINEWIDTH);
	glBegin(GL_LINES);
	for (auto &player : *m_players) {
		if (!player.isActive()) {
			continue;
		}
		auto screen_cartesian = world_to_screen(player.m_pos);
		if (screen_cartesian.has_value()) {
			if (player.m_team == Player::TEAM::T) {
				glColor3ub(255, 0, 0); // RED
			} else {
				glColor3ub(0, 0, 255); // BLUE
			}
			glVertex2f(0, 0); // Line origin - screen center
			glVertex2f(screen_cartesian->m_x, screen_cartesian->m_y); // Line end
		}
	}
	glEnd();
}

auto ESP::drawBoxESP() const -> void {
	static SDL_Color colorT{255, 0, 0, 255};  // RED
	static SDL_Color colorCT{0, 0, 255, 255}; // BLUE

	for (auto &player: *m_players) {
		if (!player.isActive()) {
			continue;
		}
		// FIXME: Find out which entity in the list is the player without dist
		// ensure there's no box drawn around our player
		if (player.m_pos.distanceTo(*m_player_pos) >= 20.0f) {
			drawBox(player.m_pos,
			        player.m_team == Player::TEAM::T ? colorT : colorCT,
			        -65.0f,
			        player.m_viewangles.m_y,
			        30.0f);
		}
	}
}

auto ESP::drawFlagESP() const -> void {
	static constexpr float l_flagHeight = 40.0;
	static constexpr float l_flagSize = 30.0;
	static Vec3f l_flagLowPointOff{0, 0, l_flagHeight};
	static Vec3f l_flagHighPointOff = l_flagLowPointOff + Vec3f{0, 0, l_flagSize};
	// flag points towards x-axis on null-orientation
	static Vec3f l_flagTipOff = l_flagLowPointOff + Vec3f{l_flagSize, 0, l_flagSize / 2.0};

	glLineWidth(LINEWIDTH);
	for (auto &player : *m_players) {
		if (!player.isActive()) {
			continue;
		}
		Vec3f l_flagLowWorld = l_flagLowPointOff + player.m_pos;
		Vec3f l_flagHighWorld = l_flagHighPointOff + player.m_pos;
		Vec3f l_flagTipWorld = player.m_pos;
		l_flagTipWorld += Util::rotateAroundZ(l_flagTipOff, player.m_viewangles.m_y);

		auto l_playerPosScreen = world_to_screen(player.m_pos);
		auto l_flagLowScreen = world_to_screen(l_flagLowWorld);
		auto l_flagHighScreen = world_to_screen(l_flagHighWorld);
		auto l_flagTipScreen = world_to_screen(l_flagTipWorld);
		if (l_playerPosScreen && l_flagLowScreen && l_flagHighScreen && l_flagTipScreen) {
//			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			// draw flag
			{
				glDisable(GL_CULL_FACE);
				{
					// draw flag surface
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					if (player.m_team == Player::TEAM::T) {
						glColor4ubv((const GLubyte *) &colorTflag);
					} else {
						glColor4ubv((const GLubyte *) &colorCTflag);
					}
					glBegin(GL_TRIANGLES);
					glVertex2f(l_flagLowScreen->m_x, l_flagLowScreen->m_y);
					glVertex2f(l_flagHighScreen->m_x, l_flagHighScreen->m_y);
					glVertex2f(l_flagTipScreen->m_x, l_flagTipScreen->m_y);
					glEnd();
					// draw flag border
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					if (player.m_team == Player::TEAM::T) {
						glColor4ubv((const GLubyte *) &colorT);
					} else {
						glColor4ubv((const GLubyte *) &colorCT);
					}
					glBegin(GL_TRIANGLES);
					glVertex2f(l_flagLowScreen->m_x, l_flagLowScreen->m_y);
					glVertex2f(l_flagHighScreen->m_x, l_flagHighScreen->m_y);
					glVertex2f(l_flagTipScreen->m_x, l_flagTipScreen->m_y);
					glEnd();
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				glEnable(GL_CULL_FACE);
				{
					glBegin(GL_LINES);

					// draw flag pole
					glVertex2f(l_playerPosScreen->m_x, l_playerPosScreen->m_y);
					glVertex2f(l_flagHighScreen->m_x, l_flagHighScreen->m_y);
					glEnd();
				}
			}
		}
	}
}

auto ESP::drawBoneBoxes() const -> void {
	SDL_Color color{255, 255, 255, 255};
	Vec3f origin(0, 0, 0);
	for (int i = 0; i < m_boneMatrices1Amount; ++i) {
		Mat3x4f &l_bone = m_boneMatrices1[i];
		Vec3f l_bonePos{
				l_bone.m_rows[0][3],
				l_bone.m_rows[1][3],
				l_bone.m_rows[2][3]
		};
		if (l_bonePos != origin &&
		    l_bonePos.distanceTo(*m_player_pos) > 100) {
			drawBox(l_bonePos, color, 10, 0, 10);
		}
	}
	for (int i = 0; i < m_boneMatrices2Amount; ++i) {
		Mat3x4f &l_bone = m_boneMatrices2[i];
		Vec3f l_bonePos{
				l_bone.m_rows[0][3],
				l_bone.m_rows[1][3],
				l_bone.m_rows[2][3]
		};
		if (l_bonePos != origin &&
		    l_bonePos.distanceTo(*m_player_pos) > 100) {
			drawBox(l_bonePos, color, 10, 0, 10);
		}
	}
}

// adapted from: http://slabode.exofire.net/circle_draw.shtml
auto ESP::drawCircleScreen(float cx, float cy, float r, int num_segments, const SDL_Color &color) const -> void {
	float aspectYoverX =
			static_cast<float>(m_screen_dimensions->second) /
			static_cast<float>(m_screen_dimensions->first);
	float theta = 2 * 3.1415926 / float(num_segments);
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = r;//we start at angle = 0
	float y = 0;


	glLineWidth(LINEWIDTH);
	glBegin(GL_LINE_LOOP);
	glColor4ubv((GLubyte *) &color);
	for (int ii = 0; ii < num_segments; ii++) {
		float xAspectCorrected = (x + cx) * aspectYoverX;
		glVertex2f(xAspectCorrected, y + cy);//output vertex

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
	glEnd();
}

auto ESP::drawAimFov() const -> void {
	// get screen coordinates of a point on the fov circle
	// TODO: Get circle radius from dummy projection independent of camera position.
	Vec3f l_anglesOnFovRing = *m_player_angles_vis;
	l_anglesOnFovRing.m_x += toDegrees(m_aimbot.m_aim_fov_rad);
	auto l_pointOnFovRing = viewAnglesToUnitvector(l_anglesOnFovRing);

	auto l_fovPointWorld = *m_player_pos + l_pointOnFovRing;
	auto l_fovPointScreen = world_to_screen(l_fovPointWorld);

	if (l_fovPointScreen.has_value()) {
		// y coordinate is the circles radius
		auto l_fovScreenY = l_fovPointScreen->m_y;

		drawCircleScreen(0, 0, l_fovScreenY,
		                 100,
		                 {128, 128, 128, 128});
	}
}

static auto rotate(Vec2f &point, float degrees) -> void {
	point = point.rotate(Util::toRadians(degrees));
}

auto ESP::drawScreenCross(Vec2f crossPos, float radius, const SDL_Color &color, bool diagonal,
                          float lineToCenterRatio) const -> void {

	float aspectYoverX =
			static_cast<float>(m_screen_dimensions->second) /
			static_cast<float>(m_screen_dimensions->first);

	crossPos += {
		1.0f/m_screen_dimensions->first,
		-1.0f/m_screen_dimensions->second
	};

	// defines an arm of the cross in screen center
	Vec2f vertexOuter{radius, 0.0f}, vertexInner{radius * lineToCenterRatio, 0};

	if (diagonal) {
		// rotate cross into diagonal orientation
		rotate(vertexOuter, 45);
		rotate(vertexInner, 45);
	}

	GLfloat lastLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lastLineWidth);
	glLineWidth(1.9);

	glBegin(GL_LINES);
	glColor4ubv((GLubyte *) &color);
	{
		for(int i=0; i<4; ++i) {
			auto vertexOuterFinal = vertexOuter;
			auto vertexInnerFinal = vertexInner;
			// correct for aspect ratio
			vertexOuterFinal.m_x *= aspectYoverX;
			vertexInnerFinal.m_x *= aspectYoverX;
			// move cross to desired location
			vertexOuterFinal += crossPos;
			vertexInnerFinal += crossPos;

			if(i < 3) {
				rotate(vertexOuter, 90);
				rotate(vertexInner, 90);
			}

			glVertex2f(vertexOuterFinal.m_x, vertexOuterFinal.m_y);
			glVertex2f(vertexInnerFinal.m_x, vertexInnerFinal.m_y);
		}
	}
	glEnd();

	glLineWidth(lastLineWidth);
}

auto ESP::drawAimTargetCross() const -> void {
	std::optional<Aimbot::AimTarget> const &l_currentTarget = m_aimbot.getCurrentTarget();

	if (l_currentTarget.has_value()) {
		auto l_targetUnitVector = viewAnglesToUnitvector(l_currentTarget->anglesToTarget);
		auto l_pointOnTargetSightLine = *m_player_pos + l_targetUnitVector;
		auto l_targetOnScreen = world_to_screen(l_pointOnTargetSightLine);

		if (l_targetOnScreen.has_value()) {
			drawScreenCross(*l_targetOnScreen, 0.06, {87, 237, 14, 255}, false);

//			const float width = 10;
//			SDL_Color color{255, 0, 0, 255};
//			Vec3f position{l_currentTarget->aimPoint};
//			position.m_z -= width / 2;
//
//			drawBox(position,
//			        color,
//			        width,
//			        l_currentTarget->target.m_viewangles.m_y,
//			        width);
		}
	}
}

auto ESP::drawBulletPrediction() const -> void {
	Vec3f l_bulletPredictionUnitVec = viewAnglesToUnitvector(m_aimbot.getBulletPredictionAngles());
	Vec3f l_bulletPointWorld = *m_player_pos + l_bulletPredictionUnitVec;
	auto l_bulletPointScreen = world_to_screen(l_bulletPointWorld);

	if (l_bulletPointScreen.has_value()) {
		drawScreenCross(*l_bulletPointScreen, 0.04, {237, 14, 14, 255}, true);
	}
}