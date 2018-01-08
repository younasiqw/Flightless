#include "../Cheat.h"

CFakeLag* FakeLag;

void CFakeLag::Run()
{
	if (Vars.Misc.FakeLags <= 0)
		return;

	if (!I::Engine->IsInGame())
		return;

	if (G::UserCmd->buttons & IN_ATTACK && !Vars.Ragebot.pSilent)
	{
		G::SendPacket = true;
		return;
	}

	if (Vars.Misc.FakeLag == 1) {
		if (ticks >= ticksMax)
		{
			G::SendPacket = true;
			ticks = 0;
		}
		else
		{
			G::SendPacket = ticks < ticksMax - Vars.Misc.FakeLags;
		}
	}
	else if (Vars.Misc.FakeLag == 2) { // Switch
		static int FakelagFactor = Vars.Misc.FakeLags;
		G::SendPacket = 1 ? !(ticks % (FakelagFactor + 1)) : 1;
		if (G::SendPacket) {
			FakelagFactor = (rand() % 7) + 6;
		}
	}
	else if (Vars.Misc.FakeLag == 3) { // Adaptive
		static int FakelagFactor = 2;
		G::SendPacket = 1 ? !(ticks % (FakelagFactor + 1)) : 1;
		if (G::SendPacket)
			FakelagFactor = max(1, min((int)(fabs(G::LocalPlayer->GetVelocity().Length() / 80.f)), Vars.Misc.FakeLags));
	}
	else if (Vars.Misc.FakeLag == 4) { // Pulsive

		Vector vel = G::LocalPlayer->GetVelocity();
		vel.z = 0.0f;

		int f = (int)(ceilf((64.0f / 0.0078125f) / vel.Length()));

		if (Vars.Misc.FakeLags > 0)
		{
			if (f > Vars.Misc.FakeLags)
				f = Vars.Misc.FakeLags;
		}
		else
		{
			if (f > 14)
				f = 14;
		}
		if (f < 1)
			f = 1;

		G::SendPacket = 1 ? !(ticks % (f)) : 1;
	}

	ticks++;
}