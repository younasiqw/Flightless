#pragma once

class CMiscellaneous
{
public:
	void GameResources();
	void Initialize();
	void Run();
	void Think();
	//const char* ReadInsult();
	void Bunnyhop();
	//void AngleToVectors(const Vector & angles, Vector * forward, Vector * right, Vector * up);
	void AngleToVectors(QAngle & angles, Vector * forward, Vector * right, Vector * up);
	float RandomFloat(float fMin, float fMax);
	void RandomSeed(UINT Seed);
	//Vector SpreadFactor(CUserCmd * pCmd, CBaseCombatWeapon * pWeapon);
	void AutoStrafe();
	void Run_LocationSpam();
	void NameSteal();
	void SwastikaSpam();
	void ChangeName( const char* name );
	void ChangeCName();
	void Chatspam();
	void CircleStrafer();
	//void ReadChatspam( const char* fileName );
	void AirStuck();
	void Panic();
private:
	

private:
	bool m_bInitialized = false;
	std::vector<std::string> m_spamlines;
	std::vector<std::string> m_insults;

};
