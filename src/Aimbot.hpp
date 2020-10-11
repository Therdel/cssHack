#pragma once

#include <atomic>

#include "Utility.hpp"
#include "DetourToCallback.hpp"
#include "Vec3f.hpp"
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
		Vec3f aimPoint;
		Vec3f anglesToTarget;
	};

	//explicit Aimbot(GUI &gui);
	explicit Aimbot();

	~Aimbot();

	void aim_once();

	void deflect_once();

	void startAim();

	void stopAim();

	void startTrigger();

	void stopTrigger();

	void start360(Mode360 modeBefore, Mode360 modeAfter);

	void stop360();

	// do primary attack if the crosshair points at an enemy
	void triggerbot_once() const;

	std::optional<Aimbot::AimTarget> const &getCurrentTarget() const;

	/// next bullet angle relative to visual angles
	Vec3f const &getBulletPredictionAngles() const;

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
	static constexpr Vec3f AIM_TARGET_OFFSET_HEAD{ 9.141f, -3.828, 0.078 };

	AIM_TYPE m_aim_type;
	bool m_friendly_fire;
	float m_aim_fov_rad;        // >= 180 results in deactivation of fov selection
	bool m_aim_noRecoil;
	bool m_aim_noVisRecoil;

	// memory fields the aimbot operates on
	SharedGamePointer<Vec3f> m_playerPos;
	SharedGamePointer<Vec3f> m_aimAngles;
	SharedGamePointer<Vec3f> m_visualAngles;
	SharedGamePointer<Vec3f> m_punchAngles;
	SharedGamePointer<Player::TEAM> m_playerTeam;
	SharedGamePointer<std::array<Player, MAX_PLAYERS>> m_players;
	SharedGamePointer<int> m_crosshair_target_id;
	SharedGamePointer<int> m_doAttack;

	Vec3f m_recoilFix_previous;
	std::optional<AimTarget> m_currentTarget;
	Vec3f m_bulletPredictionAngles;
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
		Vec3f m_startAngles;
		Player *m_target = nullptr;
	} m_360;

	void install();

	void uninstall();

	static Vec3f getTargetAimPoint(const Player &target);

	static Vec3f cartesianToPolar(const Vec3f &cartesian);

	void findTarget(AIM_TYPE method);

	std::optional<Player *> isCrosshairOnTarget() const;

	void removeVisRecoil();

	void hookViewAnglesUpdate();

	void hookViewAnglesVisUpdate();

	bool is_user_shooting() const;

        // deflect aim away from target.
        // Result is the world coordinate [targetRadius] away from player.
        auto deflectAimInWorld(const Player &target, float targetRadius) -> std::optional<Vec3f>;
};
