#pragma once

#include <atomic>

#include "Utility.hpp"
#include "DetourToCallback.hpp"
#include "Player.hpp"
#include "Pointers/SharedGamePointer.hpp"

struct Player;
//class GUI;

class Aimbot : public Util::NonCopyable, public Util::NonMovable {
public:
	enum class Mode360 {
		TRIGGER,
		AIMBOT
	};

	struct AimTarget {
		Player &target;
		glm::vec3 aimPoint;
		glm::vec3 anglesToTarget;
	};

	//explicit Aimbot(GUI &gui);
	explicit Aimbot();

	~Aimbot();

	auto aim_once() -> void;

	auto deflect_once() -> void;

	auto startAim() -> void;

	auto stopAim() -> void;

	auto startTrigger() -> void;

	auto stopTrigger() -> void;

	auto start360(Mode360 modeBefore, Mode360 modeAfter) -> void;

	auto stop360() -> void;

	// do primary attack if the crosshair points at an enemy
	auto triggerbot_once() const -> void;

	auto getCurrentTarget() const -> std::optional<Aimbot::AimTarget> const&;

	/// next bullet angle relative to visual angles
	auto getBulletPredictionAngles() const -> glm::vec3 const&;

private:
	friend class ESP;
	enum class AIM_TYPE {
		BY_ANGLE,
		BY_DISTANCE
	};

	//GUI &m_gui;

	// TODO: read max players value from memory
	// how long between attack action start and end
	static constexpr int ATTACK_SLEEP_BETWEEN_MS = 20;
	static constexpr int MAX_PLAYERS = 64;
	static constexpr float AIM_MIN_DISTANCE = 10;
	static constexpr glm::vec3 AIM_TARGET_OFFSET_HEAD{ 9.141f, -3.828, 0.078 };

	AIM_TYPE m_aim_type;
	bool m_friendly_fire;
	float m_aim_fov_rad;        // >= 180 results in deactivation of fov selection
	bool m_aim_noRecoil;
	bool m_aim_noVisRecoil;

	// memory fields the aimbot operates on
	SharedGamePointer<glm::vec3> m_playerPos;
	SharedGamePointer<glm::vec3> m_aimAngles;
	SharedGamePointer<glm::vec3> m_visualAngles;
	SharedGamePointer<glm::vec3> m_punchAngles;
	SharedGamePointer<Player::TEAM> m_playerTeam;
	SharedGamePointer<std::array<Player, MAX_PLAYERS>> m_players;
	SharedGamePointer<int> m_crosshair_target_id;
	SharedGamePointer<int> m_doAttack;

	glm::vec3 m_recoilFix_previous;
	std::optional<AimTarget> m_currentTarget;
	glm::vec3 m_bulletPredictionAngles;
	std::atomic_bool m_doAim;
	std::atomic_bool m_doTrigger;

	DetourToCallback m_detour_viewAngles_update;
	DetourToCallback m_detour_viewAnglesVis_update;

	struct Maneuver360 {
		enum State {
			INIT,
			TWIST,
			DONE
		};
		const float m_totalTimeMillis_360 = 350;
		std::atomic<State> m_state = DONE;
		std::atomic<Mode360> m_modeBefore;
		std::atomic<Mode360> m_modeAfter;
		std::chrono::time_point<std::chrono::steady_clock> m_startTime;
		glm::vec3 m_startAngles;
		Player *m_target = nullptr;
	} m_360;

	auto install() -> void;

	auto uninstall() -> void;

	static auto getTargetAimPoint(const Player &target) -> glm::vec3;

	static auto cartesianToPolar(const glm::vec3 &cartesian) -> glm::vec3;

	auto findTarget(AIM_TYPE method) -> void;

	auto isCrosshairOnTarget() const -> std::optional<Player *>;

	auto removeVisRecoil() -> void;

	auto hookViewAnglesUpdate() -> void;

	auto hookViewAnglesVisUpdate() -> void;

	auto is_user_shooting() const -> bool;

    // deflect aim away from target.
    // Result is the world coordinate [targetRadius] away from player.
    auto deflectAimInWorld(const Player &target, float targetRadius) -> std::optional<glm::vec3>;
};
