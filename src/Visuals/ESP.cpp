//
// Created by therdel on 29.06.19.
//

#include <SDL.h>    // SDL_Window
#include <GL/gl.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "ESP.hpp"
#include "../MemoryUtils.hpp"
#include "../Pointers/libNames.hpp"
#include "../Pointers/Offsets.hpp"
#include "GUI.hpp"
#include "../Aimbot.hpp"
#include "../Utility.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX

#include "../Log.hpp"

using namespace Util;

// TODO: drawing doesn't work when cl_showpos is on
ESP::ESP(GameVars gameVars, DrawHook &drawHook, GUI &gui, Aimbot &aimbot)
		: gameVars{gameVars}
		, m_drawHook(drawHook)
		, m_gui(gui)
		, m_aimbot(aimbot)
		, m_enableAimbotTargetCross(true)
		, m_enableBulletPredictionCross(true)
		, m_enableDrawFov(true)
		, m_enableBoxESP(true)
		, m_enableLineESP(false)
		, m_enableFlagESP(true)
	m_gui.registerCheckbox({m_enableAimbotTargetCross, "ESP Aimbot Target Cross"});
	m_gui.registerCheckbox({m_enableBulletPredictionCross, "ESP Bullet Prediction Cross"});
	m_gui.registerCheckbox({m_enableDrawFov, "ESP Fov"});
	m_gui.registerCheckbox({m_enableBoxESP, "ESP Boxes"});
	m_gui.registerCheckbox({m_enableLineESP, "ESP Lines"});
	m_gui.registerCheckbox({m_enableFlagESP, "ESP Flags"});
	m_gui.registerFloatSlider({0, 4, m_linewidth, "ESP Linewidth"});

	m_drawHook.attachSubscriber(this);
}

ESP::~ESP() {
	m_drawHook.detachSubscriber(this);
}

auto ESP::onDraw(SDL_Window *) -> void {
	// update transformation matrices
	m_mat_view = calcMatView();
	m_mat_projection = calcMatProjection();

	// Save game's OpenGL state
	glPushAttrib(GL_ALL_ATTRIB_BITS); // Save all attributes
	glPushMatrix(); // Save current matrix state

	// Save game's OpenGL matrices Set up the modelview and projection matrices
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	{

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

		glLineWidth(m_linewidth);


		{
			drawBox(glm::vec3{0, 0, 0}, {255, 0, 255, 180});
			// drawBox(debugBoxPos, {255, 0, 255, 180});
		if (m_enableBoxESP) {
			drawBoxESP();
		}
		if (m_enableLineESP) {
			drawLineESP();
		}
		if (m_enableFlagESP) {
			drawFlagESP();
		}
		if (m_enableDrawFov) {
			drawAimFov();
		}
			if (m_enableAimbotTargetCross) {
				drawAimbotTargetCross();
			}
			if (m_enableBulletPredictionCross) {
		drawBulletPrediction();
	}
		}
	}

	////// Restore game OpenGL matrices
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Restore game OpenGL attributes
	glPopAttrib(); // Restore all attributes
	glPopMatrix(); // Restore the matrix state
}

// (un-)swizzle game world coords format from (z, x, y) to (x, y, z)
// weirdly, we have to negate an axis for this to work
glm::mat4 ESP::m_mat_unswizzle_game_coords = [] {
	// 0  1  0  0
	// 0  0  1  0
	// 1  0  0  0
	// 0  0  0  1
	const auto unswizzle_coords = glm::transpose(glm::mat4{
		0,  1,  0,  0,
		0,  0,  1,  0,
		1,  0,  0,  0,
		0,  0,  0,  1,
		});

	const auto flip_x_axis = glm::scale(glm::vec3{-1, 1, -1});

	return flip_x_axis * unswizzle_coords;
}();

auto ESP::calcMatView() -> glm::mat4 {
	// Game coordinate system
	//   Orientation format: (x, y, z)
	//   Position format: (z, x, y)
	//   Default orientation: (0, 0, 0) is looking in positive z axis

	const glm::vec3 orientation = {-gameVars.angles_visual.x, gameVars.angles_visual.y, -gameVars.angles_visual.z};

	const glm::vec3 position = m_mat_unswizzle_game_coords * glm::vec4{gameVars.player_pos, 1};

	const glm::mat4 rotation_matrix = glm::eulerAngleYXZ(
				glm::radians(orientation.y), glm::radians(orientation.x), glm::radians(orientation.z));

	const glm::mat4 translation_matrix = glm::translate(position);

	// apply rotation first, translation second
	const glm::mat4 camera_transform = translation_matrix * rotation_matrix;

	// View transform must use inverses, because it must undo the camera pos/rot towards origin.
	return glm::inverse(camera_transform);
}

auto ESP::calcMatProjection() -> glm::mat4 {
	return glm::perspective(
		(float)glm::radians(gameVars.fov_vertical_degrees),
		(float)gameVars.screen_dimensions.first/(float)gameVars.screen_dimensions.second,
		(float)debugNear,
		(float)debugFar);
}

// source: Youtube HazardEdit - Gamehacking#13 ESP Overlay
// https://www.youtube.com/watch?v=GgTQod8Kp0k
auto ESP::world_to_screen(glm::vec3 const &worldPos) const -> std::optional<glm::vec2> {
	// as in https://stackoverflow.com/questions/8491247/c-opengl-convert-world-coords-to-screen2d-coords
	// also see: https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
	// TODO if the modelViewMatrix is unavailable:
	// https://gamedev.stackexchange.com/questions/168542/camera-view-matrix-from-position-yaw-pitch-worldup
	std::optional<glm::vec2> l_result(std::nullopt);

	glm::vec4 world_homogenous_swizzled{worldPos, 1};
	glm::vec4 world_homogenous = m_mat_unswizzle_game_coords * world_homogenous_swizzled;
	glm::vec4 view_homogenous = m_mat_view * world_homogenous;

	// check if vertex is in front of screen
	if (glm::vec4 projected_homogenous = m_mat_projection * view_homogenous;
		projected_homogenous.w > 0.1f) {
		auto projected_cartesian = glm::vec3{projected_homogenous} / projected_homogenous.w;

		l_result = glm::vec2{projected_cartesian.x, projected_cartesian.y};
	}
	return l_result;
}

auto ESP::drawBox(glm::vec3 position, SDL_Color const &color, float height, float orientationYaw, float width) const -> void {
	// describes a square of sidelength 1 centered at origin in the x/y plane
	static std::array square{
		glm::vec3{-0.5, -0.5, 0},
		glm::vec3{0.5, -0.5, 0},
		glm::vec3{0.5, 0.5, 0},
		glm::vec3{-0.5, 0.5, 0}
	};

	glLineWidth(m_linewidth);
	glBegin(GL_LINES);
	glColor4ubv((GLubyte *) &color);
	// draw bottom & top planes
	constexpr size_t squareVertices = 4;
	for (size_t idxStart = 0; idxStart < squareVertices; ++idxStart) {
		size_t idxEnd = idxStart + 1;
		if (idxEnd >= squareVertices) {
			idxEnd = 0;
		}

		auto verticeStart = rotateAroundZ(square[idxStart], orientationYaw);
		auto verticeEnd = rotateAroundZ(square[idxEnd], orientationYaw);

		auto bottomBegVert = position + verticeStart * width;
		auto bottomEndVert = position + verticeEnd * width;
		auto topBegVert = position + verticeStart * width + glm::vec3{0, 0, height};
		auto topEndVert = position + verticeEnd * width + glm::vec3{0, 0, height};

		auto bottomBegWorld = world_to_screen(bottomBegVert);
		auto bottomEndWorld = world_to_screen(bottomEndVert);
		auto topBegWorld = world_to_screen(topBegVert);
		auto topEndWorld = world_to_screen(topEndVert);

		if (bottomBegWorld.has_value() &&
		    bottomEndWorld.has_value()) {
			// draw bottom square
			glVertex2f(bottomBegWorld->x, bottomBegWorld->y);
			glVertex2f(bottomEndWorld->x, bottomEndWorld->y);
		}
		if (topBegWorld.has_value() &&
		    topEndWorld.has_value()) {
			// draw top square
			glVertex2f(topBegWorld->x, topBegWorld->y);
			glVertex2f(topEndWorld->x, topEndWorld->y);
		}
		if (bottomBegWorld.has_value() &&
		    topBegWorld.has_value()) {
			// draw lines connecting top/bottom (side faces)
			glVertex2f(bottomBegWorld->x, bottomBegWorld->y);
			glVertex2f(topBegWorld->x, topBegWorld->y);
		}
	}
	glEnd();
}

auto ESP::drawLineESP() const -> void {
	glLineWidth(m_linewidth);
	glBegin(GL_LINES);
	for (auto &player : gameVars.radar_struct.players) {
		if (!player.isActive()) {
			continue;
		}
		auto screen_cartesian = world_to_screen(player.m_pos);
		if (screen_cartesian.has_value()) {
			if (player.m_team == overlay_structs::Player::TEAM::T) {
				glColor3ub(255, 0, 0); // RED
			} else {
				glColor3ub(0, 0, 255); // BLUE
			}
			glVertex2f(0, 0); // Line origin - screen center
			glVertex2f(screen_cartesian->x, screen_cartesian->y); // Line end
		}
	}
	glEnd();
}

auto ESP::drawBoxESP() const -> void {
	static SDL_Color colorT{255, 0, 0, 255};  // RED
	static SDL_Color colorCT{0, 0, 255, 255}; // BLUE

	for (auto &player: gameVars.radar_struct.players) {
		if (!player.isActive()) {
			continue;
		}
		// FIXME: Find out which entity in the list is the player without dist
		// ensure there's no box drawn around our player
		if (glm::distance(player.m_pos, gameVars.player_pos) >= 20.0f) {
			drawBox(player.m_pos,
			        player.m_team == overlay_structs::Player::TEAM::T ? colorT : colorCT,
			        -65.0f,
			        player.m_viewangles.y,
			        30.0f);
		}
	}
}

auto ESP::drawFlagESP() const -> void {
	static constexpr float l_flagHeight = 40.0;
	static constexpr float l_flagSize = 30.0;
	static glm::vec3 l_flagLowPointOff{0, 0, l_flagHeight};
	static glm::vec3 l_flagHighPointOff = l_flagLowPointOff + glm::vec3{0, 0, l_flagSize};
	// flag points towards x-axis on null-orientation
	static glm::vec3 l_flagTipOff = l_flagLowPointOff + glm::vec3{l_flagSize, 0, l_flagSize / 2.0};

	glLineWidth(m_linewidth);
	for (auto &player : gameVars.radar_struct.players) {
		if (!player.isActive()) {
			continue;
		}
		glm::vec3 l_flagLowWorld = l_flagLowPointOff + player.m_pos;
		glm::vec3 l_flagHighWorld = l_flagHighPointOff + player.m_pos;
		glm::vec3 l_flagTipWorld = player.m_pos;
		l_flagTipWorld += Util::rotateAroundZ(l_flagTipOff, player.m_viewangles.y);

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
					if (player.m_team == overlay_structs::Player::TEAM::T) {
						glColor4ubv((const GLubyte *) &colorTflag);
					} else {
						glColor4ubv((const GLubyte *) &colorCTflag);
					}
					glBegin(GL_TRIANGLES);
					glVertex2f(l_flagLowScreen->x, l_flagLowScreen->y);
					glVertex2f(l_flagHighScreen->x, l_flagHighScreen->y);
					glVertex2f(l_flagTipScreen->x, l_flagTipScreen->y);
					glEnd();
					// draw flag border
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					if (player.m_team == overlay_structs::Player::TEAM::T) {
						glColor4ubv((const GLubyte *) &colorT);
					} else {
						glColor4ubv((const GLubyte *) &colorCT);
					}
					glBegin(GL_TRIANGLES);
					glVertex2f(l_flagLowScreen->x, l_flagLowScreen->y);
					glVertex2f(l_flagHighScreen->x, l_flagHighScreen->y);
					glVertex2f(l_flagTipScreen->x, l_flagTipScreen->y);
					glEnd();
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				glEnable(GL_CULL_FACE);
				{
					glBegin(GL_LINES);

					// draw flag pole
					glVertex2f(l_playerPosScreen->x, l_playerPosScreen->y);
					glVertex2f(l_flagHighScreen->x, l_flagHighScreen->y);
					glEnd();
				}
			}
		}
	}
}

// adapted from: http://slabode.exofire.net/circle_draw.shtml
auto ESP::drawCircleScreen(float cx, float cy, float r, int num_segments, const SDL_Color &color) const -> void {
	float aspectYoverX =
			static_cast<float>(gameVars.screen_dimensions.second) /
			static_cast<float>(gameVars.screen_dimensions.first);
	float theta = 2 * 3.1415926 / float(num_segments);
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = r;//we start at angle = 0
	float y = 0;


	glLineWidth(m_linewidth);
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
	glm::vec3 l_anglesOnFovRing = gameVars.angles_visual;
	l_anglesOnFovRing.x += toDegrees(m_aimbot.m_aim_fov_rad);
	auto l_pointOnFovRing = viewAnglesToUnitvector(l_anglesOnFovRing);

	auto l_fovPointWorld = gameVars.player_pos + l_pointOnFovRing;
	auto l_fovPointScreen = world_to_screen(l_fovPointWorld);

	if (l_fovPointScreen.has_value()) {
		// y coordinate is the circles radius
		auto l_fovScreenY = l_fovPointScreen->y;

		drawCircleScreen(0, 0, l_fovScreenY,
		                 100,
		                 {128, 128, 128, 128});
	}
}

static auto rotate(glm::vec2 &point, float degrees) -> void {
	point = glm::rotate(point, Util::toRadians(degrees));
}

auto ESP::drawScreenCross(glm::vec2 crossPos, float radius, const SDL_Color &color, bool diagonal,
                          float lineToCenterRatio) const -> void {

	float aspectYoverX =
			static_cast<float>(gameVars.screen_dimensions.second) /
			static_cast<float>(gameVars.screen_dimensions.first);

	crossPos += glm::vec2{
		1.0f/gameVars.screen_dimensions.first,
		-1.0f/gameVars.screen_dimensions.second
	};

	// defines an arm of the cross in screen center
	glm::vec2 vertexOuter{radius, 0.0f}, vertexInner{radius * lineToCenterRatio, 0};

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
			vertexOuterFinal.x *= aspectYoverX;
			vertexInnerFinal.x *= aspectYoverX;
			// move cross to desired location
			vertexOuterFinal += crossPos;
			vertexInnerFinal += crossPos;

			if(i < 3) {
				rotate(vertexOuter, 90);
				rotate(vertexInner, 90);
			}

			glVertex2f(vertexOuterFinal.x, vertexOuterFinal.y);
			glVertex2f(vertexInnerFinal.x, vertexInnerFinal.y);
		}
	}
	glEnd();

	glLineWidth(lastLineWidth);
}

auto ESP::drawAimbotTargetCross() const -> void {
	std::optional<Aimbot::AimTarget> const &l_currentTarget = m_aimbot.getCurrentTarget();

	if (l_currentTarget.has_value()) {
		auto l_targetUnitVector = viewAnglesToUnitvector(l_currentTarget->anglesToTarget);
		auto l_pointOnTargetSightLine = gameVars.player_pos + l_targetUnitVector;
		auto l_targetOnScreen = world_to_screen(l_pointOnTargetSightLine);

		if (l_targetOnScreen.has_value()) {
			drawScreenCross(*l_targetOnScreen, 0.06, {87, 237, 14, 255}, false);

//			const float width = 10;
//			SDL_Color color{255, 0, 0, 255};
//			glm::vec3 position{l_currentTarget->aimPoint};
//			position.z -= width / 2;
//
//			drawBox(position,
//			        color,
//			        width,
//			        l_currentTarget->target.m_viewangles.y,
//			        width);
		}
	}
}

auto ESP::drawBulletPrediction() const -> void {
	glm::vec3 l_bulletPredictionUnitVec = viewAnglesToUnitvector(m_aimbot.getBulletPredictionAngles());
	glm::vec3 l_bulletPointWorld = gameVars.player_pos + l_bulletPredictionUnitVec;
	auto l_bulletPointScreen = world_to_screen(l_bulletPointWorld);

	if (l_bulletPointScreen.has_value()) {
		drawScreenCross(*l_bulletPointScreen, 0.04, {237, 14, 14, 255}, true);
	}
}