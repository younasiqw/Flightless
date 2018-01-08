#pragma once

enum AA_Pitch
{
	NOAAP,
	STATIC_UP,
	STATIC_DOWN,
	DANCE,
	FRONT,
	EMOTION,
	STATIC_DOWN_FAKE,
	SKEETDOWN,
};

enum AA_Yaw
{
	NOAAY,
	SPIN,
	ADD90FAKE,
	SIDE,
	SIDE2,
	BACKWARDS,
	LEFT,
	RIGHT,
	ZERO,
	FAKESPIN,
	LOWERBODY,
	LAGSPIN,
	MEMESPIN,
	TESTING
};

enum AAZ_Pitch
{
	NOAAZ,
	SPIN_ZSLOW,
	JITTERZ,
	SIDEWAYSZ,
	ANGEL_ZSPIN
};

class CAntiAim
{
public:
	void Run();
	bool HasViableEnemy();
private:
	bool GetBestHeadAngle(QAngle& angle);
};

extern CAntiAim* AntiAim;
