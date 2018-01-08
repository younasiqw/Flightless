#include "../Cheat.h"
#include <sstream>
#include <fstream>
#include "../xor.h"

typedef float(*RandomFloat_t)(float, float);
typedef void(*RandomSeed_t)(UINT);

void CMiscellaneous::Initialize()
{
	if ( m_bInitialized )
		return;
}

void CMiscellaneous::Run()
{
	Bunnyhop();
}

void CMiscellaneous::Think()
{

}

void CMiscellaneous::Bunnyhop()
{
	/*if ( G::UserCmd->buttons & IN_JUMP && !( G::LocalPlayer->GetFlags() & FL_ONGROUND ) )
	{
		G::UserCmd->buttons &= ~IN_JUMP;
	}*/

	//LEGIT

	static bool bLastJumped = false;
	static bool bShouldFake = false;

	if (!bLastJumped && bShouldFake)
	{
		bShouldFake = false;
		G::UserCmd->buttons |= IN_JUMP;
	}
	else if (G::UserCmd->buttons & IN_JUMP)
	{
		if (G::LocalPlayer->GetFlags() & FL_ONGROUND)
		{
			bLastJumped = true;
			bShouldFake = true;
		}
		else
		{
			G::UserCmd->buttons &= ~IN_JUMP;
			bLastJumped = false;
		}
	}
	else
	{
		bLastJumped = false;
		bShouldFake = false;
	}
}

float NormalizeFloat(float result)
{
	while (result > 180.0f)
		result -= 360.0f;

	while (result < -180.0f)
		result += 360.0f;

	return result;
}

void CMiscellaneous::AngleToVectors(QAngle &angles, Vector *forward, Vector *right, Vector *up)
{
	float angle;
	static float sr, sp, sy, cr, cp, cy, cpi = (M_PI * 2 / 360);

	angle = angles.y * cpi;
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * cpi;
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * cpi;
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp*cy;
		forward->y = cp*sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right->y = (-1 * sr*sp*sy + -1 * cr*cy);
		right->z = -1 * sr*cp;
	}

	if (up)
	{
		up->x = (cr*sp*cy + -sr*-sy);
		up->y = (cr*sp*sy + -sr*cy);
		up->z = cr*cp;
	}
}

float CMiscellaneous::RandomFloat(float fMin, float fMax)
{
	static RandomFloat_t m_RandomFloat;

	if (!m_RandomFloat)
		m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat");

	return m_RandomFloat(fMin, fMax);
}
void CMiscellaneous::RandomSeed(UINT Seed)
{
	static RandomSeed_t m_RandomSeed;

	if (!m_RandomSeed)
		m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed");

	m_RandomSeed(Seed);
}

void CMiscellaneous::AutoStrafe()
{
	if( G::LocalPlayer->GetFlags() & FL_ONGROUND )
		return;

	if (!G::LocalPlayer->GetAlive())
		return;

	if (G::LocalPlayer->GetMoveType() == MOVETYPE_LADDER || G::LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	Vector vVelocity = G::LocalPlayer->GetVelocity();
	float Speed = sqrt(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y);

	float Tickrate = 1.1;

	static float OldY;
	float Difference = NormalizeFloat(G::UserCmd->viewangles.y - OldY);
	if (Vars.Misc.AutoStrafe)
	{
		//G::UserCmd->forwardmove = 450;

		if (Vars.Misc.AutoStrafe == 1) {
			bool backwards = false;
			if (G::UserCmd->mousedx < 0) G::UserCmd->sidemove = backwards ? 650.f : -650.f;
			if (G::UserCmd->mousedx > 0) G::UserCmd->sidemove = backwards ? -650.f : 650.f;
		}
		if (Vars.Misc.AutoStrafe == 2)
		{
			float Value = (8.15 - Tickrate) - (Speed / 340);

			if (Speed > 160 && Speed < 420)
			{
				Value = (4.6 - Tickrate) - (Speed / 340);
			}
			if (Speed > 420)
			{
				Value = (3 - Tickrate) - (Speed / 1000);
			}
			if (Value <= 0.275)
			{
				Value = 0.275;
			}

			if (abs(Difference) < Value)
			{
				static bool Flip = false;
				if (Flip)
				{
					G::UserCmd->viewangles.y -= Value;
					G::UserCmd->sidemove = -450;
					Flip = false;
				}
				else
				{
					G::UserCmd->viewangles.y += Value;
					G::UserCmd->sidemove = 450;
					Flip = true;
				}
			}
			else
			{
				if (Difference > 0.0f)
					G::UserCmd->sidemove = -450;

				if (Difference < 0.0f)
					G::UserCmd->sidemove = 450;
			}
			OldY = G::UserCmd->viewangles.y;
		}
	}
}

void CMiscellaneous::Run_LocationSpam()
{
	static std::string name;
	CBaseEntity* pEntity = NULL;
#define RandomInt(min, max) (rand() % (max - min + 1) + min)

	while (!pEntity)
	{
		pEntity = I::ClientEntList->GetClientEntity(RandomInt(0, I::ClientEntList->GetHighestEntityIndex()));

		if (!pEntity) return;
		if (!pEntity->GetAlive()) return;
		name = pEntity->GetName();
		if (pEntity->GetName() == "GOTV" || pEntity->GetName() == "") return;

		if (Vars.Misc.LocSpamEnemies && pEntity->GetTeam() == G::LocalPlayer->GetTeam())
			return;

		static float nextTime = 0.f;
		float flServerTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;

		if (nextTime > flServerTime)
			return;

		nextTime = flServerTime + Vars.Misc.ChatSpamDelay;

		if (!Vars.Misc.LocSpam)
			return;

		char szCmd[256];

		if (pEntity->GetHealth() <= 0) return;

		if (Vars.Misc.LocSpamChat)
			sprintf(szCmd, "say_team  \"%s is at %s with %d health!\"", name.c_str(), pEntity->GetLastPlace().c_str(), pEntity->GetHealth());
		else
			sprintf(szCmd, "say \"%s is at %s with %d health!\"", name.c_str(), pEntity->GetLastPlace().c_str(), pEntity->GetHealth());

		I::Engine->ClientCmd_Unrestricted(szCmd);
		//return;
	}
}

void CMiscellaneous::NameSteal()
{
#define RandomInt(min, max) (rand() % (max - min + 1) + min)
	if (!Vars.Misc.NameSteal)
		return;

	static float nextTime = 0.f;
	float flServerTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;

	if (nextTime > flServerTime)
		return;

	nextTime = flServerTime + Vars.Misc.NameStealDelay;


	static ConVar* name = I::Cvar->FindVar("name");

	char* szName = NULL;

	while (!szName)
	{
		player_info_t pInfo;
		I::Engine->GetPlayerInfo(RandomInt(0, I::ClientEntList->GetHighestEntityIndex()), &pInfo);

		if (strcmp(pInfo.name, "") != 0 && !(strcmp(pInfo.name, "GOTV") == 0)) szName = pInfo.name;
	}

	char nameChar[64];
	sprintf(nameChar, "%s ", szName);
	name->SetValue(nameChar);
}

void  CMiscellaneous::SwastikaSpam()
{
	/*static float nextTime = 0.f;
	float flServerTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;

	if (nextTime > flServerTime)
		return;

	nextTime = flServerTime + Vars.Misc.ChatSpamDelay;

	int chockedtickz = 1;

	if (I::Engine->IsConnected())
	{
		if (Vars.Misc.SwastikaSpam)
		{
			if (chockedtickz = 1)
			{
				I::Engine->ClientCmd_Unrestricted(charenc("say ........................."));
				//chockedtickz = 2;
			}
			if (chockedtickz = 2)
			{
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHH....HHHHHHHHHHHHHH."));
				//chockedtickz = 3;
			}
			if (chockedtickz = 3) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHH....HHHHHHHHHHHHHH."));
				//chockedtickz = 4;
			}
			if (chockedtickz = 4) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHH....HHHHH.........."));
				//chockedtickz = 5;
			}
			if (chockedtickz = 5) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHH....HHHHH.........."));
				//chockedtickz = 6;
			}
			if (chockedtickz = 6) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHHHHHHHHHHHHHHHHHHHH."));
				//chockedtickz = 7;
			}
			if (chockedtickz = 7) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHHHHHHHHHHHHHHHHHHHH."));
				//chockedtickz = 8;
			}
			if (chockedtickz = 8) {
				I::Engine->ClientCmd_Unrestricted(charenc("say ..........HHHHH....HHHHH."));
				//chockedtickz = 9;
			}
			if (chockedtickz = 9) {
				I::Engine->ClientCmd_Unrestricted(charenc("say ..........HHHHH....HHHHH."));
				//chockedtickz = 10;
			}
			if (chockedtickz = 10) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHHHHHHHHHHH....HHHHH."));
				//chockedtickz = 11;
			}
			if (chockedtickz = 11) {
				I::Engine->ClientCmd_Unrestricted(charenc("say .HHHHHHHHHHHHHH....HHHHH."));
				//chockedtickz = 12;
			}
			if (chockedtickz = 12)
			{
				I::Engine->ClientCmd_Unrestricted(charenc("say ........................."));
				chockedtickz = 1;
			}
			chockedtickz += 1;
		}
	}*/
}

void CMiscellaneous::Chatspam()
{
	static float nextTime = 0.f;
	float flServerTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;

	if( nextTime > flServerTime )
		return;

	nextTime = flServerTime + Vars.Misc.ChatSpamDelay;

	if (!Vars.Misc.ChatSpam)
		return;

	char str[512];
	sprintf( str, "say %s", Vars.Misc.ChatSpamMode);

	if (I::Engine->IsConnected())
		I::Engine->ClientCmd_Unrestricted( str );
}

void CMiscellaneous::ChangeName( const char* name )
{
	ConVar* nameConvar = I::Cvar->FindVar( charenc( "name" ) );
	*reinterpret_cast< int* >(reinterpret_cast< DWORD >(&nameConvar->fnChangeCallback) + 0xC) = NULL;
	nameConvar->SetValue( name );
}

void CMiscellaneous::CircleStrafer()
{
	if (GetAsyncKeyState(0x86)) {
	}
}

void CMiscellaneous::AirStuck()
{
	if( Vars.Ragebot.Enabled && G::UserCmd->buttons & IN_ATTACK )
		return;

	bool airset = false;

	if (GetKeyState(Vars.Misc.AirStuckKey) & 1)
		airset = !airset;
	
	if (airset)
		G::UserCmd->tick_count = INT_MAX;
}

void CMiscellaneous::GameResources()
{
	int Music = 20024;
	static DWORD GameResources = (U::FindPattern("client.dll", "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7") + 2);

	DWORD ptrResource = **(DWORD**)GameResources;

	DWORD m_nMusicID = (DWORD)ptrResource + offsets.m_nMusicID + (G::LocalPlayer->GetIndex() * 4);

	//DWORD m_nActiveCoinRank = (DWORD)ptrResource + nActiveCoinRank + (G::LocalPlayer->GetIndex() * 4);

	if (Music > 0) *(PINT)((DWORD)m_nMusicID) = Music;
}

void CMiscellaneous::Panic()
{

	I::Engine->ClientCmd_Unrestricted( charenc( "cl_mouseenable 1" ) );
	I::Engine->ClientCmd_Unrestricted( charenc( "crosshair 1" ) );

	H::ModelRender->UnHook();
	H::VPanel->UnHook();
	H::ClientMode->UnHook();
	H::Client->UnHook();
	H::Surface->UnHook();
	//H::StudioRender->UnHook();
	H::GameEvent->UnHook();
	H::D3D9->UnHook();
	SetWindowLongPtrA( G::Window, GWL_WNDPROC, ( LONG_PTR )Hooks::oldWindowProc );
}