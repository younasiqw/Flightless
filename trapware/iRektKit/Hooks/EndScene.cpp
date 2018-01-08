#include "../Cheat.h"
#include "Menu.h"
#include "../Parser.h"
#include "../SkinChanger.h"
#include "../ImGUI/imgui_internal.h"
#include "../ImGUI/imgui.h"
#include <locale>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

void Render();

void GUI_Init(IDirect3DDevice9* pDevice);

bool EntityIsInvalid(CBaseEntity* Entity)
{
	//HANDLE obs = Entity->GetObserverTargetHandle();
	//CBaseEntity *pTarget = I::ClientEntList->GetClientEntityFromHandle(obs);
	if (!Entity)
		return true;
	//if (Entity == pTarget)
	//return true;
	if (Entity->GetHealth() <= 0)
		return true;
	if (Entity->GetDormant())
		return true;

	return false;
}

Vector2D RotatePoint(Vector EntityPos, Vector LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians = false)
{
	float r_1, r_2;
	float x_1, y_1;

	r_1 = -(EntityPos.y - LocalPlayerPos.y);
	r_2 = EntityPos.x - LocalPlayerPos.x;
	float Yaw = angle - 90.0f;

	float yawToRadian = Yaw * (float)(M_PI / 180.0F);
	x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
	y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

	*viewCheck = y_1 < 0;

	x_1 *= zoom;
	y_1 *= zoom;

	int sizX = sizeX / 2;
	int sizY = sizeY / 2;

	x_1 += sizX;
	y_1 += sizY;

	if (x_1 < 5)
		x_1 = 5;

	if (x_1 > sizeX - 5)
		x_1 = sizeX - 5;

	if (y_1 < 5)
		y_1 = 5;

	if (y_1 > sizeY - 5)
		y_1 = sizeY - 5;


	x_1 += posX;
	y_1 += posY;


	return Vector2D(x_1, y_1);


	/*if (!angleInRadians)
	angle = (float)(angle * (M_PI / 180));
	float cosTheta = (float)cos(angle);
	float sinTheta = (float)sin(angle);
	Vector2 returnVec = Vector2(
	cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y),
	sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y)
	);
	returnVec += centerPoint;
	return returnVec / zoom;*/
}

void DrawRadar()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 oldPadding = style.WindowPadding;
	float oldAlpha = style.Colors[ImGuiCol_WindowBg].w;
	style.WindowPadding = ImVec2(0, 0);
	style.Colors[ImGuiCol_WindowBg].w = 1.f;
	if (ImGui::Begin(XorStr("Radar"), &Vars.Visuals.Radar.Enabled, ImVec2(200, 200), 0.4F, ImGuiWindowFlags_NoTitleBar |/*ImGuiWindowFlags_NoResize | */ImGuiWindowFlags_NoCollapse))
	{
		ImVec2 siz = ImGui::GetWindowSize();
		ImVec2 pos = ImGui::GetWindowPos();

		ImDrawList* DrawList = ImGui::GetWindowDrawList();

		DrawList->AddRect(ImVec2(pos.x - 6, pos.y - 6), ImVec2(pos.x + siz.x + 6, pos.y + siz.y + 6), Color::Black().GetU32(), 0.0F, -1, 1.5f);
		//_drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Black().GetU32(), 0.0F, -1, 1);

		//_drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Black().GetU32(), 0.0F, -1, 2);
		//_drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Silver().GetU32(), 0.0F, -1, 1.1f);

		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
		windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), Color::Black().GetU32(), 1.5f);
		windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), Color::Black().GetU32(), 1.5f);

		// Rendering players

		if (I::Engine->IsInGame() && I::Engine->IsConnected())
		{
			//CPlayer* pLocalEntity = Interface.EntityList->GetClientEntity<CPlayer>(Interface.Engine->GetLocalPlayer());
			if (G::LocalPlayer)
			{
				Vector LocalPos = G::LocalPlayer->GetEyePosition();
				QAngle ang;
				I::Engine->GetViewAngles(ang);
				for (int i = 0; i < I::Engine->GetMaxClients(); i++) {
					CBaseEntity* pBaseEntity = I::ClientEntList->GetClientEntity(i);

					player_info_t pInfo;
					I::Engine->GetPlayerInfo(i, &pInfo);

					if (EntityIsInvalid(pBaseEntity))
						continue;

					CBaseEntity* observerTarget = I::ClientEntList->GetClientEntityFromHandle(G::LocalPlayer->GetObserverTargetHandle());

					bool bIsEnemy = (G::LocalPlayer->GetTeam() != pBaseEntity->GetTeam() || pBaseEntity == observerTarget || pBaseEntity == G::LocalPlayer) ? true : false;

					if (Vars.Visuals.Radar.EnemyOnly && !bIsEnemy)
						continue;

					bool viewCheck = false;
					Vector2D EntityPos = RotatePoint(pBaseEntity->GetOrigin(), LocalPos, pos.x, pos.y, siz.x, siz.y, ang.y, Vars.Visuals.Radar.range, &viewCheck);

					//ImU32 clr = (bIsEnemy ? (isVisibled ? Color::LightGreen() : Color::Blue()) : Color::White()).GetU32();
					ImU32 clr = (bIsEnemy ? Color::Red() : Color::LightBlue()).GetU32();
					static bool drawname = true;

					if (pBaseEntity == observerTarget || pBaseEntity == G::LocalPlayer)
					{
						clr = Color::White().GetU32();
						drawname = false;
					}
					else
						drawname = true;

					int s = 2;

					windowDrawList->AddRect(ImVec2(EntityPos.x - s, EntityPos.y - s),
						ImVec2(EntityPos.x + s, EntityPos.y + s),
						clr);

					RECT TextSize = D::GetTextSize(F::ESP, pInfo.name);

					if (drawname && Vars.Visuals.Radar.Nicks)
						windowDrawList->AddText(ImVec2(EntityPos.x - (TextSize.left / 2), EntityPos.y - s), Color::White().GetU32(), pInfo.name);
				}
			}
		}
	}
	ImGui::End();
	style.WindowPadding = oldPadding;
	style.Colors[ImGuiCol_WindowBg].w = oldAlpha;
}

void color()
{
	auto bColor = Vars.g_fBColor;
	auto mColor = Vars.g_fMColor;
	auto tColor = Vars.g_fTColor;
	//int(enemyColor[0] * 255.0f), int(enemyColor[1] * 255.0f), int(enemyColor[2] * 255.0f), 255

	ImColor mainColor = ImColor(int(54), int(54), int(54), 255);
	ImColor bodyColor = ImColor(int(24), int(24), int(24), 255);
	ImColor fontColor = ImColor(int(255), int(255), int(255), 255);

	ImGuiStyle& style = ImGui::GetStyle();

	ImVec4 mainColorHovered = ImVec4(mainColor.Value.x + 0.1f, mainColor.Value.y + 0.1f, mainColor.Value.z + 0.1f, mainColor.Value.w);
	ImVec4 mainColorActive = ImVec4(mainColor.Value.x + 0.2f, mainColor.Value.y + 0.2f, mainColor.Value.z + 0.2f, mainColor.Value.w);
	ImVec4 menubarColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w - 0.8f);
	ImVec4 frameBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .1f);
	ImVec4 tooltipBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .05f);

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8, 8);
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 0.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.ChildWindowRounding = 0.0f;
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 2.0f;
	style.ItemSpacing = ImVec2(4, 2);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 3.0f;
	style.ScrollbarSize = 8.f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 1.0f;
	style.GrabRounding = 0.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.AntiAliasedLines = true;
	style.AntiAliasedShapes = true;
	style.CurveTessellationTol = 1.25f;

	style.Colors[ImGuiCol_Text] = fontColor;

	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = bodyColor;
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(.0f, .0f, .0f, .0f);
	style.Colors[ImGuiCol_PopupBg] = tooltipBgColor;
	style.Colors[ImGuiCol_Border] = mainColorActive;
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = frameBgColor;
	style.Colors[ImGuiCol_FrameBgHovered] = mainColorHovered;
	style.Colors[ImGuiCol_FrameBgActive] = mainColorActive;
	style.Colors[ImGuiCol_TitleBg] = mainColor;
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = mainColor;
	style.Colors[ImGuiCol_MenuBarBg] = menubarColor;
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(frameBgColor.x + .05f, frameBgColor.y + .05f, frameBgColor.z + .05f, frameBgColor.w);
	style.Colors[ImGuiCol_ScrollbarGrab] = mainColor;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = mainColorHovered;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = mainColorActive;
	style.Colors[ImGuiCol_ComboBg] = frameBgColor;
	style.Colors[ImGuiCol_CheckMark] = mainColor;
	style.Colors[ImGuiCol_SliderGrab] = mainColorHovered;
	style.Colors[ImGuiCol_SliderGrabActive] = mainColorActive;
	style.Colors[ImGuiCol_Button] = mainColor;
	style.Colors[ImGuiCol_ButtonHovered] = mainColorHovered;
	style.Colors[ImGuiCol_ButtonActive] = mainColorActive;
	style.Colors[ImGuiCol_Header] = mainColor;
	style.Colors[ImGuiCol_HeaderHovered] = mainColorHovered;
	style.Colors[ImGuiCol_HeaderActive] = mainColorActive;

	style.Colors[ImGuiCol_Column] = mainColor;
	style.Colors[ImGuiCol_ColumnHovered] = mainColorHovered;
	style.Colors[ImGuiCol_ColumnActive] = mainColorActive;

	style.Colors[ImGuiCol_ResizeGrip] = mainColor;
	style.Colors[ImGuiCol_ResizeGripHovered] = mainColorHovered;
	style.Colors[ImGuiCol_ResizeGripActive] = mainColorActive;
	style.Colors[ImGuiCol_CloseButton] = mainColor;
	style.Colors[ImGuiCol_CloseButtonHovered] = mainColorHovered;
	style.Colors[ImGuiCol_CloseButtonActive] = mainColorActive;
	style.Colors[ImGuiCol_PlotLines] = mainColor;
	style.Colors[ImGuiCol_PlotLinesHovered] = mainColorHovered;
	style.Colors[ImGuiCol_PlotHistogram] = mainColor;
	style.Colors[ImGuiCol_PlotHistogramHovered] = mainColorHovered;
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

char* KeyStrings[254] = { "No key", "Left Mouse", "Right Mouse", "Control+Break", "Middle Mouse", "Mouse 4", "Mouse 5",
nullptr, "Backspace", "TAB", nullptr, nullptr, nullptr, "ENTER", nullptr, nullptr, "SHIFT", "CTRL", "ALT", "PAUSE",
"CAPS LOCK", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ESC", nullptr, nullptr, nullptr, nullptr, "SPACEBAR",
"PG UP", "PG DOWN", "END", "HOME", "Left", "Up", "Right", "Down", nullptr, "Print", nullptr, "Print Screen", "Insert",
"Delete", nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", "Left Windows", "Right Windows", nullptr, nullptr, nullptr, "NUM 0", "NUM 1", "NUM 2", "NUM 3", "NUM 4", "NUM 5", "NUM 6",
"NUM 7", "NUM 8", "NUM 9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
"F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, "NUM LOCK", "SCROLL LOCK", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, "LSHIFT", "RSHIFT", "LCONTROL", "RCONTROL", "LMENU", "RMENU", nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Next Track", "Previous Track", "Stop", "Play/Pause", nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[{", "\\|", "}]", "'\"", nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

#pragma once

bool get_system_font_path(const std::string& name, std::string& path)
{
	//
	// This code is not as safe as it should be.
	// Assumptions we make:
	//  -> GetWindowsDirectoryA does not fail.
	//  -> The registry key exists.
	//  -> The subkeys are ordered alphabetically
	//  -> The subkeys name and data are no longer than 260 (MAX_PATH) chars.
	//

	char buffer[MAX_PATH];
	HKEY registryKey;

	GetWindowsDirectoryA(buffer, MAX_PATH);
	std::string fontsFolder = buffer + std::string("\\Fonts\\");

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &registryKey)) {
		return false;
	}

	uint32_t valueIndex = 0;
	char valueName[MAX_PATH];
	uint8_t valueData[MAX_PATH];
	std::wstring wsFontFile;

	do {
		uint32_t valueNameSize = MAX_PATH;
		uint32_t valueDataSize = MAX_PATH;
		uint32_t valueType;

		auto error = RegEnumValueA(
			registryKey,
			valueIndex,
			valueName,
			reinterpret_cast<DWORD*>(&valueNameSize),
			0,
			reinterpret_cast<DWORD*>(&valueType),
			valueData,
			reinterpret_cast<DWORD*>(&valueDataSize));

		valueIndex++;

		if (error == ERROR_NO_MORE_ITEMS) {
			RegCloseKey(registryKey);
			return false;
		}

		if (error || valueType != REG_SZ) {
			continue;
		}

		if (_strnicmp(name.data(), valueName, name.size()) == 0) {
			path = fontsFolder + std::string((char*)valueData, valueDataSize);
			RegCloseKey(registryKey);
			return true;
		}
	} while (true);

	return false;
}


void Render()
{
	ImGui::GetIO().MouseDrawCursor = Vars.Menu.Opened;


	if (Vars.Menu.Opened)
	{
		E::Visuals->PlayerList();

		int pX, pY;
		I::InputSystem->GetCursorPosition(&pX, &pY);
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos.x = (float)(pX);
		io.MousePos.y = (float)(pY);
		ImGui::Begin("Trapware", &Vars.Menu.Opened, ImVec2(500,300), 1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoTitleBar);
		{
			ImGui::BeginChild("mainchild", ImVec2(484, 286), true);
			{
				
				// fünf tabs
				if (ImGui::Button("rage", ImVec2(90, 20))) Vars.Menu.iTab = 1;
				ImGui::SameLine();
				if (ImGui::Button("legit", ImVec2(90, 20))) Vars.Menu.iTab = 2;
				ImGui::SameLine();
				if (ImGui::Button("visuals", ImVec2(90, 20))) Vars.Menu.iTab = 3;
				ImGui::SameLine();
				if (ImGui::Button("misc", ImVec2(90, 20))) Vars.Menu.iTab = 4;
				ImGui::SameLine();
				if (ImGui::Button("colors", ImVec2(90, 20))) Vars.Menu.iTab = 5;
				
				ImGui::BeginChild("mainchild", ImVec2(466, 226), true);
				{
					if (Vars.Menu.iTab == 0)
					{
						ImGui::Text("Trapware.Club, Welcome back. Enjoy your stay!");



					}
					else if (Vars.Menu.iTab == 1) // rage tab
					{
						ImGui::BeginChild("Aimbot", ImVec2(148, 210), true);
						{
							ImGui::Text("Aimbot");
							ImGui::Separator();
							ImGui::PushItemWidth(128);
							ImGui::Checkbox(XorStr("Ragebot Enabled"), &Vars.Ragebot.Enabled);
							
							ImGui::Checkbox(XorStr("Anti-Untrusted"), &Vars.Ragebot.UntrustedCheck);
							ImGui::SliderFloat(XorStr("FOV"), &Vars.Ragebot.FOV, 1.f, 180.f, "%.0f");
							ImGui::Checkbox(XorStr("Friendly Fire"), &Vars.Ragebot.FriendlyFire);
							ImGui::Checkbox(XorStr("Auto Fire"), &Vars.Ragebot.AutoFire);
							
							ImGui::Checkbox(XorStr("AutoPistol"), &Vars.Ragebot.AutoPistol);
							ImGui::Checkbox(XorStr("Silent"), &Vars.Ragebot.Silent);
							;
							ImGui::Checkbox(XorStr("pSilent"), &Vars.Ragebot.pSilent);
							ImGui::Text(XorStr("Recoil"));
							
							ImGui::Combo(XorStr("##RCSTYPE"), &Vars.Ragebot.RCS, XorStr("Off\0\rRemove Recoil\0\0"), -1);

							ImGui::NewLine();
							ImGui::Text("Selection");
							ImGui::Separator();

							ImGui::Combo(XorStr("Hitbox"), &Vars.Ragebot.Hitbox, XorStr("PELVIS\0\r\0\r\0\rHIP\0\rLOWER SPINE\0\rMIDDLE SPINE\0\rUPPER SPINE\0\rNECK\0\rHEAD\0\0"), -1);
							ImGui::Text(XorStr("HitScan"));
							ImGui::Combo(XorStr("HitScan"), &Vars.Ragebot.HitScan, charenc("Off\0\rSelected\0\0"), -1);
							
							ImGui::Checkbox(XorStr("MultiPoint"), &Vars.Ragebot.MultiPoints);
							ImGui::Text(XorStr("Scale"));
							
							ImGui::SliderFloat(XorStr("##MultiPointscale"), &Vars.Ragebot.PointScale, 0.f, 1.f, "%.01f");
							ImGui::Checkbox(XorStr("Autowall"), &Vars.Ragebot.AutoWall);

							ImGui::Text(XorStr("HitScan Bones"));
							ImGui::Selectable(XorStr("Head"), &Vars.Ragebot.bones[HITBOX_HEAD], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan head"));

							

							ImGui::Selectable(XorStr("Neck"), &Vars.Ragebot.bones[HITBOX_NECK], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan neck"));

							

							ImGui::Selectable(XorStr("Pelvis"), &Vars.Ragebot.bones[HITBOX_PELVIS], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan pelvis"));


							ImGui::Selectable(XorStr("Spine"), &Vars.Ragebot.bones[HITBOX_SPINE], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan spine"));

							

							ImGui::Selectable(XorStr("Legs"), &Vars.Ragebot.bones[HITBOX_LEGS], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan legs"));

							

							ImGui::Selectable(XorStr("Arms"), &Vars.Ragebot.bones[HITBOX_ARMS], 0, ImVec2(93.f, 20.f));
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip(XorStr("HitScan arms"));
						}ImGui::EndChild();

						ImGui::SameLine();

						ImGui::BeginChild("HVH", ImVec2(148, 210), true);
						{
							ImGui::Text("HVH");
							ImGui::Separator();
							ImGui::PushItemWidth(128);
							ImGui::Text(XorStr("Resolver"));
							ImGui::Combo(XorStr("Resolver"), &Vars.Ragebot.Antiaim.Resolver, XorStr("Disabled\0\rDynamic\0\rBruteforce\0\0"), -1);
							ImGui::Text(XorStr("Position Adjust"));
							ImGui::Combo(XorStr("Adjustment"), &Vars.Ragebot.ResolverHelper, XorStr("No\0\rNormal\0\0"), -1);

							ImGui::Checkbox(XorStr("Auto Stop"), &Vars.Ragebot.AutoStop);
							ImGui::Checkbox(XorStr("Auto Crouch"), &Vars.Ragebot.AutoCrouch);
							ImGui::Checkbox(XorStr("Auto Scope"), &Vars.Ragebot.AutoScope);

							ImGui::Checkbox(XorStr("Hit Chance"), &Vars.Ragebot.HitChance);

							ImGui::SliderFloat(XorStr("##HCAmount"), &Vars.Ragebot.HitChanceAmt, 1.f, 100.f, "%.0f");

							ImGui::Text(XorStr("Min Dmg"));

							ImGui::SliderFloat(XorStr("##MinDamage"), &Vars.Ragebot.AutoWallDmg, 0.1f, 120.f, "%.0f");

						}ImGui::EndChild();

						ImGui::SameLine();

						ImGui::BeginChild("Antiaim", ImVec2(148, 210), true);
						{
							ImGui::Text("Antiaim");
							ImGui::Separator();
							ImGui::PushItemWidth(128);
							ImGui::Checkbox(XorStr("Enabled"), &Vars.Ragebot.Antiaim.Enabled);
							ImGui::Checkbox(XorStr("Fake Yaw"), &Vars.Ragebot.Antiaim.FakeYaw);
							ImGui::Checkbox(XorStr("Dynamic"), &Vars.Ragebot.Antiaim.AtPlayer);
							ImGui::Checkbox(XorStr("Custom Offsets"), &Vars.Ragebot.Antiaim.YawCustom);
							ImGui::Text("Pitch");
							ImGui::Separator();
							ImGui::Combo(XorStr("  ."), &Vars.Ragebot.Antiaim.Pitch, charenc("None\0\rUp\0\rDown\0\rJitter\0\rZero\0\rHalfDown\0\rDown Jitter\0\rDown2\0\0"), -1);
							ImGui::Text("Fake");
							ImGui::Separator();
							ImGui::Combo(XorStr("  .."), &Vars.Ragebot.Antiaim.YawFake, charenc("None\0\rSpin\0\rJitter\0\rSideways\0\rFake Sideways\0\rBackwards\0\rLeft\0\rRight\0\rZero\0\rFake Spin\0\rLowerBody\0\rLag Spin\0\rRandom Spin\0\rFake Head\0\0"), -1);
							ImGui::Separator();
							ImGui::Text(XorStr("Fake Yaw"));
							(ImGui::SliderFloat(XorStr("##FakeYaw"), &Vars.Ragebot.Antiaim.YCFake, 0, 360));
							ImGui::Text("Real");
							ImGui::Separator();
							ImGui::Combo(XorStr("  ..."), &Vars.Ragebot.Antiaim.YawReal, charenc("None\0\rSpin\0\rFake Lowerbody\0\rSideways\0\rFake Sideways\0\rBackwards\0\rLeft\0\rRight\0\rZero\0\rFake Spin\0\rLowerBody\0\rLag Spin\0\rRandom Spin\0\rFake Head\0\0"), -1);
							ImGui::Separator();
							ImGui::Text(XorStr("Real Yaw"));
							(ImGui::SliderFloat(XorStr("##RealYaw"), &Vars.Ragebot.Antiaim.YCReal, 0, 360));
							ImGui::Separator();
						}ImGui::EndChild();
					}
									else if (Vars.Menu.iTab == 2) // legit tab
									{
										ImGui::Text("Under Construction.");

									}
									else if (Vars.Menu.iTab == 3) // visuals tab
									{
										ImGui::BeginChild("Visuals", ImVec2(148, 210), true);
										{
											ImGui::Text("Visuals");
											ImGui::Separator();
											ImGui::Selectable(XorStr(" -Show Enemies"), &Vars.Visuals.Filter.Enemies);
											ImGui::Selectable(XorStr(" -Show Teammates"), &Vars.Visuals.Filter.Friendlies);
											ImGui::Selectable(XorStr(" -Show Weapons"), &Vars.Visuals.Filter.Weapons);
											ImGui::Selectable(XorStr(" -Show Decoy"), &Vars.Visuals.Filter.Decoy);
											ImGui::Selectable(XorStr(" -Show C4"), &Vars.Visuals.Filter.C4);

											ImGui::NewLine();
											ImGui::Text("Aimlines");
											ImGui::Combo(XorStr("     Aimlines"), &Vars.Visuals.Line, XorStr("Off\0\rCentered\0\rBottom\0\0"), -1);
											ImGui::Checkbox(XorStr("Enable Visuals"), &Vars.Visuals.Enabled);
											ImGui::Checkbox(XorStr("2D Box"), &Vars.Visuals.Box);
											ImGui::Checkbox(XorStr("3D Box"), &Vars.Visuals.esp3d);
											ImGui::Checkbox(XorStr("Head Box"), &Vars.Visuals.bonebox);
											ImGui::Checkbox(XorStr("Skeleton"), &Vars.Visuals.Skeleton);
											ImGui::Checkbox(XorStr("Barrel"), &Vars.Visuals.BulletTrace);
											ImGui::Checkbox(XorStr("Anti-Screenshot"), &Vars.Visuals.AntiSS);

											ImGui::NewLine();
											ImGui::Selectable(XorStr(" -Show Name"), &Vars.Visuals.InfoName);
											ImGui::Selectable(XorStr(" -Show Health"), &Vars.Visuals.InfoHealth);
											ImGui::Selectable(XorStr(" -Show Weapon"), &Vars.Visuals.InfoWeapon);
											ImGui::Selectable(XorStr(" -Show If Flashed"), &Vars.Visuals.InfoFlashed);
										}ImGui::EndChild();
										ImGui::SameLine();
										ImGui::BeginChild("Rendering", ImVec2(148, 210), true);
										{
											ImGui::Text("Rendering");
											ImGui::Separator();
											ImGui::Checkbox(XorStr("Chams Enable"), &Vars.Visuals.Chams.Enabled);
											ImGui::Combo(XorStr("      Mode"), &Vars.Visuals.Chams.Mode, XorStr("Flat\0\rTextured\0\0"), -1);
											ImGui::Checkbox(XorStr("XQZ"), &Vars.Visuals.Chams.XQZ);
											ImGui::Checkbox(XorStr("Hands"), &Vars.Visuals.Chams.hands);
											ImGui::Checkbox(XorStr("Weapon"), &Vars.Visuals.ChamsWeapon);

											ImGui::Text(XorStr("Removals"));
											ImGui::Checkbox(XorStr("No V Recoil"), &Vars.Visuals.RemovalsVisualRecoil);
											ImGui::Checkbox(XorStr("No Flash"), &Vars.Visuals.RemovalsFlash);
											ImGui::Checkbox(XorStr("No Scope"), &Vars.Visuals.NoScope);

											ImGui::Text(XorStr("Override Fov"));
											ImGui::SliderInt(XorStr("##OverrideFov"), &Vars.Misc.fov, -70, 70);
											ImGui::Checkbox(XorStr("ThirdPerson"), &Vars.Visuals.thirdperson.enabled);
											ImGui::Combo(XorStr("Key"), &Vars.Visuals.thirdpersonkey, keyNames, IM_ARRAYSIZE(keyNames));
											ImGui::Text(XorStr("Distance"));
											ImGui::SliderFloat(XorStr("##THRDPERSDIST"), &Vars.Visuals.thirdperson.distance, 30.f, 200.f, "%.0f");
											ImGui::Checkbox(XorStr("Disable PP"), &Vars.Visuals.disablepost.enabled);
											ImGui::Checkbox(XorStr("Nightmode"), &Vars.Visuals.nightmode.enabled);
										}ImGui::EndChild();
										ImGui::SameLine();
										ImGui::BeginChild("Drawing", ImVec2(148, 210), true);
										{
											ImGui::Text("Drawing");
											ImGui::Separator();
											ImGui::Checkbox(XorStr("Spectator List"), &Vars.Visuals.SpectatorList);
											ImGui::Separator();
											ImGui::Text("Radar");
											ImGui::Separator();
											ImGui::Checkbox(XorStr("Enable Radar"), &Vars.Visuals.Radar.Enabled);
											ImGui::SliderInt(XorStr("Range"), &Vars.Visuals.Radar.range, 1, 5);
											ImGui::Checkbox(XorStr("Only Enemy"), &Vars.Visuals.Radar.EnemyOnly);
											ImGui::Checkbox(XorStr("Show Nicknames"), &Vars.Visuals.Radar.Nicks);
											ImGui::Separator();
											ImGui::Checkbox(XorStr("Crosshair Enabled"), &Vars.Visuals.CrosshairOn);
											ImGui::Checkbox(XorStr("Show Spread"), &Vars.Visuals.CrosshairSpread);
											ImGui::Checkbox(XorStr("Show FOV"), &Vars.Visuals.CrosshairFOV);
											ImGui::Checkbox(XorStr("Show Recoil"), &Vars.Visuals.CrosshairType);
											ImGui::Checkbox(XorStr("Rainbow Color"), &Vars.Visuals.CrosshairRainbow);
											ImGui::Text(XorStr("Crosshair Style"));
											ImGui::Combo(XorStr("##CRSHRStyle"), &Vars.Visuals.CrosshairStyle, XorStr("Plus\0\rCircle\0\rBox\0\0"), -1);
											ImGui::Checkbox(XorStr("Bullet Impacts"), &Vars.Visuals.trace.enabled);
										}ImGui::EndChild();
									}
									else if (Vars.Menu.iTab == 4) // misc tab
									{
										ImGui::BeginChild("Misc", ImVec2(148, 210), true);  
										{
											ImGui::Text("Misc");
											ImGui::Separator();
											ImGui::Text(XorStr("Movement"));
											ImGui::Combo(XorStr("Flag"), &Vars.Misc.FakeLag, XorStr("Off\0\rFactor\0\rSwitch\0\rAdaptive\0\rAdaptive 2\0\0"), -1);
											ImGui::SliderInt(XorStr("##FAKELAG"), &Vars.Misc.FakeLags, 0, 16);
											ImGui::Checkbox(XorStr("Bhop"), &Vars.Misc.Bhop);
											ImGui::Combo(XorStr("##AStrafe"), &Vars.Misc.AutoStrafe, XorStr("Off\0\rStrafe Helper\0\rFull Strafer\0\0"), -1);
											ImGui::Checkbox(XorStr("Air Stuck"), &Vars.Misc.AirStuck);
											if (Vars.Misc.AirStuck)
											{
												ImGui::Combo(XorStr("Key"), &Vars.Misc.AirStuckKey, keyNames, IM_ARRAYSIZE(keyNames));
											}
											ImGui::Text(XorStr("Miscellaneous"));
											ImGui::Checkbox(XorStr("Ranks Reveal"), &Vars.Misc.Ranks);
											ImGui::Checkbox(XorStr("Auto Accept"), &Vars.Misc.AutoAccept);
											ImGui::Separator();
											//ImGui::Text(XorStr("Clantag"));
											//ImGui::Separator();
											ImGui::Checkbox(XorStr("Enabled"), &Vars.Misc.ClantagChanger.enabled);
											ImGui::Text(XorStr("Text"));
											static bool init_clan = false;
											if (!init_clan)
											{
												sprintf(Vars.Misc.ClantagChanger.value, "trapware.club");
												init_clan = true;
											}

											if (ImGui::InputText(XorStr("##CLANTAGTEXT"), Vars.Misc.ClantagChanger.value, 30))
												E::ClantagChanger->UpdateClanTagCallback();
											ImGui::Text(XorStr("Clantag Type"));
											if (ImGui::Combo(XorStr("##ANIM"), &Vars.Misc.ClantagChanger.type, XorStr("Static\0\rMarquee\0\rWords\0\rLetters\0\rTime\0\0"), -1))
												E::ClantagChanger->UpdateClanTagCallback();
											ImGui::Text(XorStr("Animation Speed"));
											if (ImGui::SliderInt(XorStr("##ANIMSPEED"), &Vars.Misc.ClantagChanger.animation_speed, 0, 500))
												E::ClantagChanger->UpdateClanTagCallback();
										}ImGui::EndChild();
										ImGui::SameLine();

										ImGui::BeginChild("Skinchanger", ImVec2(148, 210), true);
										{
											ImGui::Text("Skins");
											ImGui::Separator();
											if (G::LocalPlayer->GetAlive())
												ImGui::Text(XorStr("%s"), G::LocalPlayer->GetWeapon()->GetWeaponName().c_str());
											else
												ImGui::Text(XorStr("Not Alive"));


											ImGui::Combo(XorStr("Skin"), &Vars.Skins.Weapons[Vars.wpn].PaintKit, [](void* data, int idx, const char** out_text)
											{
												*out_text = k_Skins[idx].strName.c_str();
												return true;
											}, nullptr, k_Skins.size(), 10);
											ImGui::Separator();
											ImGui::Combo(XorStr("Knife"), &Vars.Skins.KnifeModel, charenc("No Knife\0\rBayonet\0\rFlip Knife\0\rGut Knife\0\rKarambit\0\rM9 Bayonet\0\rHuntsman Knife\0\rButterfly Knife\0\rFalchion Knife\0\rShadow Daggers\0\rBowie Knife\0\0"), -1);
											ImGui::Separator();

											if (ImGui::Combo(XorStr("Glove"), &Vars.Skins.Glove, XorStr("No Glove\0\rBloodhound\0\rSport\0\rDriver\0\rWraps\0\rMoto\0\rSpecialist\0\0"), -1))
												U::CL_FullUpdate();
											ImGui::Separator();
											const char* gstr;
											if (Vars.Skins.Glove == 1)
											{
												gstr = XorStr("Charred\0\rSnakebite\0\rBronzed\0\rGuerilla\0\0");
											}
											else if (Vars.Skins.Glove == 2)
											{
												gstr = XorStr("Superconductor\0\rPandoras Box\0\rHedge Maze\0\rArid\0\0");
											}
											else if (Vars.Skins.Glove == 3)
											{
												gstr = XorStr("Lunar Weave\0\rConvoy\0\rCrimson Weave\0\rDiamondback\0\0");
											}
											else if (Vars.Skins.Glove == 4)
											{
												gstr = XorStr("Leather\0\rSpruce DDPAT\0\rSlaughter\0\rBadlands\0\0");
											}
											else if (Vars.Skins.Glove == 5)
											{
												gstr = XorStr("Eclipse\0\rSpearmint\0\rBoom!\0\rCool Mint\0\0");
											}
											else if (Vars.Skins.Glove == 6)
											{
												gstr = XorStr("Forest DDPAT\0\rCrimson Kimono\0\rEmerald Web\0\rFoundation\0\0");
											}
											else
												gstr = XorStr("No Glove\0\0");

											if (ImGui::Combo(XorStr("##2"), &Vars.Skins.GloveSkin, gstr, -1))
												U::CL_FullUpdate();
											if (ImGui::Button(XorStr("Full Update"), ImVec2(93.f, 20.f))) {
												U::CL_FullUpdate();
											}
										}ImGui::EndChild();

									}
									else if (Vars.Menu.iTab == 5) // colors tab
									{
										ImGui::BeginChild("Colors", ImVec2(450, 210), true);
										{
											ImGui::Text(XorStr("Chams Colors"));
											ImGui::ColorEdit3(XorStr("Chams CT Visible"), Vars.g_iChamsCTV);
											ImGui::ColorEdit3(XorStr("Chams CT Hidden"), Vars.g_iChamsCTH);
											ImGui::ColorEdit3(XorStr("Chams T Visible"), Vars.g_iChamsTV);
											ImGui::ColorEdit3(XorStr("Chams T Hidden"), Vars.g_iChamsTH);

										}ImGui::EndChild();
									} 
				}ImGui::EndChild();

				if (ImGui::Button(XorStr("Save"), ImVec2(93.f, 20.f))) Config->Save();
				ImGui::SameLine();
				if (ImGui::Button(XorStr("Load"), ImVec2(93.f, 20.f))) {
					Config->Load(); color();
				}
				ImGui::SameLine();
				ImGui::PushItemWidth(145);
				ImGui::InputText(XorStr("Configs - Trapware"), Vars.Misc.configname, 30);
			}ImGui::EndChild();

		}ImGui::End();
			
		
			
	}
}


EndSceneFn oEndScene;
long __stdcall Hooks::EndScene(IDirect3DDevice9* pDevice)
{
	if (!G::d3dinit)
		GUI_Init(pDevice);

	H::D3D9->ReHook();

	ImGui::GetIO().MouseDrawCursor = Vars.Menu.Opened;

	ImGui_ImplDX9_NewFrame(); 

	if (Vars.Visuals.SpectatorList)
		E::Visuals->SpecList();

	if (Vars.Visuals.Radar.Enabled && I::Engine->IsConnected())
		DrawRadar();

	Render();

	ImGui::Render();
	return oEndScene(pDevice);
}

ResetFn oReset;
long __stdcall Hooks::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	if (!G::d3dinit)
		return oReset(pDevice, pPresentationParameters);

	ImGui_ImplDX9_InvalidateDeviceObjects();

	auto hr = oReset(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();


	return hr;
}

void GUI_Init(IDirect3DDevice9* pDevice)
{
	ImGui_ImplDX9_Init(G::Window, pDevice);

	color();

	G::d3dinit = true;
}
std::vector<std::string> configs;


void GetConfigMassive()
{
	//get all files on folder

	configs.clear();

	static char path[MAX_PATH];
	std::string szPath1;

	if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
		return;

	szPath1 = std::string(path) + XorStr("\\Trapware\\*");


	WIN32_FIND_DATA FindFileData;
	HANDLE hf;
	configs.push_back("default.ini");

	hf = FindFirstFile(szPath1.c_str(), &FindFileData);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			std::string filename = FindFileData.cFileName;

			if (filename == ".")
				continue;

			if (filename == "..")
				continue;

			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (filename.find(".ini") != std::string::npos)
				{
					configs.push_back(std::string(filename));
				}
			}
		} while (FindNextFile(hf, &FindFileData) != 0);
		FindClose(hf);
	}
}

void configtabyeet()
{
	//ImGui::Text(XorStr("Configs"));
	//ImGui::Separator();
	GetConfigMassive();
	ImGui::Text(XorStr("Config Name"));
	//configs
	static int selectedcfg = 0;
	static std::string cfgname = "default";
	if (ImGui::Combo(XorStr("Presets"), &selectedcfg, [](void* data, int idx, const char** out_text)
	{
		*out_text = configs[idx].c_str();
		return true;
	}, nullptr, configs.size(), 10))
	{
		cfgname = configs[selectedcfg];
		cfgname.erase(cfgname.length() - 4, 4);
		strcpy(Vars.Misc.configname, cfgname.c_str());
	}
	ImGui::InputText(XorStr("Current Cfg"), Vars.Misc.configname, 128);
	if (ImGui::Button(XorStr("Save Config"), ImVec2(93.f, 20.f))) Config->Save();
	ImGui::SameLine();
	if (ImGui::Button(XorStr("Load Config"), ImVec2(93.f, 20.f))) {
		Config->Load(); color();
	}
}