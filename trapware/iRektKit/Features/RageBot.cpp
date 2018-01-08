#include "../Cheat.h"
//#include "../nospread.h"
// lazy
float m_bestfov = 360.0f;
float m_bestdist = 8192.f;
float m_bestthreat = 0.f;
//Vector m_besthitbox = Vector( 0, 0, 0 );
CBaseEntity* m_bestent = nullptr;
bool m_target = false;
QAngle AimStepLastAngle;
#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / I::Globals->interval_per_tick ) )
#define TICKS_TO_TIME( t ) ( I::Globals->interval_per_tick *( t ) )
std::vector<int> CPlayerList::friends = {};

std::unordered_map<int, std::vector<const char*>> hitboxes = {
		{ HITBOX_HEAD, { "head_0" } },
		{ HITBOX_NECK, { "neck_0" } },
		{ HITBOX_PELVIS, { "pelvis" } },
		{ HITBOX_SPINE, { "spine_0", "spine_1", "spine_2", "spine_3" } },
		{ HITBOX_LEGS, { "leg_upper_L", "leg_upper_R", "leg_lower_L", "leg_lower_R", "ankle_L", "ankle_R" } },
		{ HITBOX_ARMS, { "hand_L", "hand_R", "arm_upper_L", "arm_lower_L", "arm_upper_R", "arm_lower_R" } },
};

bool CRageBot::Run()
{
	QAngle oldAngle;
	I::Engine->GetViewAngles(oldAngle);
	oldForward = G::UserCmd->forwardmove;
	oldSideMove = G::UserCmd->sidemove;

	if (!Vars.Ragebot.Enabled)
		return false;

	G::BestTarget = -1;

	if (!G::LocalPlayer->GetWeapon()->IsGun() || G::LocalPlayer->GetWeapon()->IsEmpty())
		return false;

	if (G::BestTarget == -1)
		FindTarget();

	if (Vars.Ragebot.Hold && !G::PressedKeys[Vars.Ragebot.HoldKey])
		return false;

	//PositionAdjusment();

	if (G::BestTarget != -1 && !G::LocalPlayer->GetWeapon()->IsReloading() && m_target)
		GoToTarget();

	M::CorrectMovement(oldAngle, G::UserCmd, oldForward, oldSideMove);

	return true;
}

void CRageBot::GetBestBone(CBaseEntity* entity, float& best_damage, Bone& best_bone)
{
	best_bone = BONE_HEAD;

	for (std::unordered_map<int, std::vector<const char*>>::iterator it = hitboxes.begin(); it != hitboxes.end(); it++)
	{
		if (!Vars.Ragebot.bones[it->first])
			continue;

		std::vector<const char*> hitboxList = hitboxes[it->first];
		for (std::vector<const char*>::iterator it2 = hitboxList.begin(); it2 != hitboxList.end(); it2++)
		{
			Bone bone = (Bone)entity->GetBoneByName(*it2);
			Vector vec_bone = entity->GetBonePosition(bone);

			float damage = AutoWall->GetDamagez(vec_bone);

			if (damage > best_damage)
			{
				best_damage = damage;
				best_bone = bone;
			}
		}
	}
}

void CRageBot::FindTarget()
{
	m_bestfov = Vars.Ragebot.FOV;
	m_bestdist = 8192.f;
	m_bestthreat = 0.f;
	G::BestHitboxVec = Vector(0, 0, 0);
	m_target = false;
	m_bestent = nullptr;

	for (int i = 0; i <= I::Globals->maxClients; i++)
	{
		if (!EntityIsValid(i))
			continue;

		player_info_t entityInformation;
		I::Engine->GetPlayerInfo(i, &entityInformation);


		if (std::find(CPlayerList::friends.begin(), CPlayerList::friends.end(), entityInformation.userid) != CPlayerList::friends.end())
			continue;

		CBaseEntity* Entity = I::ClientEntList->GetClientEntity(i);
		Vector hitbox = Entity->GetBonePosition(Vars.Ragebot.Hitbox);

		QAngle viewAngles;
		I::Engine->GetViewAngles(viewAngles);

		float fov = M::GetFov(viewAngles, M::CalcAngle(G::LocalPlayer->GetEyePosition(), hitbox));
		if (fov < m_bestfov)
		{
			G::BestTarget = i;
			if (Vars.Ragebot.HitScan > 0)
			{
				if (Vars.Ragebot.HitScan == 1)
				{
					//if( BestAimPointAll( Entity, m_besthitbox ) )
					//	m_target = true;
					float damage = 0.0f;
					Bone bone;
					GetBestBone(Entity, damage, bone);

					if (damage >= Vars.Ragebot.AutoWallDmg)
					{
						m_bestent = Entity;
						G::BestHitboxVec = Entity->GetBonePosition(bone);
						m_target = true;
					}
				}
			}
			else
			{
				m_bestent = Entity;
				m_bestfov = fov;
				G::BestHitboxVec = hitbox;
				m_target = true;
			}
		}
	}
}

void VectorTransform(Vector& in1, matrix3x4_t& in2, Vector &out)
{
	out.x = in1.Dot(in2.m_flMatVal[0]) + in2.m_flMatVal[0][3];
	out.y = in1.Dot(in2.m_flMatVal[1]) + in2.m_flMatVal[1][3];
	out.z = in1.Dot(in2.m_flMatVal[2]) + in2.m_flMatVal[2][3];
}

bool HitChance(float flChance, CBaseCombatWeapon *pWep)
{
	if (flChance >= 99.f)
		flChance = 99.f;

	if (flChance < 1)
		flChance = 1;

	float flSpread = pWep->GetInaccuracyReal() * 10;
	return ((((100.f - flChance) * 0.65f) * 0.01125) >= flSpread);
} 

bool MultiPoints(CBaseEntity* target, Vector Start)
{
	if (target) {
		matrix3x4_t matrix[128];

		CBaseEntity* pLocal = (CBaseEntity*)I::ClientEntList->GetClientEntity(I::Engine->GetLocalPlayer());
		if (!pLocal)
			return false;

		CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
		if (!pWeapon)
			return false;

		if (!target->SetupBones(matrix, 128, 0x00000100, 0))
			return false;

		studiohdr_t* studioHdr = I::ModelInfo->GetStudioModel(target->GetModel());

		if (!studioHdr)
			return false;

		mstudiobbox_t* pBox = studioHdr->pHitbox(Vars.Ragebot.Hitbox, 0);

		if (!pBox)
			return false;

		Vector points[8] = { Vector(pBox->bbmin.x, pBox->bbmin.y, pBox->bbmin.z), // 0
			Vector(pBox->bbmin.x, pBox->bbmax.y, pBox->bbmin.z), // 1
			Vector(pBox->bbmax.x, pBox->bbmax.y, pBox->bbmin.z), // 2
			Vector(pBox->bbmax.x, pBox->bbmin.y, pBox->bbmin.z), // 3
			Vector(pBox->bbmax.x, pBox->bbmax.y, pBox->bbmax.z), // 4
			Vector(pBox->bbmin.x, pBox->bbmax.y, pBox->bbmax.z), // 5
			Vector(pBox->bbmin.x, pBox->bbmin.y, pBox->bbmax.z), // 6
			Vector(pBox->bbmax.x, pBox->bbmin.y, pBox->bbmax.z)  // 7
		};

		Vector vMin, vMax;

		VectorTransform(pBox->bbmin, matrix[pBox->bone], vMin);
		VectorTransform(pBox->bbmax, matrix[pBox->bone], vMax);

		Vector vCenter = (vMin + vMax) * 0.5f;
		Start = vCenter;

		if (Vars.Ragebot.MultiPoints)
		{
			int iCount = 7;

			for (int i = 0; i <= iCount; i++)
			{
				Vector vTargetPos;
				switch (i)
				{
				case 0:
				default:
					vTargetPos = vCenter; break;
				case 1:
					vTargetPos = (points[7] + points[1]) * Vars.Ragebot.PointScale; break;
				case 2:
					vTargetPos = (points[3] + points[4]) * Vars.Ragebot.PointScale; break;
				case 3:
					vTargetPos = (points[4] + points[0]) * Vars.Ragebot.PointScale; break;
				case 4:
					vTargetPos = (points[2] + points[7]) * Vars.Ragebot.PointScale; break;
				case 5:
					vTargetPos = (points[6] + points[7]) * Vars.Ragebot.PointScale; break;
				case 6:
					vTargetPos = (points[5] + points[3]) * Vars.Ragebot.PointScale; break;
				case 7:
					vTargetPos = (points[1] + points[2]) * Vars.Ragebot.PointScale; break;
				}

				if (i > 0)
				{
					Vector vReturnPos;
					VectorTransform(vTargetPos, matrix[pBox->bone], vReturnPos);
					Start = vReturnPos;
				}
			}

			return true;
		}
		else
			return true;
	}
	else
		return false;
}

float GetDistance(float* A, float* B)
{
	float dx = A[0] - B[0];
	float dy = A[1] - B[1];
	float dz = A[2] - B[2];

	return (float)sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

void CRageBot::GoToTarget()
{
	bool auto_shoot = false;
	bool can_shoot = true;
	bool reloading = false;

	if (!Vars.Ragebot.Enabled)
		return;

	auto weapon = G::LocalPlayer->GetWeapon();
	float server_time = G::LocalPlayer->GetTickBase() * I::Globals->interval_per_tick;

	float next_shot = weapon->GetNextPrimaryAttack() - server_time;
	if (next_shot > 0 || (Vars.Ragebot.ShootWhenStopped && G::LocalPlayer->GetVelocity().Length() > 60))
		can_shoot = false;

	MultiPoints(m_bestent, G::BestHitboxVec);

	QAngle aim_angle = M::CalcAngle(G::LocalPlayer->GetEyePosition(), G::BestHitboxVec);

	static int chocked = 0;

	switch (Vars.Ragebot.RCS)
	{
	case 0:
		break;
	case 1:
		aim_angle -= G::LocalPlayer->GetPunch() * 2.f;
		break;
	case 2:
		aim_angle -= (G::LocalPlayer->GetPunch() * (2.f + (Vars.Ragebot.RCShow / 50.f)));
		break;
	}

	if (can_shoot)
	{
		if (Vars.Ragebot.Aimstep)
		{
			auto_shoot = AimStep(aim_angle);
			if (auto_shoot)
			{
				G::Aimbotting = true;
				G::UserCmd->viewangles = aim_angle;
			}
		}
		else
		{
			auto_shoot = true;
			G::Aimbotting = true;
			G::UserCmd->viewangles = aim_angle;
		}
	}

	if (!Vars.Ragebot.Silent)
		I::Engine->SetViewAngles(G::UserCmd->viewangles);

	if (Vars.Ragebot.AutoStop)
	{
		if (!Vars.Ragebot.ShootWhenStopped && (!Vars.Ragebot.AutoStopCanShoot || can_shoot))
		{
			if (!Vars.Ragebot.AutoStopFast || G::LocalPlayer->GetVelocity().Length() <= 60)
			{
				oldForward = 0;
				oldSideMove = 0;
				G::UserCmd->upmove = 0;
			}
			else
			{
				oldForward = -oldForward;
				oldSideMove = -oldSideMove;
				G::UserCmd->upmove = 0;
				auto_shoot = false;
			}
		}
	}

	if (Vars.Ragebot.AutoCrouch)
		G::UserCmd->buttons |= IN_DUCK;

	/* Auto Reload */
	if (weapon->Clip() == 0)
	{
		G::UserCmd->buttons &= ~IN_ATTACK;
		G::UserCmd->buttons |= IN_RELOAD;
	}

	if (auto_shoot && can_shoot && Vars.Ragebot.AutoFire && (!Vars.Ragebot.HitChance || HitChance(Vars.Ragebot.HitChanceAmt, G::LocalPlayer->GetWeapon()))) {
		float dist = M::VectorDistance(G::LocalPlayer->GetOrigin(), m_bestent->GetOrigin());

		if (weapon->GetItemDefinitionIndex() == WEAPON_TASER && dist < 800.f)
			G::UserCmd->buttons |= IN_ATTACK;
		else if (weapon->GetItemDefinitionIndex() != WEAPON_TASER)
			G::UserCmd->buttons |= IN_ATTACK;

		G::Aimbotting = true;
	}

	if (weapon->IsSniper() && Vars.Ragebot.AutoScope)
	{
		if (!G::LocalPlayer->IsScoped())
		{
			G::UserCmd->buttons &= ~IN_ATTACK;
			G::UserCmd->buttons |= IN_ZOOM;
		}
	}
}

bool CRageBot::AimStep( QAngle& angle )
{
	if ( !G::Aimbotting )
		AimStepLastAngle = G::UserCmd->viewangles;

	float fov = M::GetFov(AimStepLastAngle, angle);

	G::Aimbotting = fov > Vars.Ragebot.AimstepAmount;

	if (!G::Aimbotting)
		return true;

	QAngle AimStepDelta = AimStepLastAngle - angle;

	if (AimStepDelta.y < 0)
		AimStepLastAngle.y += Vars.Ragebot.AimstepAmount;
	else
		AimStepLastAngle.y -= Vars.Ragebot.AimstepAmount;

	AimStepLastAngle.x = angle.x;
	angle = AimStepLastAngle;
	
	return false;
}

float GenerateRandomFloat(float Min, float Max)
{
	float randomized = (float)rand() / (float)RAND_MAX;
	return Min + randomized * (Max - Min);
}

bool CRageBot::GetHitbox(CBaseEntity* target, Hitbox* hitbox)
{
	matrix3x4a_t matrix[MAXSTUDIOBONES];

	if (!target->SetupBones(matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0))
		return false;


	studiohdr_t *hdr = I::ModelInfo->GetStudioModel(target->GetModel());

	if (!hdr)
		return false;

	mstudiohitboxset_t *hitboxSet = hdr->pHitboxSet(target->GetHitboxSet());
	mstudiobbox_t *untransformedBox = hitboxSet->pHitbox(hitbox->hitbox);

	Vector bbmin = untransformedBox->bbmin;
	Vector bbmax = untransformedBox->bbmax;

	if (untransformedBox->radius != -1.f)
	{
		bbmin -= Vector(untransformedBox->radius, untransformedBox->radius, untransformedBox->radius);
		bbmax += Vector(untransformedBox->radius, untransformedBox->radius, untransformedBox->radius);
	}

	Vector points[] = { ((bbmin + bbmax) * .5f),
		Vector(bbmin.x, bbmin.y, bbmin.z),
		Vector(bbmin.x, bbmax.y, bbmin.z),
		Vector(bbmax.x, bbmax.y, bbmin.z),
		Vector(bbmax.x, bbmin.y, bbmin.z),
		Vector(bbmax.x, bbmax.y, bbmax.z),
		Vector(bbmin.x, bbmax.y, bbmax.z),
		Vector(bbmin.x, bbmin.y, bbmax.z),
		Vector(bbmax.x, bbmin.y, bbmax.z) };


	for (int index = 0; index <= 8; ++index)
	{
		if (index != 0)
			points[index] = ((((points[index] + points[0]) * .5f) + points[index]) * .5f);

		M::VectorTransform(points[index], matrix[untransformedBox->bone], hitbox->points[index]);
	}

	Vector vMin, vMax, vCenter, sCenter;
	VectorTransform(bbmin, matrix[hitbox->hitbox], vMin);
	VectorTransform(bbmax, matrix[hitbox->hitbox], vMax);
	vCenter = (vMin + vMax) * 0.5;

	return true;
}

bool CRageBot::GetBestPoint(CBaseEntity* target, Hitbox* hitbox, BestPoint *point)
{
	Vector center = hitbox->points[0];

	if (hitbox->hitbox == HITBOX_HEAD)
	{
		Vector high = ((hitbox->points[3] + hitbox->points[5]) * .5f);

		float pitch = target->GetEyeAngles().x;
		if ((pitch > 0.f) && (pitch <= 89.f))
		{
			Vector height = (((high - hitbox->points[0]) / 3) * 4);
			Vector new_height = (hitbox->points[0] + (height * (pitch / 89.f)));

			hitbox->points[0] = new_height;
			point->flags |= FL_HIGH;
		}
		else if ((pitch < 292.5f) && (pitch >= 271.f))
		{
			hitbox->points[0] -= Vector(0.f, 0.f, 1.f);
			point->flags |= FL_LOW;
		}
	}

	for (int index = 0; index <= 8; ++index)
	{
		int temp_damage = AutoWall->GetDamagez(hitbox->points[index]);

		if ((point->dmg < temp_damage))
		{
			point->dmg = temp_damage;
			point->point = hitbox->points[index];
			point->index = index;
		}
	}

	return (point->dmg > Vars.Ragebot.AutoWallDmg);
}

// goes through the above hitboxes and selects the best point on the target that will deal the most damage
bool CRageBot::BestAimPointAll( CBaseEntity* target, Vector& hitbox )
{
	/*BestPoint aim_point;

	for( int i = 0; i < 7; i++ )
	{
		Hitbox hitbox( hitboxes[ i ] );
		GetHitbox( target, &hitbox );

		if( !GetBestPoint( target, &hitbox, &aim_point ) )
			continue;
	}

	if( aim_point.dmg > Vars.Ragebot.AutoWallDmg )
	{
		hitbox = aim_point.point;
		return true;
	}
		

	return false;*/
	return false;
}

enum class CSGOHitboxID
{
	Head = 0,
	Neck,
	NeckLower,
	Pelvis,
	Stomach,
	LowerChest,
	Chest,
	UpperChest,
	RightThigh,
	LeftThigh,
	RightShin,
	LeftShin,
	RightFoot,
	LeftFoot,
	RightHand,
	LeftHand,
	RightUpperArm,
	RightLowerArm,
	LeftUpperArm,
	LeftLowerArm
};

// checks multiple points inside the selected hitbox and picks the best point which will deal the most damage
int CRageBot::BestAimPointHitbox(CBaseEntity* target)
{
	return -1;
}

bool CRageBot::EntityIsValid(int i)
{
	CBaseEntity* Entity = I::ClientEntList->GetClientEntity(i);

	player_info_t pInfo;
	I::Engine->GetPlayerInfo(i, &pInfo);

	if (!Entity)
		return false;
	if (Entity == G::LocalPlayer)
		return false;
	if (Entity->GetClientClass()->m_ClassID != 35)
		return false;
	if (!Vars.Ragebot.FriendlyFire)
	{
		if (Entity->GetTeam() == G::LocalPlayer->GetTeam())
			return false;
	}

	if (Entity->GetDormant())
		return false;
	if (Entity->GetImmune())
		return false;
	if (Entity->GetHealth() <= 0)
		return false;

	if (Vars.Ragebot.HitScan == 0)
	{
		if (Vars.Ragebot.AutoWallDmg > AutoWall->GetDamagez(Entity->GetBonePosition(Vars.Ragebot.Hitbox)))
			return false;
	}

	return true;
}
