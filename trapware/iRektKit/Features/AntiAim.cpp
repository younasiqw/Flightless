#include "../Cheat.h"

#define RandomInt(min, max) (rand() % (max - min + 1) + min)
CAntiAim* AntiAim = new CAntiAim;

float Distance(Vector a, Vector b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void CalcAngle(Vector src, Vector dst, QAngle &angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.00;

	if (delta.x >= 0.0)
		angles.y += 180.0f;
}

bool CAntiAim::GetBestHeadAngle(QAngle& angle)
{
	Vector position = G::LocalPlayer->GetEyePosition();

	float closest_distance = 100.0f;

	float radius = Vars.Ragebot.Antiaim.EdgeDistance + 0.1f;
	float step = M_PI * 2.0 / 8;

	for (float a = 0; a < (M_PI * 2.0); a += step)
	{
		Vector location(radius * cos(a) + position.x, radius * sin(a) + position.y, position.z);

		Ray_t ray;
		trace_t tr;
		ray.Init(position, location);
		CTraceFilter traceFilter;
		traceFilter.pSkip = G::LocalPlayer;
		I::EngineTrace->TraceRay(ray, 0x4600400B, &traceFilter, &tr);

		float distance = Distance(position, tr.endpos);

		if (distance < closest_distance)
		{
			closest_distance = distance;
			angle.y = RAD2DEG(a);
		}
	}

	return closest_distance < Vars.Ragebot.Antiaim.EdgeDistance;
}

bool CAntiAim::HasViableEnemy()
{
	for (int i = 1; i < I::Engine->GetMaxClients(); ++i)
	{
		CBaseEntity* entity = I::ClientEntList->GetClientEntity(i);

		if (!entity
			|| entity == G::LocalPlayer
			|| entity->GetDormant()
			|| !entity->GetAlive()
			|| entity->GetImmune())
			continue;

		if (Vars.Ragebot.FriendlyFire || entity->GetTeam() != G::LocalPlayer->GetTeam())
			return true;
	}

	return false;
}

QAngle AtTargets() {

	if (!Vars.Ragebot.Antiaim.AtPlayer)
		return QAngle(0, 0, 0);

	if (!AntiAim->HasViableEnemy())
		return QAngle(0, 0, 0);

	float lowest = 99999999.f;
	Vector EyePos = G::LocalPlayer->GetEyePosition() + G::LocalPlayer->GetOrigin();

	for (int i = 1; i < 65; i++) {
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEnt = I::ClientEntList->GetClientEntity(i);

		if (!pEnt)
			continue;

		if (pEnt->GetDormant())
			continue;

		if (!pEnt->GetAlive())
			continue;

		if (pEnt->GetTeam() == G::LocalPlayer->GetTeam())
			continue;

		Vector CurPos = pEnt->GetEyePosition() + pEnt->GetOrigin();

		QAngle angles;

		if (CurPos.DistToSqr(EyePos) < lowest) {
			lowest = CurPos.Dist(EyePos);
			CalcAngle(EyePos, CurPos, angles);
			return angles;
		}
	}
	return QAngle(0, 0, 0);
}

void DoAntiAimY(QAngle& angle, bool bFlip)
{
	static bool n1;
	static bool n2;
	float add;
	static bool attck;
	static bool flip;
	static bool flip2;
	flip = !flip;
	flip2 = !flip2;

	static float fYaw = 0.0f;
	static bool yFlip;

	if (bFlip)
		n1 = !n1;
	else
		n2 = !n2;


	if (bFlip)
		yFlip = !yFlip;

	if (Vars.Ragebot.Antiaim.FakeYaw)
	{
		int aa_type = G::SendPacket ? Vars.Ragebot.Antiaim.YawReal : Vars.Ragebot.Antiaim.YawFake;


		if (aa_type == SPIN)
		{
			if (Vars.Ragebot.Antiaim.spinspeed < 1) Vars.Ragebot.Antiaim.spinspeed = 3;
			float CalculatedCurTime_1 = (I::Globals->curtime * (Vars.Ragebot.Antiaim.spinspeed * 100));
			add = 1.f + CalculatedCurTime_1;
		}
		else if (aa_type == ADD90FAKE)
		{
			//if (!flip)
				//angle.y += 1.f + RandomInt(85, 105);
			static bool Switch = false;

			if (GetAsyncKeyState(VK_XBUTTON1) & 1)
				Switch = !Switch;

			if (G::LocalPlayer->GetVelocity().Length2D() > 0.5) {
				add = 179;
			}

			if (Switch)
			{
				if (G::SendPacket) // Fake
					add = 90;
				else		       // Real
					add = -90;
			}
			else
			{
				if (G::SendPacket) // Fake
					add = -90;
				else			   // Real
					add = 90;
			}

			if (G::LastLBYUpdate)
			{
				if (Switch)
				{
					if (G::SendPacket) // Fake
						add = -90;
					else		       // Real
						add = 90;
				}
				else
				{
					if (G::SendPacket) // Fake
						add = 90;
					else			   // Real
						add = -90;
				}
			}
		}
		else if (aa_type == SIDE)
		{
			if (!Vars.Ragebot.Antiaim.AtPlayer)
			{
				if (flip)
					add = 1.f + 90.0f;
				else
					add = 1.f + -90.0f;
			}
			else
			{
				if (flip)
					add = RandomInt(80, 120);
				else
					add = RandomInt(-80, -120);
			}
		}
		else if (aa_type == SIDE2)
		{
			if (G::SendPacket)
			{
				if (G::LocalPlayer->GetVelocity().Length2D() > 1)
				{
					if (!(G::LocalPlayer->GetFlags() & FL_ONGROUND))
					{
						add = 90;
					}
					else
					{
						add = -90;
					}
				}
				else
				{
					add = 170;
				}
			}
			else
			{
				if (!(G::LocalPlayer->GetFlags() & FL_ONGROUND))
				{
					add = -45;
				}
				else
				{
					if (G::LocalPlayer->GetVelocity().Length2D() > 1)
					{
						add = -161;
					}
					else
					{
						add = -20;
					}
				}
			}
		}
		else if (aa_type == BACKWARDS)
			add = 180.0f;
		else if (aa_type == LEFT)
			add = 90.0f;
		else if (aa_type == RIGHT)
			add = -90.0f;
		else if (aa_type == ZERO)
			add = 0.0f;
		else if (aa_type == FAKESPIN)
		{
			static int yawadd;
			int AddRate = I::Globals->curtime * 7000;

			while (AddRate > 180.f)
				AddRate -= 360.f;

			while (AddRate < -180.f)
				AddRate += 360.f;

			add = (flip2 ? AddRate : (yawadd == 5 ? (G::UserCmd->command_number % 2 ? 90 : -90) : (G::UserCmd->command_number % 2 ? RandomInt(60, 195) : RandomInt(-60, -195))));

			yawadd++;
			if (yawadd >= 4)
				yawadd = 0;
		}
		else if (aa_type == LOWERBODY)
		{
			//add = 1.f + (G::FSNLBY + flip2 ? RandomInt(60, 195) : RandomInt(-60, -195));
			static bool Switch = false;

			if (GetAsyncKeyState(VK_XBUTTON1) & 1)
				Switch = !Switch;

			if (G::LocalPlayer->GetVelocity().Length2D() > 0.5) {
				add = 179;
			}

			if (Switch)
				add = -90;
			else
				add = 90;


			if (G::LastLBYUpdate)
			{
				if (Switch)
					add = 90;
				else
					add = -90;
			}
		}
		else if (aa_type == LAGSPIN)
		{
			if (G::SendPacket)
			{
				if (n1)
					add = 1.f + 70;
				else
					add = 1.f + 140;

				if (n2)
					add = 1.f + -70;
				else
					add = 1.f + -140;
			}
			else
			{
				if (n1)
					add = 1.f + RandomInt(55, 80);
				else
					add = 1.f + RandomInt(125, 140);

				if (n2)
					add = 1.f + RandomInt(-55, -80);
				else
					add = 1.f + RandomInt(-125, -140);
			}
		}
		else if (aa_type == MEMESPIN)
		{
			static int jitterangle = 0;

			if (jitterangle <= 1)
			{
				add = 1.f + 135;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				add = 1.f + 225;
			}

			static int iChoked = -1;
			iChoked++;
			if (iChoked < 1)
			{
				G::SendPacket = false;
				if (jitterangle <= 1)
				{
					add = 1.f + 45;
					jitterangle += 1;
				}
				else if (jitterangle > 1 && jitterangle <= 3)
				{
					add = 1.f + 45;
					jitterangle += 1;
				}
				else
				{
					jitterangle = 0;
				}
			}
			else
			{
				G::SendPacket = true;
				iChoked = -1;
			}
			jitterangle++;
		}
		else if (aa_type == TESTING)
		{
			static int tickmeme = 0;

			if (G::LocalPlayer->GetVelocity().Length2D() > 1.f) {
				add = 179.0f;
			}

			if (tickmeme <= 30)
			{
				if (G::SendPacket)
				{
					if (G::LastLBYUpdate)
						add = 219.f;
					else
						add = 149.f;
				}
				else
				{
					if (G::LastLBYUpdate)
						add = 149.f;
					else
						add = 219.f;
				}
			}
			else if (tickmeme >= 30 && tickmeme <= 60)
			{
				if (G::SendPacket)
				{
					if (G::LastLBYUpdate)
						add = 149.f;
					else
						add = 219.f;
				}
				else
				{
					if (G::LastLBYUpdate)
						add = 219.f;
					else
						add = 149.f;
				}
			}
			tickmeme++;

			if (tickmeme >= 61)
				tickmeme = 0;
		}
	}
	else
	{
		int aa_type = Vars.Ragebot.Antiaim.YawReal;


		if (aa_type == SPIN)
		{
			if (Vars.Ragebot.Antiaim.spinspeed < 1) Vars.Ragebot.Antiaim.spinspeed = 3;
			float CalculatedCurTime_1 = (I::Globals->curtime * (Vars.Ragebot.Antiaim.spinspeed * 100));
			add = 1.f + CalculatedCurTime_1;
		}
		else if (aa_type == ADD90FAKE)
		{
			//if (!flip)
			//angle.y += 1.f + RandomInt(85, 105);
			static bool Switch = false;

			if (GetAsyncKeyState(VK_XBUTTON1) & 1)
				Switch = !Switch;

			if (G::LocalPlayer->GetVelocity().Length2D() > 0.5) {
				add = 179;
			}

			if (Switch)
			{
				if (G::SendPacket) // Fake
					add = 90;
				else		       // Real
					add = -90;
			}
			else
			{
				if (G::SendPacket) // Fake
					add = -90;
				else			   // Real
					add = 90;
			}

			if (G::LastLBYUpdate)
			{
				if (Switch)
				{
					if (G::SendPacket) // Fake
						add = -90;
					else		       // Real
						add = 90;
				}
				else
				{
					if (G::SendPacket) // Fake
						add = 90;
					else			   // Real
						add = -90;
				}
			}

		}
		else if (aa_type == SIDE)
		{
			if (!Vars.Ragebot.Antiaim.AtPlayer)
			{
				if (flip)
					add = 1.f + 90.0f;
				else
					add = 1.f + -90.0f;
			}
			else
			{
				if (flip)
					add = RandomInt(80, 120);
				else
					add = RandomInt(-80, -120);
			}
		}
		else if (aa_type == SIDE2)
		{
			if (G::SendPacket)
			{
				if (G::LocalPlayer->GetVelocity().Length2D() > 1)
				{
					if (!(G::LocalPlayer->GetFlags() & FL_ONGROUND))
					{
						add = 90;
					}
					else
					{
						add = -90;
					}
				}
				else
				{
					add = 170;
				}
			}
			else
			{
				if (!(G::LocalPlayer->GetFlags() & FL_ONGROUND))
				{
					add = -45;
				}
				else
				{
					if (G::LocalPlayer->GetVelocity().Length2D() > 1)
					{
						add = -161;
					}
					else
					{
						add = -20;
					}
				}
			}
		}
		else if (aa_type == BACKWARDS)
			add = 180.0f;
		else if (aa_type == LEFT)
			add = 90.0f;
		else if (aa_type == RIGHT)
			add = -90.0f;
		else if (aa_type == ZERO)
			add = 0.0f;
		else if (aa_type == FAKESPIN)
		{
			static int yawadd;
			int AddRate = I::Globals->curtime * 7000;

			while (AddRate > 180.f)
				AddRate -= 360.f;

			while (AddRate < -180.f)
				AddRate += 360.f;

			add = (flip2 ? AddRate : (yawadd == 5 ? (G::UserCmd->command_number % 2 ? 90 : -90) : (G::UserCmd->command_number % 2 ? RandomInt(60, 195) : RandomInt(-60, -195))));

			yawadd++;
			if (yawadd >= 4)
				yawadd = 0;
		}
		else if (aa_type == LOWERBODY)
		{
			//add = 1.f + (G::FSNLBY + flip2 ? RandomInt(60, 195) : RandomInt(-60, -195));
			static bool Switch = false;

			if (GetAsyncKeyState(VK_XBUTTON1) & 1)
				Switch = !Switch;

			if (G::LocalPlayer->GetVelocity().Length2D() > 0.5) {
				add = 179;
			}

			if (Switch)
				add = -90;
			else
				add = 90;

			if (G::LastLBYUpdate)
			{
				if (Switch)
					add = 90;
				else
					add = -90;
			}
		}
		else if (aa_type == LAGSPIN)
		{
			if (G::SendPacket)
			{
				if (n1)
					add = 1.f + 70;
				else
					add = 1.f + 140;

				if (n2)
					add = 1.f + -70;
				else
					add = 1.f + -140;
			}
			else
			{
				if (n1)
					add = 1.f + RandomInt(55, 80);
				else
					add = 1.f + RandomInt(125, 140);

				if (n2)
					add = 1.f + -RandomInt(-55, -80);
				else
					add = 1.f + RandomInt(-125, -140);
			}
		}
		else if (aa_type == MEMESPIN)
		{
			static int jitterangle = 0;

			if (jitterangle <= 1)
			{
				add = 1.f + 135;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				add = 1.f + 225;
			}

			static int iChoked = -1;
			iChoked++;
			if (iChoked < 1)
			{
				G::SendPacket = false;
				if (jitterangle <= 1)
				{
					add = 1.f + 45;
					jitterangle += 1;
				}
				else if (jitterangle > 1 && jitterangle <= 3)
				{
					add = 1.f + 45;
					jitterangle += 1;
				}
				else
				{
					jitterangle = 0;
				}
			}
			else
			{
				G::SendPacket = true;
				iChoked = -1;
			}
			jitterangle++;
		}
		else if (aa_type == TESTING)
		{
			//G::SendPacket = yFlip;
			static int tickmeme = 0;

			if (G::LocalPlayer->GetVelocity().Length2D() > 1.f) {
				add = 179.0f;
			}

			if (tickmeme <= 30)
			{
				if (G::SendPacket)
				{
					if (G::LastLBYUpdate)
						add = 219.f;
					else
						add = 149.f;
				}
				else
				{
					if (G::LastLBYUpdate)
						add = 149.f;
					else
						add = 219.f;
				}
			}
			else if (tickmeme >= 30 && tickmeme <= 60)
			{
				if (G::SendPacket)
				{
					if (G::LastLBYUpdate)
						add = 149.f;
					else
						add = 219.f;
				}
				else
				{
					if (G::LastLBYUpdate)
						add = 219.f;
					else
						add = 149.f;
				}
			}
			tickmeme++;

			if (tickmeme >= 61)
				tickmeme = 0;
		}

		if (Vars.Ragebot.Antiaim.Shuffle)
		{
			if (*G::LocalPlayer->GetLowerBodyYawTarget() == angle.y)
				add = angle.y + 33.983148f;
		}
	}

	angle.y += add;
}

void CAntiAim::Run()
{
	/*	if (Vars.Ragebot.Aimstep && G::Aimbotting)
			return;*/

	if (!Vars.Ragebot.Enabled)
		return;

	if (!G::LocalPlayer || !G::LocalPlayer->GetWeapon() || !G::LocalPlayer->GetAlive())
		return;

	bool canshoot = true;

	QAngle oldAngle = G::UserCmd->viewangles;
	float oldForward = G::UserCmd->forwardmove;
	float oldSideMove = G::UserCmd->sidemove;

	float server_time = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;
	float next_shot = G::LocalPlayer->GetWeapon()->GetNextPrimaryAttack() - server_time;

	if (next_shot > 0 || (Vars.Ragebot.AutoStop && G::LocalPlayer->GetVelocity().Length() > 10))
		canshoot = false;

	static int chocked = 0;

	if (canshoot && (G::UserCmd->buttons & IN_ATTACK) && Vars.Ragebot.pSilent)
	{
		G::SendPacket = false;

		if (G::SendPacket == false)
			chocked++;

		if (chocked >= 13) {
			G::SendPacket = true;
			chocked = 0;
		}
	}

	if (((G::UserCmd->buttons & IN_ATTACK) && canshoot) || G::UserCmd->buttons & IN_USE || G::LocalPlayer->GetWeapon()->IsGrenade())
		return;

	if (G::LocalPlayer->GetMoveType() == MOVETYPE_LADDER || G::LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	if (Vars.Ragebot.Antiaim.knife_held && G::LocalPlayer->GetAlive() && G::LocalPlayer->GetWeapon()->IsKnife())
		return;

	if (Vars.Ragebot.Antiaim.no_enemy && G::LocalPlayer->GetAlive() && !HasViableEnemy()) // 
		return;

	//QAngle anglez;
	QAngle View = G::UserCmd->viewangles;

	if (Vars.Ragebot.Antiaim.AtPlayer)
		View = AtTargets();

	static bool bFlip;
	static float pDance = 0.0f;

	bFlip = !bFlip;

	if (Vars.Ragebot.Antiaim.YawReal)
		DoAntiAimY(View, bFlip);

	if (Vars.Ragebot.UntrustedCheck)
		View.Clamp();


	if (Vars.Ragebot.Antiaim.Pitch)
	{
		int pitch_aa_type = Vars.Ragebot.Antiaim.Pitch;

		if (pitch_aa_type == STATIC_UP)
			View.x = -89.0f;
		else if (pitch_aa_type == STATIC_DOWN)
			View.x = 89.0f;
		else if (pitch_aa_type == DANCE)
		{
			pDance += 10.0f;

			if (pDance > 89.f)
				pDance = -89.f;

			View.x = pDance;

		}
		else if (pitch_aa_type == FRONT)
			View.x = 0.0f;
		else if (pitch_aa_type == EMOTION)
		{
			View.x = 70.000000f;
		}
		else if (pitch_aa_type == STATIC_DOWN_FAKE)
		{
			View.x = bFlip ? 89.0f : 87.000000f;
			G::SendPacket = bFlip;
		}
		else if (pitch_aa_type == SKEETDOWN)
		{
			View.x = 87.000000f;
		}
	}

	if (Vars.Ragebot.UntrustedCheck)
		View.Clamp();

	if (Vars.Ragebot.Antiaim.YawCustom)
		View.y += bFlip ? Vars.Ragebot.Antiaim.YCFake : Vars.Ragebot.Antiaim.YCReal;
	if (Vars.Ragebot.Antiaim.PitchCustom)
		View.x += bFlip ? Vars.Ragebot.Antiaim.PCFake : Vars.Ragebot.Antiaim.PCReal;

	G::UserCmd->viewangles = View;

	M::CorrectMovement(oldAngle, G::UserCmd, oldForward, oldSideMove);
}