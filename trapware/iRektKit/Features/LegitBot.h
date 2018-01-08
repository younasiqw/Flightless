#pragma once

class CLegitBot
{
public:
	void Run();


private:
	int FindTarget(CBaseEntity * pLocalPlayer, CUserCmd * pCmd);
	void Triggerbot();
	//int m_fbestfov;
	//int m_ibesttarget;
};

