#pragma once
#include "../Cheat.h"
CLegitBot* LegitBot;

int hitBox;

	float m_fbestfov = 20.0f;
	int m_ibesttargetlegit = -1;

	float deltaTime;
	float curAimTime;

	bool CLegitBot::Run()
	{
		DropTarget();

		static float oldServerTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;
		float serverTime = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;
		deltaTime = serverTime - oldServerTime;
		oldServerTime = serverTime;

		if (Vars.Legitbot.fastzoom && G::LocalPlayer->GetAlive())
		{
			if (G::UserCmd->buttons & IN_ATTACK)
			{
				if (G::LocalPlayer->GetWeapon()->IsSniper()) {
					if (!G::LocalPlayer->IsScoped())
					{
						G::UserCmd->buttons &= ~IN_ATTACK;
						G::UserCmd->buttons |= IN_ZOOM;
					}
					if (G::UserCmd->buttons & IN_ATTACK && Vars.Legitbot.fastzoomswitch) {
						I::Engine->ExecuteClientCmd("lastinv");
					}
				}
			}
		}

		if (!G::LocalPlayer->GetWeapon()->IsGun() || G::LocalPlayer->GetWeapon()->IsEmpty())
			return false;

		if ((G::PressedKeys[Vars.Legitbot.Triggerbot.Key] || Vars.Legitbot.Triggerbot.AutoFire) && Vars.Legitbot.Triggerbot.Enabled)
			Triggerbot();

		if ((!(G::UserCmd->buttons & IN_ATTACK) || m_ibesttargetlegit == -1) && Vars.Legitbot.Aimbot.Enabled)
			FindTarget();

		if (m_ibesttargetlegit != -1 && (Vars.Legitbot.Aimbot.AlwaysOn) || (Vars.Legitbot.Aimbot.OnKey && G::PressedKeys[Vars.Legitbot.Aimbot.Key]))
			GoToTarget();

		return true;
	}
	/*
	void CalcAngle(const Vector& vStart, const Vector& vEnd, Vector& vOut)
	{
		Vector vDelta(vStart - vEnd);

		float fHyp = vDelta.Length();

		vOut.x = RAD2DEG(atan(vDelta.z / fHyp));
		vOut.y = RAD2DEG(atan(vDelta.y / vDelta.x));
		vOut.z = 0.0f;

		if (vDelta.x >= 0.0f)
			vOut.y += 180.0f;
	}

	void MakeVector(const Vector& vIn, Vector& vOut)
	{
		float pitch = DEG2RAD(vIn.x);
		float yaw = DEG2RAD(vIn.y);
		float temp = cos(pitch);

		vOut.x = -temp * -cos(yaw);
		vOut.y = sin(yaw) * temp;
		vOut.z = -sin(pitch);
	}*/

	int ClosestBone(CBaseEntity *Entity)
	{
			hitBox = Vars.Legitbot.Weapon[Vars.wpn].Hitbox;

		int bones = Vars.Visuals.Radar.range;

		if (hitBox >= 9)
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

					auto thisdist = M::GetFov(G::UserCmd->viewangles, M::CalcAngle(G::LocalPlayer->GetEyePosition(), Vector(matrix[hitbox->bone][0][3], matrix[hitbox->bone][1][3], matrix[hitbox->bone][2][3])));

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
			return hitBox;
	}

	void CLegitBot::FindTarget()
	{
			m_fbestfov = Vars.Legitbot.Weapon[Vars.wpn].FOV;

		for (int i = 0; i <= I::Globals->maxClients; i++)
		{
			if (!EntityIsValid(i))
				continue;

			CBaseEntity* Entity = I::ClientEntList->GetClientEntity(i);

			if (!Entity)
				continue;

			Vector vEndS;

				hitBox = Vars.Legitbot.Weapon[Vars.wpn].Hitbox;

			QAngle viewangles = G::UserCmd->viewangles + G::LocalPlayer->GetPunch() * 2.f;

			float fov = M::GetFov(G::UserCmd->viewangles, M::CalcAngle(G::LocalPlayer->GetEyePosition(), Entity->GetBonePosition(ClosestBone(Entity))));
		
		
			if (fov < m_fbestfov)
			{
				m_fbestfov = fov;
				m_ibesttargetlegit = i;
			}
		}
	}
	bool psile = false;

	void CLegitBot::GoToTarget()
	{
		CBaseEntity* Entity = I::ClientEntList->GetClientEntity(m_ibesttargetlegit);

		if (!Entity)
			return;

		hitBox = Vars.Legitbot.Weapon[Vars.wpn].Hitbox;

		Vector predicted = Entity->GetPredicted(Entity->GetBonePosition(ClosestBone(Entity)));

		QAngle dst = M::CalcAngle(G::LocalPlayer->GetEyePosition(), predicted);
		QAngle src = G::UserCmd->viewangles;

		Vector eVecTarget = predicted;
		Vector pVecTarget = G::LocalPlayer->GetEyePosition();

		if (Vars.Legitbot.Aimbot.SmokeCheck && U::LineToSmoke(pVecTarget, eVecTarget, true))
			return;

		float	RCSY = Vars.Legitbot.Weapon[Vars.wpn].RCSAmountY;
		float	RCSX = Vars.Legitbot.Weapon[Vars.wpn].RCSAmountX;



		auto qPunchAngles = G::LocalPlayer->GetPunch();

		dst.y -= qPunchAngles.y * (RCSY / 50.f);
		dst.x -= qPunchAngles.x * (RCSX / 50.f);


		QAngle delta = dst - src;

		delta.Clamp();

		static DWORD dwDelayTime = GetTickCount();

		if (Vars.Legitbot.delay > 0)
		{

			if (!GetAsyncKeyState(0x01))
				dwDelayTime = GetTickCount();

			if (GetTickCount() - dwDelayTime < Vars.Legitbot.delay) {
					if (G::LocalPlayer->GetShotsFired() < 1)
						G::UserCmd->buttons &= ~IN_ATTACK;
			}
			U::Log("B4 SHOOT: %d | DELAYTIME : %d", GetTickCount() - dwDelayTime < Vars.Legitbot.delay, dwDelayTime);
		}

		int m_fov = Vars.Legitbot.Weapon[Vars.wpn].FOV;
		bool psalo = Vars.Legitbot.Weapon[Vars.wpn].pSilent;
		bool salo = Vars.Legitbot.Weapon[Vars.wpn].Silent;
		float smooth = Vars.Legitbot.Weapon[Vars.wpn].Speed;

		if (psalo && GetAsyncKeyState(0x01))
		{
			if (G::LocalPlayer->GetShotsFired() < 1)
			{
				G::SendPacket = false;
				psile = true;
			}
			else
			{
				G::SendPacket = true;
				psile = false;
			}
		}
		if (!psile || !psalo || !salo)
		{
			if (!delta.IsZero())
			{
				float finalTime = delta.Length() / (smooth / 10);

				curAimTime += deltaTime;

				if (curAimTime > finalTime)
					curAimTime = finalTime;

				float percent = curAimTime / finalTime;

				delta *= percent;
				dst = src + delta;
			}
		}

		G::UserCmd->viewangles = dst.Clamp();

		if (G::LocalPlayer->GetShotsFired() > 1 && Vars.Legitbot.Weapon[Vars.wpn].pSilent)
			I::Engine->SetViewAngles(G::UserCmd->viewangles);

		if (!Vars.Legitbot.Weapon[Vars.wpn].Silent)
			I::Engine->SetViewAngles(G::UserCmd->viewangles);
	}

	void CLegitBot::DropTarget()
	{
		if (!EntityIsValid(m_ibesttargetlegit))
		{
			m_ibesttargetlegit = -1;
			curAimTime = 0.f;
		}

	}

	bool CLegitBot::EntityIsValid(int i)
	{
		CBaseEntity* pEntity = I::ClientEntList->GetClientEntity(i);
		int fov = Vars.Legitbot.Weapon[Vars.wpn].FOV;
		hitBox = Vars.Legitbot.Weapon[Vars.wpn].Hitbox;

		if (!pEntity)
			return false;
		if (pEntity == G::LocalPlayer)
			return false;
		if (pEntity->GetHealth() <= 0)
			return false;
		if (pEntity->GetImmune())
			return false;
		if (pEntity->GetDormant())
			return false;
		if (pEntity->GetTeam() == G::LocalPlayer->GetTeam() && !Vars.Legitbot.Aimbot.FriendlyFire)
			return false;
		if (!pEntity->IsVisible(ClosestBone(pEntity)))
			return false;
		if (M::GetFov(G::UserCmd->viewangles, M::CalcAngle(G::LocalPlayer->GetEyePosition(), pEntity->GetBonePosition(ClosestBone(pEntity)))) > fov)
			return false;
		player_info_t pInfo;
		I::Engine->GetPlayerInfo(i, &pInfo);

		if (strcmp(pInfo.guid, XorStr("STEAM_1:0:63143114")) == 0)
			return false;

		if (strcmp(pInfo.guid, XorStr("STEAM_1:1:24446041")) == 0)
			return false;


		return true;
	}

	bool IsHitChance(float flChance, CBaseCombatWeapon *pWep)
	{
		if (flChance >= 99.f)
			flChance = 99.f;

		if (flChance < 1)
			flChance = 1;

		float flSpread = pWep->GetInaccuracyReal(pWep) * 10;
		return((((100.f - flChance) * 0.65f) * 0.01125) >= flSpread);
	}

	void CLegitBot::Triggerbot()
	{
		CBaseEntity* localplayer = G::LocalPlayer;
		if (!localplayer || !localplayer->GetAlive())
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
			if (AutoWalls.GetDamage(traceEnd,data) == 0.0f)
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

		CBaseEntity* player = (CBaseEntity*)tr.m_pEnt;

		if (!player)
			return;

		//I::Engine->GetPlayer

		if (player->GetClientClass()->m_ClassID != 35)
			return;

		if (player == localplayer
			|| player->GetDormant()
			|| !player->GetAlive()
			|| player->GetImmune())
			return;

		if (player->GetTeam() == localplayer->GetTeam() && !Vars.Legitbot.Triggerbot.Filter.Friendly)
			return;

		bool filter;

		if (!tr.m_pEnt)
			return;

		if (strcmp(tr.m_pEnt->GetPlayerInfo().guid, XorStr("STEAM_1:0:63143114")) == 0)
			return;

		if (strcmp(tr.m_pEnt->GetPlayerInfo().guid, XorStr("STEAM_1:1:24446041")) == 0)
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

			if (didHit && (!Vars.Legitbot.Triggerbot.HitChance || IsHitChance(Vars.Legitbot.Triggerbot.HitChanceAmt, G::LocalPlayer->GetWeapon()))) {
				if (G::LocalPlayer->GetWeapon()->IsRevolver())
					G::UserCmd->buttons |= IN_ATTACK2;
				else
					G::UserCmd->buttons |= IN_ATTACK;

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
	}
	QAngle m_oldangle;

	/*void CLegitBot::RCS()
	{
		if (G::UserCmd->buttons & IN_ATTACK)
		{
			QAngle aimpunch = G::LocalPlayer->GetPunch() * (Vars.Legitbot.Aimbot.RCSAmountX / 50.f);


			G::UserCmd->viewangles += (m_oldangle - aimpunch);

			m_oldangle = aimpunch;
		}
		else {
			m_oldangle.x = m_oldangle.y = m_oldangle.z = 0;
		}
	}*/