#include "../Cheat.h"
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#include <wininet.h>
#include <cstring>
#include <windows.h>
#include <iostream>
#include <urlmon.h>
#include <ctype.h>
#include "../xor.h"

#pragma comment(lib,"wininet.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"WS2_32.lib")
bool done = false;
void CVisuals::Run()
{

	if (I::Engine->IsTakingScreenshot() && Vars.Visuals.AntiSS)
		return;

	if (Vars.Misc.Watermark)
		E::Visuals->Watermark();

	if (!InGame())
		return;

	if (!Vars.Visuals.Enabled)
		return;

	Nightmode();

	if (Vars.Visuals.NoScope)
	{
		if (G::LocalPlayer->IsScoped())
		{
			int Width;
			int Height;
			I::Engine->GetScreenSize(Width, Height);

			Color cColor = Color(0, 0, 0, 255);
			D::DrawLine(Width / 2, 0, Width / 2, Height, cColor);
			D::DrawLine(0, Height / 2, Width, Height / 2, cColor);
		}
	}

	static float rainbow;
	rainbow += 0.005f;

	if (rainbow > 1.f) rainbow = 0.f;

	if (Vars.Legitbot.Aimbot.Enabled && Vars.Visuals.CrosshairFOV)
		FovCrosshair();

	if (Vars.Visuals.CrosshairOn)
	{
		DrawCrosshair(Vars.Visuals.CrosshairRainbow ? Color::FromHSB(rainbow, 1.f, 1.f) : Color::Red());
	}

	for (int i = 0; i < I::ClientEntList->GetHighestEntityIndex(); i++)
	{
		PlayerESP(i);
		WorldESP(i);
	}

}

bool CVisuals::GetBox(CBaseEntity *entity, int& x, int& y, int& w, int& h)
{
	Vector vOrigin, min, max, flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	vOrigin = entity->GetOrigin();
	min = entity->GetCollideable()->OBBMins() + vOrigin;
	max = entity->GetCollideable()->OBBMaxs() + vOrigin;

	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	if (!D::WorldToScreen(points[3], flb) || !D::WorldToScreen(points[5], brt)
		|| !D::WorldToScreen(points[0], blb) || !D::WorldToScreen(points[4], frt)
		|| !D::WorldToScreen(points[2], frb) || !D::WorldToScreen(points[1], brb)
		|| !D::WorldToScreen(points[6], blt) || !D::WorldToScreen(points[7], flt))
		return false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	x = (int)left;
	y = (int)top;
	w = (int)(right - left);
	h = (int)(bottom - top);

	return true;
}

void Line(Vector2D start_pos, Vector2D end_pos, Color col)
{
	D::DrawLine(start_pos.x, start_pos.y, end_pos.x, end_pos.y, col);
}

void Circle(Vector2D position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector2D start(radius * cosf(a) + position.x, radius * sinf(a) + position.y);
		Vector2D end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y);
		Line(start, end, color);
	}
}

void CVisuals::DrawEntity(CBaseEntity* entity, const char* string, Color color)
{
	Vector max = entity->GetCollideable()->OBBMaxs();
	Vector pos, pos3D;
	Vector top, top3D;
	pos3D = entity->GetOrigin();
	top3D = pos3D + Vector(0, 0, max.z);

	if (!D::WorldToScreen(pos3D, pos) || !D::WorldToScreen(top3D, top))
		return;

	float height = (pos.y - top.y);
	float width = height / 4.f;
	//PlayerBox(top.x, top.y, width + 10, height + 10, color);

	D::DrawString(F::Meme, top.x + width + 13, top.y + ((height + 10) / 2), Color(255, 255, 255, 255), FONT_LEFT, string);
}

void CVisuals::DrawThrowable(CBaseEntity* throwable, ClientClass* client)
{
	model_t* nadeModel = throwable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = I::ModelInfo->GetStudioModel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->name, charenc("thrown")) && !strstr(hdr->name, charenc("dropped")))
		return;

	Color nadeColor = Color(255, 255, 255, 255);
	std::string nadeName = charenc("Flying Grenade");

	IMaterial* mats[32];
	I::ModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;

		if (strstr(mat->GetName(), charenc("flashbang")))
		{
			nadeName = charenc("Flashbang");
			nadeColor = Color(238, 233, 75);
			break;
		}
		else if (strstr(mat->GetName(), charenc("m67_grenade")) || strstr(mat->GetName(), charenc("hegrenade")))
		{
			nadeName = charenc("HE Grenade");
			nadeColor = Color(210, 0, 40);
			break;
		}
		else if (strstr(mat->GetName(), charenc("smoke")))
		{
			nadeName = charenc("Smoke");
			nadeColor = Color(93, 175, 97);
			break;
		}
		else if (strstr(mat->GetName(), charenc("decoy")))
		{
			nadeName = charenc("Decoy");
			nadeColor = Color(230, 78, 255);
			break;
		}
		else if (strstr(mat->GetName(), charenc("incendiary")) || strstr(mat->GetName(), charenc("molotov")))
		{
			nadeName = charenc("Molotov");
			nadeColor = Color(208, 131, 0);
			break;
		}
	}

	DrawEntity(throwable, nadeName.c_str(), nadeColor);
}

void CVisuals::DrawOnFloor(CBaseEntity* throwable, ClientClass* client)
{
	model_t* nadeModel = throwable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = I::ModelInfo->GetStudioModel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->name, charenc("thrown")) && !strstr(hdr->name, charenc("dropped")))
		return;

	std::string nadeName = charenc("Flying Grenade");

	IMaterial* mats[32];
	I::ModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;

		if (strstr(mat->GetName(), charenc("flashbang")))
		{
			nadeName = charenc("Flashbang");
			break;
		}
		else if (strstr(mat->GetName(), charenc("m67_grenade")) || strstr(mat->GetName(), charenc("hegrenade")))
		{
			nadeName = charenc("HE Grenade");
			break;
		}
		else if (strstr(mat->GetName(), charenc("smoke")))
		{
			nadeName = charenc("Smoke");
			break;
		}
		else if (strstr(mat->GetName(), charenc("decoy")))
		{
			nadeName = charenc("Decoy");
			break;
		}
		else if (strstr(mat->GetName(), charenc("incendiary")) || strstr(mat->GetName(), charenc("molotov")))
		{
			nadeName = charenc("Molotov");
			break;
		}
	}

	DrawEntity(throwable, nadeName.c_str(), Color::Grey());
}

void CVisuals::FovCrosshair()
{
	if (!G::LocalPlayer->GetAlive())
		return;

	if (!Vars.Visuals.CrosshairFOV)
		return;

	if (!G::LocalPlayer->GetWeapon()->IsGun())
		return;

	int width, height;
	I::Engine->GetScreenSize(width, height);

	bool psile = (Vars.Legitbot.Weapon[G::LocalPlayer->GetWeapon()->GetItemDefinitionIndex()].pSilent && (G::LocalPlayer->GetShotsFired() < 1)) ? true : false;

	float fov = psile ? Vars.Legitbot.Weapon[G::LocalPlayer->GetWeapon()->GetItemDefinitionIndex()].PFOV : Vars.Legitbot.Weapon[G::LocalPlayer->GetWeapon()->GetItemDefinitionIndex()].FOV;

	float radAimbotFov = (float)(fov * M_PI / 180);
	float radViewFov = (float)(G::FOV * M_PI / 180);

	float circleRadius = tanf(radAimbotFov / 2) / tanf(radViewFov / 2) * width;
	#define LOC(x,y) Vector2D(x,y)
	Circle(LOC(width / 2, height / 2), 20, circleRadius, Color::White());
}

std::list<int> GetObservervators(int playerId)
{
	std::list<int> list;

	if (!I::Engine->IsInGame())
		return list;

	CBaseEntity* player = I::ClientEntList->GetClientEntity(playerId);
	if (!player)
		return list;

	if (!player->GetAlive())
	{
		CBaseEntity* observerTarget = I::ClientEntList->GetClientEntityFromHandle(player->GetObserverTargetHandle());
		if (!observerTarget)
			return list;

		player = observerTarget;
	}

	for (int i = 1; i < I::Engine->GetMaxClients(); i++)
	{
		CBaseEntity* pPlayer = I::ClientEntList->GetClientEntity(i);
		if (!pPlayer)
			continue;

		if (pPlayer->GetDormant() || pPlayer->GetAlive())
			continue;

		CBaseEntity* target = I::ClientEntList->GetClientEntityFromHandle(pPlayer->GetObserverTargetHandle());
		if (player != target)
			continue;

		list.push_back(i);
	}

	return list;
}

//Correction_Struct CorrectionsPlayers[65];
void CVisuals::PlayerList()
{
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin(XorStr("Player list"), &Vars.Players.Playerlist, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
	{
		static int currentPlayer = -1;

		if (!I::Engine->IsInGame())
			currentPlayer = -1;

		ImGui::ListBoxHeader(XorStr("##PLAYERS"), ImVec2(-1, (ImGui::GetWindowSize().y - 135)));
		{
			if (I::Engine->IsInGame())
			{
				for (int i = 0; i <= I::Globals->maxClients; i++)
				{

					if (i == I::Engine->GetLocalPlayer())
						continue;

					CBaseEntity* Target = I::ClientEntList->GetClientEntity(i);
					if (!Target)
						continue;
					if (Target->GetClientClass()->m_ClassID != 35)
						continue;

					if (Target->GetTeam() == G::LocalPlayer->GetTeam())
						continue;

					if (Target->GetName() == "GOTV" && !Target->GetWeapon())
						continue;

					player_info_t entityInformation;
					I::Engine->GetPlayerInfo(i, &entityInformation);

					 player_info_t localinfo;I::Engine->GetPlayerInfo(I::Engine->GetLocalPlayer(), &localinfo);

					ImGui::Separator();
					char buf[255]; sprintf_s(buf, u8"%s", entityInformation.name);
					if (ImGui::Selectable(buf, i == currentPlayer, ImGuiSelectableFlags_SpanAllColumns))
						currentPlayer = i;
				}
			}
		}
		ImGui::ListBoxFooter();

		//Vars.Players.AAA.MiscFixes

		if (currentPlayer != -1)
		{
			player_info_t entityInformation;
			I::Engine->GetPlayerInfo(currentPlayer, &entityInformation);

			ImVec2 siz = ImVec2(120, 625 - ImGui::GetCursorPosY() - 40);
			ImVec2 size = ImVec2(200, 625 - ImGui::GetCursorPosY() - 40);
			if (ImGui::BeginChild(XorStr("##PLISTFIX-CHILD"), siz, false))
			{

				bool isFriendly = std::find(CPlayerList::friends.begin(), CPlayerList::friends.end(), entityInformation.userid) != CPlayerList::friends.end();
				if (ImGui::Checkbox(XorStr("Friend"), &isFriendly))
				{
					if (isFriendly)
						CPlayerList::friends.push_back(entityInformation.userid);
					else
						CPlayerList::friends.erase(std::find(CPlayerList::friends.begin(), CPlayerList::friends.end(), entityInformation.userid));
				}

				bool shouldResolve = std::find(CPlayerList::Players.begin(), CPlayerList::Players.end(), entityInformation.userid) != CPlayerList::Players.end();
				if (ImGui::Checkbox(XorStr("Resolver Whitelist"), &shouldResolve))
				{
					if (shouldResolve)
						CPlayerList::Players.push_back(entityInformation.userid);
					else
						CPlayerList::Players.erase(std::find(CPlayerList::Players.begin(), CPlayerList::Players.end(), entityInformation.userid));
				}

				if (ImGui::Combo(XorStr("Pitch"), &Vars.Players.AAA.PAngle[currentPlayer], XorStr("Auto\0\rDown\0\rUp\0\rZero\0\rEmotion\0\0"), -1))
					U::CL_FullUpdate();

				if (ImGui::Combo(XorStr("Yaw"), &Vars.Players.AAA.YAngle[currentPlayer], XorStr("Off\0\rInverse\0\rpResolve\0\rShuffle 1\0\rShuffle 2\0\0"), -1))
					U::CL_FullUpdate();
			} ImGui::EndChild(); ImGui::SameLine();
			if (ImGui::BeginChild(XorStr("##CLEARS-CHILD"), size, false))
			{
				if (ImGui::Button(XorStr("Clear Friends"), ImVec2(93.f, 20.f)))
					CPlayerList::friends.clear();
				if (ImGui::Button(XorStr("Clear Resolve"), ImVec2(93.f, 20.f)))
					CPlayerList::Players.clear();
				if (ImGui::Button(XorStr("Clear Fixes"), ImVec2(93.f, 20.f))) {
					if (I::Engine->IsInGame())
					{
						for (int i = 1; i < I::Engine->GetMaxClients(); i++)
						{
							Vars.Players.AAA.PAngle[i] = 0;
							Vars.Players.AAA.YAngle[i] = 0;
						}
					}
				}
			} ImGui::EndChild(); ImGui::NewLine();
		}
	}
	ImGui::End();
}

void CVisuals::SpecList()
{
	if (!Vars.Visuals.SpectatorList)
		return;

	if (!I::Engine->IsInGame())
		return;

	if (ImGui::Begin(XorStr("Spectators"), &Vars.Visuals.SpectatorList, ImVec2(300, 300), 0.9f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
	{
		//ImGui::Separator();

		for (int playerId : GetObservervators(I::Engine->GetLocalPlayer()))
		{
			CBaseEntity* player = I::ClientEntList->GetClientEntity(playerId);

			player_info_t entityInformation;
			I::Engine->GetPlayerInfo(playerId, &entityInformation);

			//ImGuiStyle& style = ImGui::GetStyle();
			//style.Colors[ImGuiCol_Text] = player->GetTeam() == 2 ? ImVec4(240, 70, 80, 255) : ImVec4(0, 0, 170, 255);

			char* bufix;
			switch (player->GetTeam())
			{
			case 0:
				bufix = XorStr("[NO TEAM]");
				//col = { 100,100,100 };
				break;
			case 1:
				bufix = XorStr("[SPEC]");
				break;
			case 2:
				bufix = XorStr("[T]");
				break;
			case 3:
				bufix = XorStr("[CT]");
				break;
			}

			char buf[255]; sprintf_s(buf, u8"%s | %s", entityInformation.name, bufix);
			//D::DrawString(F::ESP, scrn.right - TextSize.right - 4, (scrn.bottom / 2) + (16 * ayy) + 9, pTarget->GetIndex() == G::LocalPlayer->GetIndex() ? Color(240, 70, 80, 255) : Color(255, 255, 255, 255), FONT_LEFT, buf);
			ImGui::Text(XorStr(u8"%s"), buf);
		}

		//ImGui::Separator();

		ImGui::End();
	}
}

void SnapRotate(int x, int y, int _x, int _y, int& ox, int& oy) {
	ox = _x - x;
	oy = _y - y;

	double ang = atan2(oy, ox);

	double xx = cos(ang - 1.57f);
	double yy = -cos(ang);

	ox = (int)(xx > 0 ? ceil(xx) : floor(xx));
	oy = (int)(yy > 0 ? ceil(yy) : floor(yy));
}

void CVisuals::PlayerESP(int index)
{
	CBaseEntity* Entity = I::ClientEntList->GetClientEntity(index);

	player_info_t pInfo; player_info_t localinfo;
	I::Engine->GetPlayerInfo(index, &pInfo);

	I::Engine->GetPlayerInfo(I::Engine->GetLocalPlayer(), &localinfo);

	if (EntityIsInvalid(Entity))
		return;
	
	int x, y, w, h;
	if (!GetBox(Entity, x, y, w, h))
		return;

	if (Entity->GetClientClass()->m_ClassID != 35)
		return;

	if (!Vars.Visuals.Filter.Friendlies && !Entity->IsEnemy() && Entity != G::LocalPlayer)
		return;

	if (!Vars.Visuals.Filter.Enemies && Entity->IsEnemy())
		return;

	int iScreenWidth, iScreenHeight;
	I::Engine->GetScreenSize(iScreenWidth, iScreenHeight);

	if (Vars.Visuals.Skeleton)
		Skeleton(Entity, Color::White());

	int bottom = 0;



	if (Entity->IsVisible(6) && Vars.Visuals.Box)
		EspBox(Entity, Entity->GetTeam() == 2 ? Color(247, 180, 20) : Color(32, 180, 57));
	else if (!Entity->IsVisible(6) && Vars.Visuals.Box)
		EspBox(Entity, Entity->GetTeam() == 2 ? Color(240, 30, 35) : Color(63, 72, 205));

	if (Vars.Visuals.InfoName) {
		std::string name = Entity->GetName();
		
		D::DrawString(F::ESP, x + (w / 2), y - 10, Color::White(), FONT_CENTER, "%s", name.c_str());
	}

	if (Vars.Visuals.bonebox)
		CVisuals::Hitboxes(Entity, Color::White());

	if (Vars.Visuals.esp3d)
	{
		ICollideable* coll = Entity->GetCollideable();

		Vector min = coll->OBBMins();
		Vector max = coll->OBBMaxs();

		Vector corners[8] =
		{
			Vector(min.x,min.y,min.z),
			Vector(min.x,max.y,min.z),
			Vector(max.x,max.y,min.z),
			Vector(max.x,min.y,min.z),
			Vector(min.x,min.y,max.z),
			Vector(min.x,max.y,max.z),
			Vector(max.x,max.y,max.z),
			Vector(max.x,min.y,max.z)
		};

		float ang = Entity->GetEyeAngles().y;

		for (int i = 0; i <= 7; i++)
			corners[i].Rotate2D(ang);

		Vector _corners[8];

		for (int i = 0; i <= 7; i++)
			D::WorldToScreen(Entity->GetOrigin() + corners[i], _corners[i]);

		int x1 = iScreenWidth * 2, y1 = iScreenHeight * 2, x2 = -iScreenWidth, y2 = -iScreenHeight;

		for (int i = 0; i <= 7; i++)
		{
			x1 = (int)(min(x1, _corners[i].x));
			y1 = (int)(min(y1, _corners[i].y));
			x2 = (int)(max(x2, _corners[i].x));
			y2 = (int)(max(y2, _corners[i].y));
		}

		int diff = x2 - x1;
		int diff2 = y2 - y1;

		static Color col;
		if (Entity->IsVisible(6))
			col = Entity->GetTeam() == 2 ? Color(247, 180, 20) : Color(32, 180, 57);
		else
			col = Entity->GetTeam() == 2 ? Color(240, 30, 35) : Color(63, 72, 205);


		for (int i = 1; i <= 4; i++)
		{
			D::DrawLine((int)(_corners[i - 1].x), (int)(_corners[i - 1].y), (int)(_corners[i % 4].x), (int)(_corners[i % 4].y), col);
			D::DrawLine((int)(_corners[i - 1].x), (int)(_corners[i - 1].y), (int)(_corners[i + 3].x), (int)(_corners[i + 3].y), col);
			D::DrawLine((int)(_corners[i + 3].x), (int)(_corners[i + 3].y), (int)(_corners[i % 4 + 4].x), (int)(_corners[i % 4 + 4].y), col);
		}
	}

	if (Vars.Visuals.InfoHealth) {
		HealthBar(x, y, w, h, Entity->GetHealth(), Entity->GetArmor());
	}

	if (Vars.Visuals.BulletTrace)
		BulletTrace(Entity, Color(200, 50, 50, 150));

	if (Vars.Visuals.Line && Entity->GetTeam() != G::LocalPlayer->GetTeam())
	{
		int ox, oy = 0;

		SnapRotate(iScreenWidth / 2, Vars.Visuals.Line == 1 ? iScreenHeight / 2 : iScreenHeight, x, y + h + 4, ox, oy);
		//D::DrawLine(iScreenWidth / 2 - ox, iScreenHeight - oy, x - ox + (w / 2), y + h + 4 - oy, Color(0, 0, 0));
		D::DrawLine(iScreenWidth / 2, Vars.Visuals.Line == 1 ? iScreenHeight / 2 : iScreenHeight, x + (w / 2), y + h + 4, Color(200, 60, 60));
		//D::DrawLine(iScreenWidth / 2 + ox, iScreenHeight + oy, x + ox + (w / 2), y + h + 4 + oy, Color(0, 0, 0));
	}


	if (Vars.Visuals.InfoWeapon) {
		char mews[519];
		sprintf(mews, "%s (%d/%d)", Entity->GetWeapon()->GetWeaponName().c_str(), Entity->GetWeapon()->Clip(), Entity->GetWeapon()->Clip2());

		RECT TextSizeWEP = D::GetTextSize(F::ESP, XorStr("AK47"));
		D::DrawString(F::ESP, x + w + 11, y + TextSizeWEP.bottom + 2, Color::White(), FONT_LEFT, mews);
	}

	if (Vars.Visuals.InfoFlashed && Entity->IsFlashed())
		D::DrawString(F::ESP, x + (w / 2), y + h + 12 + (10 * bottom++), Color::Green(), FONT_CENTER, XorStr("FLASHED"));

	if (Entity->GetUserID() == Vars.bomber)
		D::DrawString(F::ESP, x + (w / 2), y + h + 12 + (10 * bottom++), Color::Red(), FONT_CENTER, XorStr("BOMB"));

	if (Entity->GetUserID() == Vars.Hostage)
		D::DrawString(F::ESP, x + (w / 2), y + h + 12 + (10 * bottom++), Color::Green(), FONT_CENTER, XorStr("HOSTAGE CARRIER"));

	if (Entity->GetUserID() == Vars.defuser)
		D::DrawString(F::ESP, x + (w / 2), y + h + 12 + (10 * bottom++), Color::Red(), FONT_CENTER, XorStr("DEFUSER"));
}

void CVisuals::WorldESP(int index)
{
	CBaseEntity* Entity = I::ClientEntList->GetClientEntity(index);

	if (!Entity)
		return;

	bombtimer(Entity);

	Vector pos, pos3D;
	pos3D = Entity->GetOrigin();

	if (pos3D.IsZero())
		return;

	if (!D::WorldToScreen(pos3D, pos))
		return;

	int x, y, w, h;
	if (GetBox(Entity, x, y, w, h))
	{
		if (Entity->GetClientClass()->m_ClassID == 85) // CHostage ClassID - Thx
		{
			if (Vars.Visuals.InfoName)
				D::DrawString(F::ESP, x + (w / 2), y - 8, Color::Green(), FONT_CENTER, charenc("Hostage"));

			EspBox(Entity, Color(255, 255, 255));
		}
	}

	int owner = Entity->GetOwner();

	if (owner == -1)
	{
		if (Vars.Visuals.Filter.Weapons)
		{
			if (strstr(Entity->GetClientClass()->m_pNetworkName, XorStr("CWeapon")))
			{
				std::string wpnname = "unknown";

				wpnname = std::string(Entity->GetClientClass()->m_pNetworkName).substr(7);

				DrawEntity(Entity, wpnname.c_str(), Color::White());

			}

			if (Entity->GetClientClass()->m_ClassID == 1) {
				DrawEntity(Entity, XorStr("AK-47"), Color::White());
			}
			else if (Entity->GetClientClass()->m_ClassID == 39)
			{
				DrawEntity(Entity, XorStr("Deagle"), Color::White());
			}
		}

		if (Entity->GetClientClass()->m_ClassID == 29 && Vars.Visuals.Filter.C4) {
			BombCarrier = nullptr;
			D::DrawString(F::ESP, pos.x, pos.y, Color::LightBlue(), FONT_RIGHT, "%s", charenc("C4"));
			DrawEntity(Entity, XorStr("C4 Dropped"), Color::LightBlue());
		}
	}

	if (Entity->GetClientClass()->m_ClassID == 41 && Vars.Visuals.Filter.Decoy)
	{
		CBaseEntity* OwnerEntity = I::ClientEntList->GetClientEntity(owner & 0xFFF);
		//D::DrawString( F::ESP, pos.x, pos.y, Color( 230, 78, 255 ), FONT_RIGHT, charenc( "Decoy [%s]" ), OwnerEntity ? OwnerEntity->GetName().c_str() : charenc( "Disconnected" ) );
		char mews[519];
		sprintf(mews, "Decoy by [%s]", OwnerEntity ? OwnerEntity->GetName().c_str() : "Disconnected");
		DrawEntity(Entity, mews, Color(230, 78, 255));
	}

	if (Vars.Visuals.Filter.Weapons && strstr(Entity->GetClientClass()->m_pNetworkName, XorStr("Projectile")))
	{
		DrawThrowable(Entity, Entity->GetClientClass());
	}
	if (Vars.Visuals.Filter.Weapons && strstr(Entity->GetClientClass()->m_pNetworkName, XorStr("Grenade")) || strstr(Entity->GetClientClass()->m_pNetworkName, XorStr("Flashbang")))
	{
		DrawOnFloor(Entity, Entity->GetClientClass());
	}

	if (strstr(Entity->GetClientClass()->m_pNetworkName, XorStr("CPlantedC4")) && Vars.Visuals.Filter.C4 && Vars.bomb) {
		char mew[519];
		sprintf(mew, "Planted C4 [%.1f]", Entity->GetBombTimer());
		D::DrawString(F::ESP, pos.x, pos.y, Color::Red(), FONT_CENTER, XorStr("C4 [%.1f]"), Entity->GetBombTimer());
		DrawEntity(Entity, mew, Color::Red());
	}
}

bool CVisuals::InGame()
{
	if( !I::Engine->IsInGame() )
		return false;

	if( !G::LocalPlayer )
		return false;

	return true;
}

bool CVisuals::EntityIsInvalid(CBaseEntity* Entity)
{
	//CBaseEntity *pTarget = I::ClientEntList->GetClientEntityFromHandle(Entity->GetObserverTargetHandle());
	if (!Entity)
		return true;
	if (Entity == G::LocalPlayer && !Vars.Visuals.thirdperson.enabled)
		return true;
	CBaseEntity* observerTarget = I::ClientEntList->GetClientEntityFromHandle(G::LocalPlayer->GetObserverTargetHandle());
	//if (Entity == pTarget)
		//return true;
	if (Entity == observerTarget)
		return true;
	if (Entity->GetHealth() <= 0)
		return true;
	if (Entity->GetDormant())
		return true;

	return false;
}




void CVisuals::BulletTraceOverlay(CBaseEntity* entity, Color color)
{
	Vector src, dst, forward;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	M::AngleVectors(entity->GetEyeAngles(), &forward);
	filter.pSkip = entity;
	src = entity->GetBonePosition(6);
	dst = src + (forward * 1500.f);

	ray.Init(src, dst);

	I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	I::DebugOverlay->AddLineOverlay(src, tr.endpos, color.r(), color.g(), color.b(), true, -1);
}

void CVisuals::BulletTrace(CBaseEntity* entity, Color color)
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	M::AngleVectors(entity->GetEyeAngles(), &forward);
	filter.pSkip = entity;
	src3D = entity->GetBonePosition(8) - Vector(0, 0, 0);
	dst3D = src3D + (forward * 1500);

	ray.Init(src3D, dst3D);

	I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!D::WorldToScreen(src3D, src) || !D::WorldToScreen(tr.endpos, dst))
		return;

	D::DrawLine(src.x, src.y, dst.x, dst.y, color);
	D::DrawRect(dst.x - 3, dst.y - 3, 6, 6, color);
}


void CVisuals::Watermark()
{
	//char* shit = banretard();

	
		//static const char* meme = ("nicotinecheats");
		//D::DrawString(F::Watermark, 5, 5, Color(255, 255, 255), FONT_LEFT, "%s", shit);
	

	
	RECT TextxSizeR = D::GetTextSize(F::Revue, "LBY");

	int screenW, screenH;
	I::Engine->GetScreenSize(screenW, screenH);

	if (InGame() && G::LocalPlayer->GetAlive())
		D::DrawString(F::Revue, 5, 1000, G::LastLBYUpdate ? Color(200, 0, 0) : Color(0, 200, 0), FONT_LEFT, XorStr("LBY"));
}

void CVisuals::bombtimer(CBaseEntity* pEntity)
{
	RECT TextxSizeS = D::GetTextSize(F::Watermark, "A | T");
	//14
	int screenW, screenH;
	I::Engine->GetScreenSize(screenW, screenH);

	int light = screenW - TextxSizeS.right - 20;

	if (Vars.bomb && strstr(pEntity->GetClientClass()->m_pNetworkName, XorStr("CPlantedC4")))
		D::DrawString(F::ESP, light + 10, 12 + TextxSizeS.bottom + 3, Color::Red(), FONT_CENTER, "%.1f", pEntity->GetBombTimer());
}

void CVisuals::PlayerBox(float x, float y, float w, float h, Color clr)
{
	I::Surface->DrawSetColor(clr);
	I::Surface->DrawOutlinedRect(x - w, y, x + w, y + h);

	I::Surface->DrawSetColor(Color::Black());
	I::Surface->DrawOutlinedRect(x - w - 1, y - 1, x + w + 1, y + h + 1);
	I::Surface->DrawOutlinedRect(x - w + 1, y + 1, x + w - 1, y + h - 1);
}

void CVisuals::EspBox(CBaseEntity* pEntityPlayer, Color clr)
{
	int x, y, w, h;
	GetBox(pEntityPlayer, x, y, w, h);
	{
		// Corner Box
		int VertLine = (((float)w) * (0.20f));
		int HorzLine = (((float)h) * (0.20f));

		D::DrawRect(x, y - 1, VertLine, 1, Color(10, 10, 10, 150));
		D::DrawRect(x + w - VertLine, y - 1, VertLine, 1, Color(10, 10, 10, 150));
		D::DrawRect(x, y + h - 1, VertLine, 1, Color(10, 10, 10, 150));
		D::DrawRect(x + w - VertLine, y + h - 1, VertLine, 1, Color(10, 10, 10, 150));

		D::DrawRect(x - 1, y, 1, HorzLine, Color(10, 10, 10, 150));
		D::DrawRect(x - 1, y + h - HorzLine, 1, HorzLine, Color(10, 10, 10, 150));
		D::DrawRect(x + w - 1, y, 1, HorzLine, Color(10, 10, 10, 150));
		D::DrawRect(x + w - 1, y + h - HorzLine, 1, HorzLine, Color(10, 10, 10, 150));

		D::DrawRect(x, y, VertLine, 1, clr);
		D::DrawRect(x + w - VertLine, y, VertLine, 1, clr);
		D::DrawRect(x, y + h, VertLine, 1, clr);
		D::DrawRect(x + w - VertLine, y + h, VertLine, 1, clr);

		D::DrawRect(x, y, 1, HorzLine, clr);
		D::DrawRect(x, y + h - HorzLine, 1, HorzLine, clr);
		D::DrawRect(x + w, y, 1, HorzLine, clr);
		D::DrawRect(x + w, y + h - HorzLine, 1, HorzLine, clr);
	}
}

void CVisuals::HealthBar(int x, int y, int w, int h, float health, float armor)
{
	int hp = health;
	if (hp > 100)
		hp = 100;

	int Width = (w * (hp / 100.f));

	int Red = 255 - (hp*2.55);
	int Green = hp*2.55;


	D::DrawRect(x - 1, y + h + 1, Width + 2, 5, Color(10, 10, 10, 150));
	D::DrawRect(x, y + h + 2, Width, 3, Color(Red, Green, 0, 255));
	D::DrawString(F::Watermark, x + Width, y + h + 4, Color(255, 255, 255), FONT_LEFT, "%.0f", health);
}

void CVisuals::Skeleton(CBaseEntity *Entity, Color color)
{
	studiohdr_t* pStudioModel = I::ModelInfo->GetStudioModel(Entity->GetModel());
	if (pStudioModel)
	{
		static matrix3x4_t pBoneToWorldOut[128];
		if (Entity->SetupBones(pBoneToWorldOut, 128, 256, I::Globals->curtime))
		{
			for (int i = 0; i < pStudioModel->numbones; i++)
			{
				mstudiobone_t* pBone = pStudioModel->pBone(i);
				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
					continue;

				Vector vBonePos1;
				if (!D::WorldToScreen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1))
					continue;

				Vector vBonePos2;
				if (!D::WorldToScreen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2))
					continue;

				D::DrawLine((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, color);
			}
		}
	}
}

void CVisuals::Nightmode()
{
	if (Vars.Visuals.nightmode.enabled)
	{
		if (!done)
		{


			static auto sv_skyname = I::Cvar->FindVar("sv_skyname");
			static auto r_DrawSpecificStaticProp = I::Cvar->FindVar("r_DrawSpecificStaticProp");
			r_DrawSpecificStaticProp->SetValue(1);
			sv_skyname->SetValue("sky_csgo_night02");

			for (MaterialHandle_t i = I::MaterialSystem->FirstMaterial(); i != I::MaterialSystem->InvalidMaterial(); i = I::MaterialSystem->NextMaterial(i))
			{
				IMaterial *pMaterial = I::MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{
					pMaterial->ColorModulate(0.10, 0.10, 0.10);
				}
				if (strstr(group, "StaticProp"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}

				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				done = true;
			}

		}
	}
	else
	{
		if (done)
		{
			for (MaterialHandle_t i = I::MaterialSystem->FirstMaterial(); i != I::MaterialSystem->InvalidMaterial(); i = I::MaterialSystem->NextMaterial(i))
			{
				IMaterial *pMaterial = I::MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "StaticProp"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
			}
			done = false;
		}
	}
}

void CVisuals::DrawHitbox(matrix3x4a_t* matrix, Vector bbmin, Vector bbmax, int bone, Color color)
{
	Vector points[] = {
		Vector(bbmin.x, bbmin.y, bbmin.z),
		Vector(bbmin.x, bbmax.y, bbmin.z),
		Vector(bbmax.x, bbmax.y, bbmin.z),
		Vector(bbmax.x, bbmin.y, bbmin.z),
		Vector(bbmax.x, bbmax.y, bbmax.z),
		Vector(bbmin.x, bbmax.y, bbmax.z),
		Vector(bbmin.x, bbmin.y, bbmax.z),
		Vector(bbmax.x, bbmin.y, bbmax.z)
	};

	Vector pointsTransformed[8];

	for (int index = 0; index <= 8; ++index)
	{
		if (index != 0)
			points[index] = ((((points[index] + points[0]) * .5f) + points[index]) * .5f);

		M::VectorTransform(points[index], matrix[bone], pointsTransformed[index]);
	}

	D::Draw3DBox(pointsTransformed, color);
}

void CVisuals::Hitboxes(CBaseEntity *Entity, Color color)
{
	matrix3x4a_t matrix[MAXSTUDIOBONES];

	if (!Entity->SetupBones(matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, I::Engine->GetLastTimeStamp()))
		return;

	studiohdr_t* pStudioModel = I::ModelInfo->GetStudioModel(Entity->GetModel());
	if (!pStudioModel)
		return;

	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(Entity->GetHitboxSet());
	if (!set)
		return;

	//for (int i = 0; i < set->numhitboxes; i++)
	//{
		mstudiobbox_t* hitbox = set->pHitbox(0);
		if (!hitbox)
			return;

		int bone = hitbox->bone;

		Vector vMaxUntransformed = hitbox->bbmax;
		Vector vMinUntransformed = hitbox->bbmin;

		if (hitbox->radius != -1.f)
		{
			vMinUntransformed -= Vector(hitbox->radius, hitbox->radius, hitbox->radius);
			vMaxUntransformed += Vector(hitbox->radius, hitbox->radius, hitbox->radius);
		}

		DrawHitbox(matrix, vMinUntransformed, vMaxUntransformed, hitbox->bone, color);
	//}
}

void CVisuals::DrawCrosshair(Color color)
{
	int screenW, screenH;
	I::Engine->GetScreenSize(screenW, screenH);

	int crX = screenW / 2, crY = screenH / 2;
	int dy = screenH / G::FOV;
	int dx = screenW / G::FOV;
	int drX;
	int drY;

	if (Vars.Visuals.CrosshairType)
	{
		if (Vars.Visuals.RemovalsVisualRecoil)
		{
			drX = crX - (int)(dx * (G::LocalPlayer->GetPunch().y * 2));
			drY = crY + (int)(dy * (G::LocalPlayer->GetPunch().x * 2));
		}
		else
		{
			drX = crX - (int)(dx * (((G::LocalPlayer->GetPunch().y * 2.f) * 0.45f) + G::LocalPlayer->GetViewPunch().y));
			drY = crY + (int)(dy * (((G::LocalPlayer->GetPunch().x * 2.f) * 0.45f) + G::LocalPlayer->GetViewPunch().x));
		}
	}
	else
	{
		drX = crX;
		drY = crY;
	}

	if (Vars.Visuals.CrosshairStyle == 0)
	{
		//D::DrawCircle( drX, drY, 5, 30, color );
		if (G::LocalPlayer->GetAlive() && I::Engine->IsInGame() && I::Engine->IsConnected())
		{
			CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();
			auto accuracy = 0.f;

			if (Vars.Visuals.CrosshairSpread)
				accuracy = (pWeapon->GetInaccuracyReal() + pWeapon->GetSpreadReal()) * 500.f;
			else
				accuracy = 0.f;

			D::DrawLine(crX + accuracy, crY, crX + 5 + accuracy, crY, color);
			D::DrawLine(crX - 5 - accuracy, crY, crX - accuracy, crY, color);
			D::DrawLine(crX, crY + accuracy, crX, crY + 5 + accuracy, color);
			D::DrawLine(crX, crY - 5 - accuracy, crX, crY - accuracy, color);

			if (Vars.Visuals.CrosshairType)
				D::DrawBox(drX - 1, drY - 1, 3, 3, color);
		}
	}
	if (Vars.Visuals.CrosshairStyle == 1)
	{
		//D::DrawCircle( drX, drY, 5, 30, color );
		if (G::LocalPlayer->GetAlive() && I::Engine->IsInGame() && I::Engine->IsConnected())
		{
			CBaseCombatWeapon* pWeapon = G::LocalPlayer->GetWeapon();

			auto accuracy = (pWeapon->GetInaccuracyReal() + pWeapon->GetSpreadReal()) * 500.f;

			if (pWeapon->IsGun())
				D::DrawCircle(drX, drY, Vars.Visuals.CrosshairSpread ? accuracy + 3 : 6, 30, color);
			else
				D::DrawCircle(drX, drY, 6, 30, color);
		}
		else
		{
			D::DrawCircle(drX, drY, 6, 30, color);
		}
	}

	if (Vars.Visuals.CrosshairStyle == 2)
	{
		//D::DrawOutlinedRect( drX - 5, drY - 5, 10, 30, color );


		POINT Middle; Middle.x = drX; Middle.y = drY;
		int a = (int)(drY / 30);
		float gamma = atan(a / a);

		static int rotationdegree = 0;

		if (rotationdegree > 89) { rotationdegree = 0; }

		for (int i = 0; i < 4; i++)
		{
			std::vector <int> p;
			p.push_back(a * sin(DEG2RAD(rotationdegree + (i * 90))));									//p[0]		p0_A.x
			p.push_back(a * cos(DEG2RAD(rotationdegree + (i * 90))));									//p[1]		p0_A.y
			p.push_back((a / cos(gamma)) * sin(DEG2RAD(rotationdegree + (i * 90) + RAD2DEG(gamma))));	//p[2]		p0_B.x
			p.push_back((a / cos(gamma)) * cos(DEG2RAD(rotationdegree + (i * 90) + RAD2DEG(gamma))));	//p[3]		p0_B.y

			D::DrawLine(Middle.x, Middle.y, Middle.x + p[0], Middle.y - p[1], color);
			//D::DrawLine(Middle.x + p[0], Middle.y - p[1], Middle.x + p[2], Middle.y - p[3], Color(255, 255, 0, 255));
		}
		rotationdegree++;
	}
}