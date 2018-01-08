#pragma once
#include "../SDK/Interfaces/IVModelInfo.h"

struct Hitbox
{
	Hitbox( void )
	{
		hitbox = -1;
	}

	Hitbox( int newHitBox )
	{
		hitbox = newHitBox;
	}

	int  hitbox;
	Vector points[ 9 ];
};

enum
{
	FL_HIGH = ( 1 << 0 ),
	FL_LOW = ( 1 << 1 ),
	FL_SPECIAL = ( 1 << 2 )
};

struct BestPoint
{
	BestPoint( void )
	{
		hitbox = -1;
		point = Vector( 0, 0, 0 );
		index = -1;
		dmg = -1;
		flags = 0;
	}

	BestPoint( int newHitBox )
	{
		hitbox = newHitBox;
		point = Vector( 0, 0, 0 );
		index = -1;
		dmg = -1;
		flags = 0;
	}

	Vector point;
	int  index;
	int  dmg;
	int  flags;
	int  hitbox;
};

namespace CPlayerList
{
	extern std::vector<int> friends;
	extern std::vector<int> Players;
	//extern std::vector<int> FixPitch;
}

class CRageBot
{
public:
	bool Run();
	bool Autowall( CBaseEntity* other, const Vector& startpos, const Vector& endpos, float* damage );
	bool AimStep( QAngle& angle );
private:
	void FindTarget();
	void GoToTarget();
	void DropTarget();
	void GetBestBone(CBaseEntity* entity, float& best_damage, Bone& best_bone);
	bool GhettoAutowall( CBaseEntity* );
	
	
	int BestAimPointHitbox(CBaseEntity * target);

	bool EntityIsValid( int i );
	bool IsEnemyUsingAntiAim( CBaseEntity* );
	bool GetHitbox( CBaseEntity* target, Hitbox* hitbox );
	bool GetHitbox(CBaseEntity * target, Hitbox * hitbox, Vector vEnd);
	bool GetBestPoint( CBaseEntity* target, Hitbox* hitbox, BestPoint* point );
	bool BestAimPointAll( CBaseEntity* target, Vector& hitbox );

	float oldForward;
	float oldSideMove;
};