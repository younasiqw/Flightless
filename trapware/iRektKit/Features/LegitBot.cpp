#pragma once
#include "../Cheat.h"
CLegitBot* LegitBot;
#define C_BaseEntity CBaseEntity
float curAimTime;
float deltaTime;

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	delta.Normalized();
	return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}

int ClosestBone(C_BaseEntity *Entity, CUserCmd* cmd)
{
	if (Vars.Legitbot.Weapon[Vars.wpn].Hitbox >= 9)
	{
		float BestDist = 360.f;
		int aimbone;

		matrix3x4a_t matrix[MAXSTUDIOBONES];

		if (!Entity->SetupBones(matrix, 128, BONE_USED_BY_HITBOX, I::Engine->GetLastTimeStamp()))
			return -1;

		studiohdr_t* pStudioModel = I::ModelInfo->GetStudioModel(Entity->GetModel());
		if (!pStudioModel)
			return -1;

		mstudiohitboxset_t* set = pStudioModel->pHitboxSet(Entity->GetHitboxSet());
		if (!set)
			return -1;

		for (int i = 0; i < set->numhitboxes; i++)
		{
			if (i == HITBOX_RIGHT_THIGH || i == HITBOX_LEFT_THIGH || i == HITBOX_RIGHT_CALF
				|| i == HITBOX_LEFT_CALF || i == HITBOX_RIGHT_FOOT || i == HITBOX_LEFT_FOOT
				|| i == HITBOX_RIGHT_HAND || i == HITBOX_LEFT_HAND || i == HITBOX_RIGHT_UPPER_ARM
				|| i == HITBOX_RIGHT_FOREARM || i == HITBOX_LEFT_UPPER_ARM || i == HITBOX_LEFT_FOREARM)
				continue;

			mstudiobbox_t* hitbox = set->pHitbox(i);

			if (!hitbox)
				continue;

			auto thisdist = GetFov(cmd->viewangles, M::CalcAngle(G::LocalPlayer->GetEyePosition(), Vector(matrix[hitbox->bone][0][3], matrix[hitbox->bone][1][3], matrix[hitbox->bone][2][3])));

			if (BestDist > thisdist)
			{
				BestDist = thisdist;
				aimbone = hitbox->bone;
				continue;
			}
		}
		return aimbone;
	}
	else
		return Vars.Legitbot.Weapon[Vars.wpn].Hitbox;
}

int CLegitBot::FindTarget(CBaseEntity* pLocalPlayer, CUserCmd* pCmd)
{
	float flMax = 8192.f;
	int iBestTarget = -1;
	curAimTime = 0.f;

	if (pLocalPlayer->GetAlive() && !pLocalPlayer->GetDormant()) {
		for (int i = 0; i < I::ClientEntList->GetHighestEntityIndex(); i++)
		{
			CBaseEntity* pEntityPlayer = I::ClientEntList->GetClientEntity(i);
			if (!pEntityPlayer)
				continue;
			if (pEntityPlayer == pLocalPlayer)
				continue;
			if (!pEntityPlayer->GetAlive())
				continue;
			if (pEntityPlayer->GetDormant())
				continue;
			if (pEntityPlayer->GetTeam() == pLocalPlayer->GetTeam() && !Vars.Legitbot.Aimbot.FriendlyFire)
				continue;
			if (!pEntityPlayer->IsVisible(ClosestBone(pEntityPlayer, pCmd)))
				continue;
			float flFoV = GetFov(pCmd->viewangles, M::CalcAngle(pLocalPlayer->GetEyePosition(), pEntityPlayer->GetBonePosition(ClosestBone(pEntityPlayer, pCmd))));
			if (flFoV < flMax) {
				flMax = flFoV;
				iBestTarget = i;
			}
		}
	}
	return iBestTarget;
}

bool psile = false;

void CLegitBot::Run()
{
	static int CustomDelay = 0;
	if ((G::PressedKeys[Vars.Legitbot.Triggerbot.Key] || Vars.Legitbot.Triggerbot.AutoFire) && Vars.Legitbot.Triggerbot.Enabled)
	{
		if (Vars.Legitbot.Triggerbot.Delay > 1)
		{
			if (GetTickCount() > CustomDelay)
			{
				Triggerbot();
				CustomDelay = GetTickCount() + Vars.Legitbot.Triggerbot.Delay;
			}
		}
		else
			Triggerbot();
	}

	if (!Vars.Legitbot.Aimbot.Enabled)
		return;

	CBaseEntity* pLocalPlayer = G::LocalPlayer;

	if (!pLocalPlayer)
		return;

	if (!pLocalPlayer->GetAlive())
		return;

	if (!G::LocalPlayer->GetWeapon()->IsGun())
		return;

	if (!((Vars.Legitbot.Aimbot.AlwaysOn) || (Vars.Legitbot.Aimbot.OnKey && G::PressedKeys[Vars.Legitbot.Aimbot.Key])))
		return;

	int iTarget = FindTarget(pLocalPlayer, G::UserCmd);

	if (iTarget == -1)
		return;

	CBaseEntity* pEntityPlayer = I::ClientEntList->GetClientEntity(iTarget);

	Vector vecBone = pEntityPlayer->GetBonePosition(ClosestBone(pEntityPlayer, G::UserCmd));

	if (vecBone.IsZero())
		return;

	if (Vars.Legitbot.Aimbot.SmokeCheck && U::LineToSmoke(G::LocalPlayer->GetEyePosition(), vecBone, true))
		return;

	QAngle qAim = M::CalcAngle(pLocalPlayer->GetEyePosition(), vecBone);

	if (!pEntityPlayer->IsVisible(ClosestBone(pEntityPlayer, G::UserCmd)))
		return;

	static bool psile = false;

	psile = (Vars.Legitbot.Weapon[Vars.wpn].pSilent && (G::LocalPlayer->GetShotsFired() < 1)) ? true : false;

	float FOV = psile ? Vars.Legitbot.Weapon[Vars.wpn].PFOV : Vars.Legitbot.Weapon[Vars.wpn].FOV;

	if (GetFov(G::UserCmd->viewangles, qAim) > FOV)
		return;

	int flRecoilX = Vars.Legitbot.Weapon[Vars.wpn].RCSAmountX;
	int flRecoilY = Vars.Legitbot.Weapon[Vars.wpn].RCSAmountY;

	if (Vars.Menu.Opened)
		return;

	if ((flRecoilX > 1 || flRecoilY > 1) && G::LocalPlayer->GetShotsFired() > 2) {
		if (pLocalPlayer->GetPunch().Length() > 0 && pLocalPlayer->GetPunch().Length() < 150)
		{
			if (flRecoilX > 1)
				qAim.x -= pLocalPlayer->GetPunch().x * (flRecoilX / 50.f);

			if (flRecoilY > 1)
				qAim.y -= pLocalPlayer->GetPunch().y * (flRecoilY / 50.f);
		}
	}

	G::SendPacket = psile ? false : true;

	float flSmooth = psile ? 0.f : Vars.Legitbot.Weapon[Vars.wpn].Speed;

	if (flSmooth >= 1.f)
	{
		QAngle delta = G::UserCmd->viewangles - qAim;
		if (!delta.IsZero())
		{
			delta.Normalized();
			qAim = G::UserCmd->viewangles - delta / flSmooth;
		}
	}

	qAim.Normalized();
	qAim.Clamp();

	/*static DWORD dwDelayTime = GetTickCount();

	if (Vars.Legitbot.delay > 0)
	{
		if (!GetAsyncKeyState(0x01))
			dwDelayTime = GetTickCount();

		if (GetTickCount() - dwDelayTime < Vars.Legitbot.delay)
		{
			G::UserCmd->buttons &= ~1;
		}
	}*/

	if (Vars.Legitbot.fastzoom)
	{
		if (G::UserCmd->buttons & IN_ATTACK)
		{
			if (G::LocalPlayer->GetWeapon()->IsSniper()) {
				if (!G::LocalPlayer->IsScoped())
				{
					G::UserCmd->buttons &= ~IN_ATTACK;
					G::UserCmd->buttons |= IN_ZOOM;
				}

				if ((G::UserCmd->buttons & IN_ATTACK) && Vars.Legitbot.fastzoomswitch) {
					I::Engine->ExecuteClientCmd("lastinv");
				}
			}
		}
	}

	G::UserCmd->viewangles = qAim;

	if (!Vars.Legitbot.Weapon[Vars.wpn].pSilent || (G::LocalPlayer->GetShotsFired() > 1 && Vars.Legitbot.Weapon[Vars.wpn].pSilent) || !psile)
		I::Engine->SetViewAngles(G::UserCmd->viewangles);

	return;
}

bool IsHitChance(float flChance, CBaseCombatWeapon *pWep)
{
	if (flChance >= 99.f)
		flChance = 99.f;

	if (flChance < 1)
		flChance = 1;

	float flSpread = pWep->GetInaccuracyReal() * 10;
	return((((100.f - flChance) * 0.65f) * 0.01125) >= flSpread);
}

void CLegitBot::Triggerbot()
{
	CBaseEntity* localplayer = G::LocalPlayer;
	if (!localplayer || !localplayer->GetAlive())
		return;

	if (!localplayer->GetWeapon()->IsGun())
		return;

	long currentTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	static long timeStamp = currentTime_ms;
	long oldTimeStamp;

	Vector traceStart, traceEnd;
	trace_t tr;

	QAngle viewAngles;
	I::Engine->GetViewAngles(viewAngles);
	QAngle viewAngles_rcs = viewAngles + localplayer->GetPunch() * 2.0f;

	M::AngleVectors(viewAngles_rcs, &traceEnd);

	traceStart = localplayer->GetEyePosition();
	traceEnd = traceStart + (traceEnd * 8192.0f);

	if (Vars.Legitbot.Triggerbot.AutoWall)
	{
		CAutowall AutoWalls;
		CAutowall::FireBulletData data;
		if (AutoWalls.GetDamage(traceEnd, data) == 0.0f)
			return;

		tr = data.enter_trace;
	}
	else
	{
		Ray_t ray;
		ray.Init(traceStart, traceEnd);
		CTraceFilter traceFilter;
		traceFilter.pSkip = localplayer;
		I::EngineTrace->TraceRay(ray, 0x46004003, &traceFilter, &tr);
	}

	oldTimeStamp = timeStamp;
	timeStamp = currentTime_ms;

	CBaseEntity* player = tr.m_pEnt;

	if (!player)
		return;

	if (player->GetClientClass()->m_ClassID != 35)
		return;

	if (player == localplayer
		|| player->GetDormant()
		|| !player->GetAlive()
		|| player->GetImmune())
		return;

	if (player->GetTeam() == localplayer->GetTeam() && !Vars.Legitbot.Triggerbot.Filter.Friendly)
		return;

	if (Vars.Legitbot.Aimbot.SmokeCheck && U::LineToSmoke(traceStart, traceEnd, true))
		return;

	if (!tr.m_pEnt)
		return;

	int hitgroup = tr.hitgroup;
	bool didHit = false;

	if (Vars.Legitbot.Triggerbot.Filter.Head && hitgroup == HITGROUP_HEAD)
		didHit = true;
	if (Vars.Legitbot.Triggerbot.Filter.Chest && hitgroup == HITGROUP_CHEST)
		didHit = true;
	if (Vars.Legitbot.Triggerbot.Filter.Stomach && hitgroup == HITGROUP_STOMACH)
		didHit = true;
	if (Vars.Legitbot.Triggerbot.Filter.Arms && (hitgroup == HITGROUP_LEFTARM || hitgroup == HITGROUP_RIGHTARM))
		didHit = true;
	if (Vars.Legitbot.Triggerbot.Filter.Legs && (hitgroup == HITGROUP_LEFTLEG || hitgroup == HITGROUP_RIGHTLEG))
		didHit = true;

	//if (Vars.Legitbot.Triggerbot.Filter.smoke && LineGoesThroughSmoke(tr.startpos, tr.endpos, 1))
	//return;

	if (didHit)
	{
		if (Vars.Legitbot.fastzoom)
		{
			if (G::LocalPlayer->GetWeapon()->IsOneShotSniper())
			{
				if (!G::LocalPlayer->IsScoped())
				{
					G::UserCmd->buttons &= ~IN_ATTACK;
					G::UserCmd->buttons |= IN_ZOOM;
				}
			}
		}
	}

	if (didHit && (!Vars.Legitbot.Triggerbot.HitChance || IsHitChance(Vars.Legitbot.Triggerbot.HitChanceAmt, G::LocalPlayer->GetWeapon()))) {
		if (G::LocalPlayer->GetWeapon()->IsRevolver())
			G::UserCmd->buttons |= IN_ATTACK2;
		else
		{
			float dist = M::VectorDistance(G::LocalPlayer->GetOrigin(), tr.m_pEnt->GetOrigin());

			if (G::LocalPlayer->GetWeapon()->GetItemDefinitionIndex() == WEAPON_TASER && dist < 800.f)
				G::UserCmd->buttons |= IN_ATTACK;
			else if (G::LocalPlayer->GetWeapon()->GetItemDefinitionIndex() != WEAPON_TASER)
				G::UserCmd->buttons |= IN_ATTACK;
		}


	}
}