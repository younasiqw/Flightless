#pragma once

struct GlowObject_t {
	CBaseEntity*	m_pEntity;					// 0
	Vector			m_vGlowColor;				// 4
	float			m_flGlowAlpha;				// 16
	unsigned char   junk[ 16 ];					// 20
	bool			m_bRenderWhenOccluded;		// 36
	bool			m_bRenderWhenUnoccluded;	// 37
	bool			m_bFullBloomRender;			// 38
	unsigned char   junk2[ 13 ];				// 39
};

class CVisuals
{
public:
	bool InThirdPerson = false;
	void Nightmode();
	void Run( );
	void PlayerList();
	bool GetBox(CBaseEntity * entity, int & x, int & y, int & w, int & h);
	void Watermark();
	void bombtimer(CBaseEntity * pEntity);
	void BulletTraceOverlay( CBaseEntity* entity, Color color );
	bool EntityIsInvalid( CBaseEntity* Entity );
	void SpecList();
private:
	bool InGame();
	
	CBaseEntity *BombCarrier;

	//void SpecList();
	void PlayerESP( int index );
	void WorldESP( int index );
	void DrawEntity(CBaseEntity * entity, const char * string, Color color);
	void DrawThrowable(CBaseEntity * throwable, ClientClass * client);
	void DrawOnFloor(CBaseEntity * throwable, ClientClass * client);
	void FovCrosshair();
	void BulletTrace( CBaseEntity* entity, Color color );
	void PlayerBox( float x, float y, float w, float h, Color clr );
	void EspBox(CBaseEntity * pEntityPlayer, Color clr);
	void HealthBar(int x, int y, int w, int h, float health, float armor);
	void Skeleton( CBaseEntity *Entity, Color color );
	void DrawHitbox( matrix3x4a_t* matrix, Vector bbmin, Vector bbmax, int bone, Color color );
	void Hitboxes( CBaseEntity *Entity, Color color );
	void DrawCrosshair( Color color );
};
