#include "../Cheat.h"
static bool svchts;
#define RandomInt(min, max) (rand() % (max - min + 1) + min)

OverrideViewFn oOverrideView;
void __fastcall Hooks::OverrideView(void* ecx, void* edx, CViewSetup* vsView)
{
	int SFOV = vsView->fov;

	if (I::Engine->IsInGame())
	{
		if (!G::LocalPlayer->IsScoped())
			vsView->fov = SFOV + Vars.Misc.fov;
		else
			vsView->fov = SFOV;
	}

	G::FOV = vsView->fov;

	if (Vars.Visuals.disablepost.enabled) {
		ConVar* postprocess = I::Cvar->FindVar("mat_postprocess_enable");
		postprocess->SetValue(0);
	}

	if (Vars.Visuals.trace.enabled) {
		ConVar* postprocess = I::Cvar->FindVar("sv_showimpacts");
		postprocess->SetValue(2);
	}

			if (Vars.Visuals.thirdperson.enabled) {
				static bool svchts = false;
				if (!svchts)
				{
					ConVar* cheats = I::Cvar->FindVar(XorStr("sv_cheats"));
					if (cheats != NULL)
					{
						SpoofedConvar* cheats_spoofed = new SpoofedConvar(cheats);
						cheats_spoofed->SetInt(FCVAR_CHEAT);
						svchts = true;
					}
				}

				bool inthp = G::LocalPlayer->GetAlive();

				if (svchts && inthp)
				{
					if (GetKeyState(Vars.Visuals.thirdpersonkey) & 1) {
						I::Engine->ClientCmd_Unrestricted(XorStr("thirdperson"));
					}
					else
					{
						I::Engine->ClientCmd_Unrestricted(XorStr("firstperson"));
					}
				}
				if (!inthp)
				{
					I::Engine->ClientCmd_Unrestricted(XorStr("firstperson"));
				}
			}
			else
			{
				I::Engine->ClientCmd_Unrestricted(XorStr("firstperson"));
			}

			char buf[255];
			sprintf_s(buf, "cam_idealdist %f", Vars.Visuals.thirdperson.distance);
			I::Engine->ClientCmd_Unrestricted(charenc(buf));

			oOverrideView(ecx, vsView);
		}
