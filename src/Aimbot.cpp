#include <iostream>
#include <iomanip>    //setw
#include <string>
#include <vector>
#include <thread>     //this_thread, sleep_for
#include <cmath>        // std::acos

#include <glm/geometric.hpp> // glm::distance

#include "Aimbot.hpp"
#include "MemoryUtils.hpp"
#include "Player.hpp"

#define DEFAULT_LOG_CHANNEL Log::Channel::MESSAGE_BOX
#include "Log.hpp"
#include "Visuals/GUI.hpp"
#include "Utility.hpp"
#include "Pointers/GamePointerFactory.hpp"
#include "Pointers/Signatures.hpp"

using namespace Util;

//Aimbot::Aimbot(GUI& gui)
Aimbot::Aimbot()
		//: m_gui(gui)
		: m_aim_type(AIM_TYPE::BY_ANGLE)
		, m_friendly_fire(false)
		, m_aim_fov_rad(toRadians(15))
		, m_aim_noRecoil(true)
		, m_aim_noVisRecoil(true)
		, m_playerPos(GamePointerFactory::get(GamePointerDef::playerPos()))
		, m_aimAngles(GamePointerFactory::get(GamePointerDef::aimAngles()))
		, m_visualAngles(GamePointerFactory::get(GamePointerDef::aimAnglesVisual()))
		, m_punchAngles(GamePointerFactory::get(GamePointerDef::punchAngles()))
		, m_playerTeam(GamePointerFactory::get(GamePointerDef::playerTeam()))
		, m_players(GamePointerFactory::get(GamePointerDef::players()))
		, m_crosshair_target_id(GamePointerFactory::get(GamePointerDef::targetId()))
		, m_doAttack(GamePointerFactory::get(GamePointerDef::doAttack()))
		, m_recoilFix_previous()
		, m_currentTarget()
		, m_doAim(false)
		, m_doTrigger(false)
		, m_detour_viewAngles_update()
		, m_detour_viewAnglesVis_update()
		, m_360() {
	/*
	m_gui.registerCheckbox({m_friendly_fire, "Aim Friendly Fire"});
	m_gui.registerAngleRadSlider({0, 180, m_aim_fov_rad, "Aim FOV"});
	m_gui.registerCheckbox({m_aim_noRecoil, "Aim NoRecoil"});
	m_gui.registerCheckbox({m_aim_noVisRecoil, "Aim NoVisRecoil"});

	std::vector<std::string> aimTypeLabels{"Angle", "Distance"};
	GuiElements::ComboBox aimTypeComboBox(std::move(aimTypeLabels), "Aimtype",
	                                      [this](size_t selectedIndex) {
		                                   if (selectedIndex == 0) {
			                                   m_aim_type = AIM_TYPE::BY_ANGLE;
		                                   } else {
			                                   m_aim_type = AIM_TYPE::BY_DISTANCE;
		                                   }
	                                   });
	m_gui.registerComboBox(std::move(aimTypeComboBox));
	*/

	install();
}

Aimbot::~Aimbot() {
	stopAim();
	uninstall();
}

auto normalizeHeading(glm::vec3 &angles) -> void {
	float heading_360_deg_range_corrected = std::fmod(angles.y + 180, 360);
	float heading_180_deg_range_corrected = heading_360_deg_range_corrected - 180;
	angles.y = heading_180_deg_range_corrected;
}

auto Aimbot::aim_once() -> void {
	findTarget(m_aim_type);

	if (m_360.m_state == Maneuver360::INIT) {
		bool l_target_360_found = false;
		// initialize 360 start fields
		if (m_360.m_modeBefore == Mode360::TRIGGER) {
			// find target using crosshair detection
			std::optional<Player *> l_target = isCrosshairOnTarget();
			if (l_target.has_value()) {
				m_360.m_target = l_target.value();
				l_target_360_found = true;
			}
		} else {
			// find target using aimbot
			findTarget(m_aim_type);
			if (m_currentTarget.has_value()) {
				m_360.m_target = &m_currentTarget->target;
				l_target_360_found = true;
			}
		}
		if (l_target_360_found) {
			m_360.m_startTime = std::chrono::steady_clock::now();
			m_360.m_startAngles = *m_aimAngles;
			// transition to twist state
			m_360.m_state = Maneuver360::TWIST;
		}
	}
	if (m_360.m_state == Maneuver360::TWIST) {
		// perform 360 maneuver
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float, std::milli> deltaTime = currentTime - m_360.m_startTime;
		float progress = deltaTime.count() / m_360.m_totalTimeMillis_360;

		if (progress <= 1.0) {
			const glm::vec3 targetAimPoint = getTargetAimPoint(*m_360.m_target);
			const glm::vec3 targetVec = targetAimPoint - *m_playerPos;
			glm::vec3 targetAngles = cartesianToPolar(targetVec);
			glm::vec3 aimDiff = targetAngles - m_360.m_startAngles;
			normalizeHeading(aimDiff);

			float shortDiffPitch = aimDiff.x;
			float greatDiffHeading = 360.0f - std::abs(aimDiff.y);
			if (aimDiff.y > 0) {
				greatDiffHeading *= -1;
			}
			glm::vec3 totalAngDiff360{shortDiffPitch, greatDiffHeading, aimDiff.z};
			glm::vec3 currentAngDiff360 = totalAngDiff360 * progress;

			glm::vec3 currentAng = m_360.m_startAngles + currentAngDiff360;
			normalizeHeading(currentAng);
			*m_aimAngles = currentAng;
		} else {
			m_360.m_state = Maneuver360::DONE;
			if (m_360.m_modeAfter == Mode360::TRIGGER) {
				// transition to triggerbot
				m_doAim = false;
				m_doTrigger = true;
			} else {
				// transition to aimbot state
				m_doAim = true;
				m_doTrigger = false;
			}
		}
	}
	if (m_360.m_state == Maneuver360::DONE) {
		bool l_aim_foundTarget = false;
		if (m_doAim) {
			// normal aimbot
			findTarget(m_aim_type);

			// check if player found
			if (m_currentTarget.has_value()) {
				// aim at target
				*m_aimAngles = m_currentTarget->anglesToTarget;
				l_aim_foundTarget = true;
			}
		}
		if (m_doTrigger || (m_doAim && l_aim_foundTarget)) {
			// triggerbot / autoshoot
			if (isCrosshairOnTarget()) {
				// TODO: implement autopistol
//				if (*m_doAttack == 4) {
					*m_doAttack = 5;
//				} else {
//					*m_doAttack = 4;
//				}
			} else if (!is_user_shooting()) {
				*m_doAttack = 4;
			}
		} else {
			// TODO do wait after next shoot depending on timepoint of last one (autopistol)
			if (!is_user_shooting()) {
				*m_doAttack = 4;
			}
		}
	}
}

auto Aimbot::deflect_once() -> void {
  const auto lastFOV = m_aim_fov_rad;
  m_aim_fov_rad = Util::toRadians(180);
  findTarget(AIM_TYPE::BY_ANGLE);
  m_aim_fov_rad = lastFOV;

  // check if player found
  if (m_currentTarget.has_value()) {
    auto deflectPointWorld = deflectAimInWorld(m_currentTarget->target, 45);
    if (deflectPointWorld.has_value()) {
      // the aim should be deflected, the aim point is too close to the target center
      glm::vec3 vecEyeToDeflectPoint = *deflectPointWorld - *m_playerPos;
      glm::vec3 angles = cartesianToPolar(vecEyeToDeflectPoint);
      *m_aimAngles = angles;
    }
  }
}

auto Aimbot::startAim() -> void {
	m_doAim = true;
}

auto Aimbot::stopAim() -> void {
	m_doAim = false;
}

auto Aimbot::startTrigger() -> void {
	m_doTrigger = true;
}

auto Aimbot::stopTrigger() -> void {
	m_doTrigger = false;
}

auto Aimbot::start360(Mode360 modeBefore, Mode360 modeAfter) -> void {
	// FIXME? set state after modes - #race condition
	m_360.m_state = Maneuver360::INIT;
	m_360.m_modeBefore = modeBefore;
	m_360.m_modeAfter = modeAfter;
}

auto Aimbot::stop360() -> void {
	m_360.m_state = Maneuver360::DONE;
}

auto Aimbot::triggerbot_once() const -> void {
	if (isCrosshairOnTarget()) {
		// shoot once

		// +attack
		// TODO don't shoot while we have to reload
		*m_doAttack = 5;

		std::this_thread::sleep_for(std::chrono::milliseconds(ATTACK_SLEEP_BETWEEN_MS));

		// -attack
		if (!is_user_shooting()) {
			*m_doAttack = 4;
		}
	}
}

auto Aimbot::getCurrentTarget() const -> std::optional<Aimbot::AimTarget> const& {
	return m_currentTarget;
}

auto Aimbot::getBulletPredictionAngles() const-> glm::vec3 const& {
	return m_bulletPredictionAngles;
}

auto Aimbot::install() -> void {
	bool l_ang_detour_success = m_detour_viewAngles_update.install(
		Signatures::aimAngles_x_op_read,
		[this] { hookViewAnglesUpdate(); },
		DetourToCallback::CODE_AFTER_DETOUR
	);
	if (!l_ang_detour_success) {
		Log::log("Aimbot failed to detour update_ang");
	}

	bool l_vis_ang_detour_success = m_detour_viewAnglesVis_update.install(
		Signatures::aimAnglesVisual_update,
		[this] { hookViewAnglesVisUpdate(); },
		DetourToCallback::CODE_BEFORE_DETOUR
	);
	if (!l_vis_ang_detour_success) {
		Log::log("Aimbot failed to detour update_vis_ang");
	}
}

auto Aimbot::uninstall() -> void {
	if (!m_detour_viewAngles_update.remove()) {
		Log::log("Aimbot failed to un-detour update_ang");
	}

	if (!m_detour_viewAnglesVis_update.remove()) {
		Log::log("Aimbot failed to un-detour update_vis_ang");
	}

	// TODO: necessary?
	// stop shooting if the aimbot was disabled while shooting
	if (!is_user_shooting()) {
		*m_doAttack = 4;
	}
}

auto Aimbot::getTargetAimPoint(const Player &target) -> glm::vec3 {
	// include the target offset (aim a little at the belly)
	// rotate static target offset with targets heading angle:
	// source: https://developer.valvesoftware.com/wiki/QAngle
	auto l_aim_target_offset_rotated = rotateAroundZ(AIM_TARGET_OFFSET_HEAD, target.m_viewangles.y);

	return target.m_pos + l_aim_target_offset_rotated;
}

auto Aimbot::cartesianToPolar(const glm::vec3 &cartesian) -> glm::vec3 {
	const float l_yawNewDeg = toDegrees(std::atan2(cartesian.y, cartesian.x));
	const float l_pitchNewDeg = toDegrees(std::acos(cartesian.z / cartesian.length()));

	// TODO: Clarify angle conversion
	return {l_pitchNewDeg - 90, l_yawNewDeg, 0.0};
}

auto Aimbot::findTarget(AIM_TYPE method) -> void {
	glm::vec3 l_targetAimPoint;
	glm::vec3 l_targetAimVec;
	Player *l_target = nullptr;
	float l_nearestDistance = -1;
	float l_nearestAngle = -1;
	for (Player &curTarget : (*m_players)) {
		// ensure current target is valid & alive
		if (!curTarget.is_valid() ||
		    curTarget.m_health <= 0) {
			continue;
		}

		// target mustn't be a spectator
		if (curTarget.m_team == Player::TEAM::SPECT
		    // if friendly fire is off, target mustn't be of same team
		    || (!m_friendly_fire && curTarget.m_team == *m_playerTeam)) {
			continue;
		}

		// the target must be inside the FOV
		// (the angle between target vector and crosshair vector is within the FOV range)
		const glm::vec3 l_targetAimPointTmp = getTargetAimPoint(curTarget);
		const glm::vec3 l_targetAimVecTmp = l_targetAimPointTmp - *m_playerPos;
		float l_angleToTarget = degreesBetweenVectors(l_targetAimVecTmp,
		                                              Util::viewAnglesToUnitvector(*m_visualAngles));
		if (l_angleToTarget > toDegrees(m_aim_fov_rad)) {
			continue;
		}

		// calculate distance to target
		float l_targetDistance = glm::distance(*m_playerPos, curTarget.m_pos);
		// prevent aiming at yourself
		if (l_targetDistance <= AIM_MIN_DISTANCE) {
			continue;
		}

		switch (method) {
			case AIM_TYPE::BY_ANGLE: {
				// aim at target closest to crosshair
				if (l_target == nullptr || l_angleToTarget < l_nearestAngle) {
					// choose current target as the angle to it is nearer than former target
					l_target = &curTarget;
					l_targetAimPoint = l_targetAimPointTmp;
					l_targetAimVec = l_targetAimVecTmp;
					l_nearestAngle = l_angleToTarget;
				}
				break;
			}
			case AIM_TYPE::BY_DISTANCE: {
				// aim at target with nearest distance
				if (l_target == nullptr || l_targetDistance < l_nearestDistance) {
					// choose current target as it is nearer than former target
					l_target = &curTarget;
					l_targetAimPoint = l_targetAimPointTmp;
					l_targetAimVec = l_targetAimVecTmp;
					l_nearestDistance = l_targetDistance;
				}
				break;
			}
		}
	}
	if (l_target != nullptr) {
		m_currentTarget.emplace(AimTarget{*l_target, l_targetAimPoint,
		                 cartesianToPolar(l_targetAimVec)});
	} else {
		m_currentTarget = std::nullopt;
	}
}

auto Aimbot::isCrosshairOnTarget() const -> std::optional<Player *> {
	std::optional<Player *> valid_target;

	// check if we're aiming at a player (not a barrel, for example)
	auto l_crossIdx = *m_crosshair_target_id;
	if (l_crossIdx != 0
	    // TODO: check if the 64th Player's id would be == 64 or <64
	    && l_crossIdx < MAX_PLAYERS) {

		// ensure the crosshair points at an enemy
		size_t l_player_array_idx = l_crossIdx - 1; // cross_idx'es are always 1 higher than their player array index
		Player *l_target = &(*m_players)[l_player_array_idx];
		if (m_friendly_fire ||
		    l_target->m_team != *m_playerTeam) {
			valid_target = l_target;
		}
	}

	return valid_target;
}

auto Aimbot::removeVisRecoil() -> void {
	if (m_aim_noRecoil) {
		// fix visual angles so that the anti-punch/recoil movement isn't visible
		// while the recoil fix substracts twice the punch angles,
		// the visuals need only 1 times the punch angles for correction
		*m_visualAngles -= m_recoilFix_previous / 2.0f;
	} else {
		*m_visualAngles += m_recoilFix_previous / 2.0f;
	}
}

auto Aimbot::hookViewAnglesUpdate() -> void {
	// undo previous recoil compensation
	if (m_aim_noRecoil) {
		*m_aimAngles -= m_recoilFix_previous;
	}

        deflect_once();
	//aim_once();

	// calculate (visual and effective) recoil compensation
	auto l_recoil_fix_new = *m_punchAngles;
	// exclude roll axis compensation, as it doesn't affect recoil
	l_recoil_fix_new.z = 0;
	l_recoil_fix_new *= -2;
	m_recoilFix_previous = l_recoil_fix_new;

	// compensate recoil
	if (m_aim_noRecoil) {
		*m_aimAngles += l_recoil_fix_new;
	}
}

auto Aimbot::hookViewAnglesVisUpdate() -> void {
	if (m_aim_noVisRecoil) {
		removeVisRecoil();
	}

	m_bulletPredictionAngles = *m_aimAngles - m_recoilFix_previous;
}

auto Aimbot::is_user_shooting() const -> bool {
	int *l_attack_user_0 = m_doAttack.pointer() - 2;
	int *l_attack_user_1 = m_doAttack.pointer() - 1;
	return *l_attack_user_0 != 0 || *l_attack_user_1 != 0;
}

auto Aimbot::deflectAimInWorld(const Player &target, float targetRadius) -> std::optional<glm::vec3> {
  glm::vec3 pTargetCenter = target.m_pos + glm::vec3{0.0f, 0.0f, -30.0f};
  glm::vec3 vEyeToTarCen = pTargetCenter - *m_playerPos;
  glm::vec3 vUnitAimVec = Util::viewAnglesToUnitvector(*m_aimAngles);

  // intersect eye vec with target "disk"
  // (intersect line with plane) source: https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
  glm::vec3 &p0 = pTargetCenter;
  glm::vec3 &l0 = *m_playerPos;
  glm::vec3 n = vEyeToTarCen;
  n.z = 0.0f;
  glm::vec3 &l = vUnitAimVec;

  float d = glm::dot((p0 - l0), n) / glm::dot(l, n);

  // pA is the point on the target circle disk
  glm::vec3 pA = *m_playerPos + vUnitAimVec * d;
  glm::vec3 pCA = pA - pTargetCenter;
  float distancePaToTargetCenter = pCA.length();
  if (distancePaToTargetCenter < targetRadius) {
    // perform deflect
    glm::vec3 pCAStretched = glm::normalize(pCA) * targetRadius;
    glm::vec3 pDeflectedOntoPerimeter = pTargetCenter + pCAStretched;
    return pDeflectedOntoPerimeter;
  } else {
    return std::nullopt;
  }
}
