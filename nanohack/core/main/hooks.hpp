#include <intrin.h>
#include <vector>
#include <shlwapi.h>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment( lib, "shlwapi.lib" )  // needed for the ColorHLSToRGB() function

int _r = 255, _g = 0, _b = 0;

#define ID3_VAL 76561197960265728

#define CALLED_BY(func,off) (reinterpret_cast<std::uint64_t>(_ReturnAddress()) > func && reinterpret_cast<std::uint64_t>(_ReturnAddress()) < func + off)

Vector3 last_pos = Vector3::Zero();

Vector3 last_head_pos = Vector3::Zero();
Vector3 last_neck_pos = Vector3::Zero();
Vector3 last_spine4_pos = Vector3::Zero();
Vector3 last_spine3_pos = Vector3::Zero();
Vector3 last_spine2_pos = Vector3::Zero();
Vector3 last_spine1_pos = Vector3::Zero();
Vector3 last_l_upperarm_pos = Vector3::Zero();
Vector3 last_l_forearm_pos = Vector3::Zero();
Vector3 last_l_hand_pos = Vector3::Zero();
Vector3 last_r_upperarm_pos = Vector3::Zero();
Vector3 last_r_forearm_pos = Vector3::Zero();
Vector3 last_r_hand_pos = Vector3::Zero();
Vector3 last_pelvis_pos = Vector3::Zero();
Vector3 last_l_knee_pos = Vector3::Zero();
Vector3 last_l_foot_pos = Vector3::Zero();
Vector3 last_r_knee_pos = Vector3::Zero();
Vector3 last_r_foot_pos = Vector3::Zero();

//std::map<Projectile*, BasePlayer*> projectiles_with_targets{};
PlayerTick* test;

bool just_joined_server = false;

bool is_noclipping = false;

bool proj = false;
Projectile* projv;

Vector3 GetTrajectoryPoint(Vector3& startingPosition, Vector3 initialVelocity, float timestep, float gravity)
{
	float physicsTimestep = Time::fixedDeltaTime();
	Vector3 stepVelocity = initialVelocity * physicsTimestep;

	//Gravity is already in meters per second, so we need meters per second per second
	Vector3 stepGravity(0, physicsTimestep * physicsTimestep * gravity, 0);

	return startingPosition + (stepVelocity * timestep) + ((stepGravity * (timestep * timestep + timestep)) / 2.0f);;
}

void ClientUpdate_hk(BasePlayer* plly) {
	auto local = LocalPlayer::Entity( );
	if (local) {
		if (aidsware::ui::get_bool(xorstr_("players")) || aidsware::ui::get_bool(xorstr_("sleepers"))) {
			bonecache::cachePlayer(plly);
		}

		if (plly->userID() != LocalPlayer::Entity()->userID()) {
			return plly->ClientUpdate( );
		}

		float speedhack_amount = aidsware::ui::get_float(xorstr_("timescale"));
		if (get_key(aidsware::ui::get_keybind(xorstr_("timescale key"))))
			Time::set_timeScale(speedhack_amount);
		else Time::set_timeScale(1.f);

		if (aidsware::ui::get_bool(xorstr_("spiderman")))
		{
			plly->movement()->groundAngle() = -1.f;
			plly->movement()->groundAngleNew() = -1.f;
		}

		float _p = (aidsware::ui::get_float(xorstr_("max radius")) / 10);

		if (aidsware::ui::get_bool(xorstr_("peek assist")) && target_ply != nullptr && target_ply->isCached() && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible))
			plly->clientTickInterval() = (_p < 0.99f ? 0.99f : _p);//0.99f;
		else
			plly->clientTickInterval() = 0.05f;

		auto held = plly->GetHeldEntity<BaseProjectile>();
		
		//auto held = plly->GetHeldEntity<BaseProjectile>( );
		if (held) {
			if (aidsware::ui::get_bool(xorstr_("no sway")) & held->class_name_hash() != STATIC_CRC32("BaseMelee")) {
				held->aimSway() = 0.f;
				held->aimSwaySpeed() = 0.f;
			}

			if (aidsware::ui::get_bool(xorstr_("automatic")))
				held->automatic() = true;

			if (aidsware::ui::get_bool(xorstr_("fake shots")) && get_key(aidsware::ui::get_keybind(xorstr_("fake shots key"))))
				held->SendSignalBroadcast(BaseEntity::Signal::Attack, xorstr_(""));

			if (aidsware::ui::get_bool(xorstr_("insta charge compound"))
				&& held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon"))
				reinterpret_cast<CompoundBowWeapon*>(held)->currentHoldProgress() = 1.5f;

			if ((aidsware::ui::get_bool(xorstr_("autoshoot")) || settings::alpha::shoot_same_target) || (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible))) {
				if (!held->Empty() && (held->class_name_hash() == STATIC_CRC32("BaseProjectile")
					|| held->class_name_hash() == STATIC_CRC32("BowWeapon")
					|| held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")
					|| held->class_name_hash() == STATIC_CRC32("BaseLauncher")
					|| held->class_name_hash() == STATIC_CRC32("CrossbowWeapon")
					|| held->class_name_hash() == STATIC_CRC32("FlintStrikeWeapon"))) {
					if (target_ply != nullptr && target_ply->isCached()) {
						auto mpv = target_ply->find_mpv_bone();
						Vector3 target;
						if (mpv != nullptr)
							target = mpv->position;
						else
							target = target_ply->bones()->head->position;

						if (LineOfSight(target, plly->eyes()->position()))
							held->DoAttack();
					}
				}
			}
		}
	}

	return plly->ClientUpdate( );
}

void ClientUpdate_Sleeping_hk(BasePlayer* player)
{
	auto local = LocalPlayer::Entity();
	if (local) {
		if (aidsware::ui::get_bool(xorstr_("players")) || aidsware::ui::get_bool(xorstr_("sleepers"))) {
			bonecache::cachePlayer(player);
		}
	}

	return player->ClientUpdate_Sleeping();
}

bool LineCircleIntersection(Vector3 center, float radius, Vector3 rayStart, Vector3 rayEnd, Vector3 direction, float& offset)
{
	Vector2 P(rayStart.x, rayStart.z);
	Vector2 Q(rayEnd.x, rayEnd.z);

	float a = Q.y - P.y;
	float b = P.x - Q.x;
	float c = (a * (P.x) + b * (P.y)) * -1.f;

	float x = center.x;
	float y = center.z;

	float c_x = (b * ((b * x) - (a * y)) - a * c) / (std::pow(a, 2) + std::pow(b, 2));
	float c_y = (a * ((-b * x) + (a * y)) - (b * c)) / (std::pow(a, 2) + std::pow(b, 2));

	Vector2 closestPoint(c_x, c_y);

	float distance = P.distance(Q);

	if (P.distance(closestPoint) > distance || Q.distance(closestPoint) > distance)
	{
		return false;
	}

	if (radius > closestPoint.distance(Vector2(center.x, center.z)))
	{
		Vector2 P(rayStart.x, rayStart.y);
		Vector2 Q(rayEnd.x, rayEnd.y);

		float a = Q.y - P.y;
		float b = P.x - Q.x;
		float c = (a * (P.x) + b * (P.y)) * -1.f;

		float x = center.x;
		float y = center.y;

		float c_x = (b * ((b * x) - (a * y)) - a * c) / (std::pow(a, 2) + std::pow(b, 2));
		float c_y = (a * ((-b * x) + (a * y)) - (b * c)) / (std::pow(a, 2) + std::pow(b, 2));

		Vector2 closestPoint(c_x, c_y);
		if (radius > closestPoint.distance(Vector2(center.x, center.y)))
		{
			return true;
		}
		else
		{
			offset += std::fabs(center.y - closestPoint.y);
			return false;
		}
	}

	return false;
};

Vector3 GetModifiedAimConeDirection_hk(float aimCone, Vector3 inputVec, bool anywhereInside = true) {

	aimCone *= aidsware::ui::get_float(xorstr_("spread %")) / 100.0f;

	printf("target_heli = nullptr?: %s\n", target_heli == nullptr ? "yes" : "no");

	if (aidsware::ui::get_bool(xorstr_("target heli"))
		&& target_heli != nullptr)
	{
		if (get_key(aidsware::ui::get_keybind(xorstr_("psilent key")))
			|| aidsware::ui::get_bool(xorstr_("psilent")))
		{
			inputVec = (aimutils::get_prediction() - LocalPlayer::Entity()->eyes()->position()).normalized();
			return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);
		}
	}

	if (aidsware::ui::get_bool(xorstr_("psilent"))
		&& target_ply)
	{
		auto held = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
		auto mag = held->primaryMagazine();
		if (!mag) return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);
		auto ammo = mag->ammoType();
		if (!ammo) return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);
		auto mod = ammo->GetComponent<ItemModProjectile>(Type::ItemModProjectile());
		if(!mod) return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);
		auto projectile = mod->projectileObject()->Get()->GetComponent<Projectile>(Type::Projectile());

		auto partialTime = projectile->tumbleSpeed();
		auto travel = 0.f;

		if (projectile)
		{
			auto projectileVelocity = mod->projectileVelocity();
			auto projectileVelocityScale = held->projectileVelocityScale();
			projectileVelocityScale = projectileVelocityScale * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.499 : 1.0f);
			float drag = projectile->drag();
			float gravityModifier = projectile->gravityModifier();
			float traveledDistance = projectile->traveledDistance();
			float initialDistance = projectile->initialDistance();
			Vector3 initialVelocity = projectile->initialVelocity();
			float traveledTime = projectile->traveledTime();
			Vector3 previousVel = projectile->previousVelocity();
			Vector3 previousPos = projectile->previousPosition();
			auto gravity = Physics::get_gravity();
			auto deltaTime = Time::fixedDeltaTime();
			auto timescale = Time::timeScale();
			float offset = 0.1;
			int simulations = 0;

			auto mpv = target_ply->find_mpv_bone();
			Vector3 actualTargetPos;
			if (mpv != nullptr)
				actualTargetPos = mpv->position;
			else
				actualTargetPos = target_ply->bones()->head->position;

			Vector3 localPos = LocalPlayer::Entity()->eyes()->get_position();


			if (aidsware::ui::get_bool(xorstr_("fat bullet"))
				&& other::fat_target != Vector3::Zero()
				&& !LineOfSight(localPos, actualTargetPos)) {
				actualTargetPos = other::fat_target;
			}
			else if (aidsware::ui::get_bool(xorstr_("fat bullet"))
				&& other::fat_target == Vector3::Zero()
				&& !LineOfSight(localPos, actualTargetPos))
			{
				for (auto e : ext)
					if (LineOfSight(localPos, actualTargetPos + e))
					{
						actualTargetPos += e;
						break;
					}
			}

			Vector3 targetvel = target_ply->playerModel()->newVelocity();



			while (simulations < 100)
			{
				auto position = localPos;
				auto origin = position;
				float num = deltaTime * timescale;
				float travelTime = 0.0f;
				int num3 = (int)(8.f / num);

				Vector3 targetPosition = actualTargetPos + Vector3(0, offset, 0);
				
				Vector3 vel = Vector3(targetvel.x, 0, targetvel.z);// *0.75;

				auto _aimDir = AimConeUtil::GetModifiedAimConeDirection(0.f, targetPosition - localPos, anywhereInside);
				Vector3 velocity = _aimDir * projectileVelocity * projectileVelocityScale;
				
				//worst movement pred in the history of man
				float x = 0.1f;
				if (localPos.distance(targetPosition) < 50.0f)
					x = 0.05f;
				x += localPos.distance(targetPosition) / 1000.0f;
				Vector3 t = aimutils::SimulateProjectile(position, velocity, x, travelTime, gravity, drag);
				printf("HitTime: %ff\partialTime: %ff\n", travelTime, partialTime);

				if (travelTime > 8.f)
					travelTime = 0.0f;
				
				actualTargetPos += (vel * travelTime);
				
				targetPosition = actualTargetPos + Vector3(0, offset, 0);
				_aimDir = AimConeUtil::GetModifiedAimConeDirection(0.f, targetPosition - localPos, anywhereInside);
				velocity = _aimDir * projectileVelocity * projectileVelocityScale;
				//end of that movement prediction

				for (size_t i = 0; i < num3; i++)
				{
					origin = position;
					position += velocity * num;
					velocity += gravity * gravityModifier * num;
					velocity -= velocity * drag * num;

					if (LineCircleIntersection(actualTargetPos, 0.1f, origin, position, _aimDir, offset))
					{
						printf("Prediction simulated %i times before hit\n", (simulations + 1));
						return _aimDir;
					}
				}
				offset += 0.1f;
				simulations++;
			}
		}
	}

	return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);
}

double CalcBulletDrop(double height, double DepthPlayerTarget, float velocity, float gravity) {
	double pitch = (Vector3::my_atan2(height, DepthPlayerTarget));
	double BulletVelocityXY = velocity * Vector3::my_cos(pitch);
	double Time = DepthPlayerTarget / BulletVelocityXY;
	double TotalVerticalDrop = (0.5f * gravity * Time * Time);
	return TotalVerticalDrop * 10;
}

#define powFFFFFFFFFFFFFFFFFFFFFF(n) (n)*(n)
#define maxSimulateSeconds 5
#define stepRate 0.01666666666

void APrediction(Vector3 local, Vector3& target, float bulletspeed, float gravity, float drag, float& te, float& distance_to_travel) {
	
	float Dist = local.distance(target);
	Vector3 targetvel = target_ply->playerModel()->newVelocity();

	auto base_projectile = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
	if (base_projectile == nullptr)
		return;

	static Type* type = Type::GetType(xorstr_("ItemModProjectile, Assembly-CSharp"));
	if (type == nullptr)
		return;

	auto mag = base_projectile->primaryMagazine();
	if (mag == nullptr)
		return;

	auto ammo = mag->ammoType();
	if (ammo == nullptr)
		return;

	auto itemModProjectile = ammo->GetComponent<ItemModProjectile>(type); // 0x3189118 for getting Projectile* ref
	if (itemModProjectile == nullptr)
		return;

	Projectile* projectile = itemModProjectile->projectileObject()->Get()->GetComponent<Projectile>(Type::Projectile());

	if (projectile == nullptr)
		return;


	float m_flBulletSpeed = (itemModProjectile->projectileVelocity() * (base_projectile->projectileVelocityScale() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.0f)));
	float distance = target.distance(LocalPlayer::Entity()->eyes()->position());
	float bullet_time = distance / m_flBulletSpeed;
	//const float m_flTimeStep = 0.005f;
	const float m_flTimeStep = 0.001f;
	float m_flYTravelled{}, m_flYSpeed{}, m_flBulletTime{}, m_flDivider{};

	//float m_flDistanceTo = fVrom.distance(aimpoint);

	for (distance_to_travel = 0.f; distance_to_travel < distance;)
	{
		//float speed_modifier = (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.0f) - m_flTimeStep * projectile->drag();
		float speed_modifier = 1.0f - m_flTimeStep * projectile->drag();
		m_flBulletSpeed *= speed_modifier;

		if (m_flBulletSpeed <= 0.f || m_flBulletSpeed >= 10000.f || m_flYTravelled >= 10000.f || m_flYTravelled < 0.f)
			break;

		if (m_flBulletTime > 10.f)
			break;

		m_flYSpeed += (9.81f * projectile->gravityModifier()) * m_flTimeStep;
		m_flYSpeed *= speed_modifier;

		distance_to_travel += m_flBulletSpeed * m_flTimeStep;
		m_flYTravelled += m_flYSpeed * m_flTimeStep;
		m_flBulletTime += m_flTimeStep;
	}

	Vector3 velocity = targetvel * 0.75f;
	if (velocity.y > 0.f)
		velocity.y /= 3.25f;

	target.y += m_flYTravelled;
	target += velocity * m_flBulletTime;
	te = m_flBulletTime;
}

Vector3 prev_angle = Vector3::Zero();
float f_travel_time = 0.0f;

bool t_WantsReload = false;
bool t_JustShot = false;
float t_FixedTimeLastShot = -0.1f;
bool t_DidReload = false;

void serverrpc_projectileshoot_hk(int64_t rcx, int64_t rdx, int64_t r9, int64_t ProjectileShoot, int64_t arg5)
{
	uintptr_t pro = 0;
	Vector3 v = LocalPlayer::Entity()->input()->recoilAngles();

	Vector3 r = v - prev_angle;
	//printf("{ %ff, %ff },\n", r.x, r.y);
	prev_angle = v;
	Vector3 vp{};
	Vector3 tp{};
	f_travel_time = 0.0f;

	//good prediction but needed for getmodifiedaimcone not rpc
	while (1)
	{
		if (!ProjectileShoot)
			break;
		if (aidsware::ui::get_bool(xorstr_("target heli"))
			&& target_heli != nullptr)
		{
			break;
		}
		auto loco = LocalPlayer::Entity();
		if (!loco) break;

		auto baseprojectile = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
		if (!baseprojectile) break;
		auto wep_class_name = baseprojectile->class_name(); 

		if (( * (int*)(wep_class_name + 4) == 'eleM' || *(int*)(wep_class_name) == 'ddaP') && (*(int*)(wep_class_name + 4) != 'aepS' || *(int*)(wep_class_name) != 'notS'))
			break;

		/*
		if (baseprojectile->class_name_hash() != STATIC_CRC32("BaseProjectile")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("BowWeapon")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("CompoundBowWeapon")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("BaseLauncher")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("CrossbowWeapon")) {
			break;
		}
		*/
		//uintptr_t projectile_list = safe_read(safe_read(baseprojectile + 0x358, uintptr_t) + 0x10, uintptr_t);
		uintptr_t projectile_list = *reinterpret_cast<uintptr_t*>(
			*reinterpret_cast<uintptr_t*>((uintptr_t)baseprojectile + 0x358) + 0x10); //createdProjectiles;

		//viewMatrix = Camera::getViewMatrix();
		auto camera_pos = loco->eyes()->get_position();//safe_read(il2cpp::field(safe_read(cam, uintptr_t), _("position"), false), Vector3);

		//uintptr_t shoot_list = safe_read(safe_read(ProjectileShoot + 0x18, uintptr_t) + 0x10, uintptr_t);
		uintptr_t shoot_list = *(uintptr_t*)(*(uintptr_t*)(ProjectileShoot + 0x18) + 0x10); //
		auto sz = safe_read(safe_read(ProjectileShoot + 0x18, uintptr_t) + 0x18, int);

		Vector3 aimbot_velocity, aim_angle, rpc_position, target;

		auto info = safe_read(baseprojectile + 0x20, DWORD64);


		auto stats = get_stats(safe_read(info + 0x18, int), baseprojectile);

		auto mpv = target_ply->find_mpv_bone();

		if (mpv != nullptr)
			target = mpv->position;
		else
			target = target_ply->bones()->head->position;

		Vector3 v = loco->eyes()->get_position();
		if (!v.x || !v.y || !v.z)
			v = loco->eyes()->get_position();

		Vector3 bonepos = target;
		Vector3 vel = target_ply->playerModel()->newVelocity();
		
		auto ammo_id = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>()->primaryMagazine()->ammoType()->itemid();

		for (size_t i = 0; i < sz; i++)
		{
			auto projectile = *(uintptr_t*)(shoot_list + 0x20 + i * 0x8); // 
			pro = projectile;
			if (target_ply)
			{
				int id = *reinterpret_cast<int*>(projectile + 0x14);
				if (!map_contains_key(projectile_targets, id))
					projectile_targets.insert(std::make_pair(id, target_ply));
				else
					projectile_targets[id] = target_ply;
			}

			rpc_position = *reinterpret_cast<Vector3*>(projectile + 0x18); //
			vp = rpc_position;
			tp = bonepos;
			auto original_vel = *reinterpret_cast<Vector3*>(projectile + 0x24); //
			//auto itemmod = *reinterpret_cast<uintptr_t*>(projectile + 0xE8); //
			//auto itemmodvel = *reinterpret_cast<float*>(itemmod + 0x34); //
			//auto scale = *reinterpret_cast<float*>(projectile + 0x284); //

			//auto itemmod = safe_read(projectile + 0xE8, uintptr_t);
			//auto itemmodvel = safe_read(itemmod + 0x34, float);
			//(itemModProjectile->projectileVelocity() * base_projectile->projectileVelocityScale());

			if (target_ply/* && !target.teammate*/) { //Vector3 local, Vector3& target, float bulletspeed, float gravity, float drag
				//APrediction(v, bonepos, vel, original_vel.Length(), stats.gravity_modifier, stats.drag, itemmodvel, scale);
				float distance_to_travel;
				if (ammo_id == shotgun || ammo_id == shotgun_slug || ammo_id == shotgun_fire || ammo_id == shotgun_handmade)
					bonepos = aimutils::get_prediction();// - LocalPlayer::Entity()->eyes()->position();
				else
					APrediction(v, bonepos, original_vel.Length(), stats.gravity_modifier, stats.drag, f_travel_time, distance_to_travel);
				

				aim_angle = /*get_aim_angle(rpc_position, target.pos, target.velocity, false, stats)*/bonepos - rpc_position;

				aimbot_velocity = (aim_angle).normalized() * original_vel.Length();

				*reinterpret_cast<Vector3*>(projectile + 0x24) = aimbot_velocity;

				Vector3 localPos = LocalPlayer::Entity()->eyes()->get_position();

				if (aidsware::ui::get_bool(xorstr_("bullet tracers"))) {
					DDraw::Line(rpc_position, bonepos, Color::Color(114, 77, 179, 255), 10.f, false, true);
				}
				//*reinterpret_cast<Vector3*>(projectile + 0x24) = aimbot_velocity;
			}
		}

		auto sz2 = safe_read(projectile_list + 0x18, int);


		for (int i = 0; i < sz2; i++)
		{
			auto projectile = *(uintptr_t*)((uintptr_t)projectile_list + 0x20 + i * 0x8);

			if (!projectile)
				continue;

			if (aidsware::ui::get_bool(xorstr_("psilent")) || (settings::alpha::shoot_same_target && target_ply->userID() == entities::target_id && target_ply->is_visible())) {
				if (target_ply) {
					//safe_write(projectile + 0x118, aimbot_velocity, Vector3);
					//p->currentVelocity() = aimbot_velocity;
				}
			}
		}


		break;
	}

	reinterpret_cast<void (*)(int64_t, int64_t, int64_t, int64_t, int64_t)>(settings::serverrpc_projectileshoot)(rcx, rdx, r9, ProjectileShoot, arg5);
	
	if (aidsware::ui::get_bool(xorstr_("always reload")))
	{
		t_WantsReload = false;
		t_JustShot = true;
		//t_TimeSinceLastShot = Time::fixedTime() - t_FixedTimeLastShot;
		t_FixedTimeLastShot = Time::fixedTime();
		t_DidReload = false;
	}

	return;
}

Attack* BuildAttackMessage_hk(HitTest* self) {
	auto ret = self->BuildAttackMessage( );
	auto entity = BaseNetworkable::clientEntities()->Find<BasePlayer*>(ret->hitID());

	if (aidsware::ui::get_bool(xorstr_("always heli weakspot")))
	{
		if (entity->class_name_hash() == STATIC_CRC32("BaseHelicopter"))
		{
			if (entity->health() <= 5000.0f)
				ret->hitBone() = StringPool::Get(xorstr_("tail_rotor_col"));
			else
				ret->hitBone() = StringPool::Get(xorstr_("engine_col"));
		}
	}

	auto localPlayer = LocalPlayer::Entity( );
	if (localPlayer) {
		if (reinterpret_cast<BasePlayer*>(self->ignoreEntity( ))->userID( ) == localPlayer->userID( )) { // isAuthoritative

			if (entity) {
				if (entity->IsPlayer( )) {
					if (entity->isCached( )) {
						if (localPlayer->isCached( )) {
							// trajectory_end = 1 meter
							// player_distance = 0.2 meter
							// profit $$$$$$


							if (aidsware::ui::get_bool(xorstr_("hitbox attraction"))) {
								auto bone = entity->model( )->find_bone(ret->hitPositionWorld( ));
								if (bone.second) {
									ret->hitPositionWorld( ) = bone.first->position( );
								}
							}
							if (aidsware::ui::get_combobox(xorstr_("hitbox override")) != 0) {
								if (aidsware::ui::get_combobox(xorstr_("hitbox override")) == 1)
									ret->hitBone( ) = StringPool::Get(xorstr_("spine4"));
								else if (aidsware::ui::get_combobox(xorstr_("hitbox override")) == 2)
									ret->hitBone( ) = StringPool::Get(xorstr_("head"));
								else if (aidsware::ui::get_combobox(xorstr_("hitbox override")) == 3) {
									int num = rand( ) % 100;
									if (num > 90)
										ret->hitBone( ) = StringPool::Get(xorstr_("head"));
									else if (num < 90 && num > 80)
										ret->hitBone( ) = StringPool::Get(xorstr_("neck"));
									else if (num < 80 && num > 70)
										ret->hitBone( ) = StringPool::Get(xorstr_("l_clavicle"));
									else if (num < 70 && num > 60)
										ret->hitBone( ) = StringPool::Get(xorstr_("pelvis"));
									else if (num < 60 && num > 50)
										ret->hitBone( ) = StringPool::Get(xorstr_("r_hip"));
									else if (num < 50 && num > 40)
										ret->hitBone( ) = StringPool::Get(xorstr_("r_foot"));
									else if (num < 40 && num > 30)
										ret->hitBone( ) = StringPool::Get(xorstr_("spine1"));
									else if (num < 30 && num > 20)
										ret->hitBone( ) = StringPool::Get(xorstr_("l_hand"));
									else if (num < 20 && num > 10)
										ret->hitBone( ) = StringPool::Get(xorstr_("r_upperarm"));
									else if (num < 10)
										ret->hitBone( ) = StringPool::Get(xorstr_("l_knee"));
									else
										ret->hitBone( ) = StringPool::Get(xorstr_("spine4"));
								}
								else if (aidsware::ui::get_combobox(xorstr_("hitbox override")) == 4) {
									int yeet = rand( ) % 100;
									if (yeet > 50)
										ret->hitBone( ) = StringPool::Get(xorstr_("head"));
									else
										ret->hitBone( ) = StringPool::Get(xorstr_("spine4"));
								}
							}
						}
					}
				}
			}
		}
	}

	return ret;
}

void DoAttack_hk(FlintStrikeWeapon* weapon) {
	if (aidsware::ui::get_bool(xorstr_("insta eoka")))
		weapon->_didSparkThisFrame( ) = true;

	return weapon->DoAttack( );
}

Vector3 BodyLeanOffset_hk(PlayerEyes* a1) {
	if (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible)) {
		if (target_ply != nullptr) {
			if (other::m_manipulate.empty( ) || !LocalPlayer::Entity( )->GetHeldEntity( ))
				return a1->BodyLeanOffset( );

			return other::m_manipulate;
		}
	}

	return a1->BodyLeanOffset( );
}

void DoFirstPersonCamera_hk(PlayerEyes* a1, Component* cam) {
	a1->DoFirstPersonCamera(cam);

	if (!projv || !a1 || !cam) return;

	if (aidsware::ui::get_bool(xorstr_("follow projectile"))
		&& proj)
	{
		Vector3 p = projv->currentPosition();
		cam->transform()->set_position(Vector3(p.x, p.y + 0.1f, p.z));
	}
	/* //Visualize peek assist?
	else if (aidsware::ui::get_bool(xorstr_("peek assist"))
		&& !proj) {
		Vector3 re_p = LocalPlayer::Entity( )->transform( )->position( ) + LocalPlayer::Entity( )->transform( )->up( ) * (PlayerEyes::EyeOffset( ).y + LocalPlayer::Entity( )->eyes( )->viewOffset( ).y);
		cam->transform( )->set_position(re_p);
	}
	*/
}


void pa(BaseMelee* self, HitTest* hit)
{
	__try
	{
		auto entity = hit->HitEntity();
		if (entity->class_name_hash() == STATIC_CRC32("OreResourceEntity")) {
			BaseNetworkable* marker = BaseNetworkable::clientEntities()->FindClosest(STATIC_CRC32("OreHotSpot"), entity, 5.0f);
			if (marker) {
				entity = marker;
				hit->HitTransform() = marker->transform();
				hit->HitPoint() = marker->transform()->InverseTransformPoint(marker->transform()->position());
				hit->HitMaterial() = String::New(xorstr_("MetalOre"));
			}
		}
		else if (entity->class_name_hash() == STATIC_CRC32("TreeEntity")) {
			BaseNetworkable* marker = BaseNetworkable::clientEntities()->FindClosest(STATIC_CRC32("TreeMarker"), entity, 5.0f);
			if (marker) {
				hit->HitTransform() = marker->transform();
				hit->HitPoint() = marker->transform()->InverseTransformPoint(marker->transform()->position());
				hit->HitMaterial() = String::New(xorstr_("Wood"));
			}
		}
	}
	__except (true)
	{
		printf(xorstr_("Exception occured in %s\n"), __FUNCTION__);
	}
}

void ProcessAttack_hk(BaseMelee* self, HitTest* hit) {
	if (!self || !hit) return;
	auto entity = hit->HitEntity();
	if (!entity) return self->ProcessAttack(hit);

	if (!aidsware::ui::get_bool(xorstr_("farm assist")) || !entity)
		return self->ProcessAttack(hit);

	pa(self, hit);

	return self->ProcessAttack(hit);
}

bool CanAttack_hk(BasePlayer* self) {
	if (aidsware::ui::get_bool(xorstr_("can hold items")))
		return true;

	return self->CanAttack( );
}

bool bp_flying = false;
int flyhack_state = 0;
float state_time = 0.0f;
Vector3 last_wall_pos = Vector3::Zero();
Vector3 head_last_bp = Vector3::Zero();
Vector3 temp_vel = Vector3::Zero();

struct NodeTarget {
	Vector3 pos;
	int steps;
	std::vector<Vector3> path;
	BaseEntity* ent;
};
NodeTarget node;

Vector3 lowest_pos(Vector3 in)
{
	Vector3 current = in;
	for (size_t i = 0; i < 100; i++)
	{
		if (LineOfSight(in, current))
		{
			current = Vector3(current.x, current.z -= 1.f, current.z);
			continue;
		}
		else break;
	}
	return Vector3(current.x, current.y += 1.6f, current.z);
}

float dfg(Vector3 v)
{
	Vector3 p = v;
	int t = 0;
	while (t++ < 100)
	{
		if (!LineOfSight(v, p))
			return v.distance(p);
		p.y -= 0.1;
	}
}

void CreatePath(Vector3 start)
{
	node.pos = start;
	node.steps = 1;
	//create path
	std::vector<Vector3> path;
	Vector3 point = LocalPlayer::Entity()->eyes()->transform()->position();
	Vector3 original = point;
	bool failed = false;
	Vector3 old_point = point;
	float control = 0.f;
	int iterations = 0;
	while (point.distance(node.pos) > 0.5f)
	{
		if (iterations++ > 10000)
			break;


		path.push_back(point);
		Vector3 new_point = lowest_pos(Vector3_::MoveTowards(point, node.pos, 1.0f));

		bool flag = false;


		if (GamePhysics::LineOfSightRadius(point, new_point, 10551296, 1.5f, 0.f))
		{
			old_point = point;
			point = new_point;
		}
		else
		{
			std::vector<Vector3> ps = {};

			for (auto e : ext) //create sphere if cannot find LOS straight ahead
				if ((GamePhysics::LineOfSightRadius(point, point + e, 10551296, 1.5f, 0.f) && //0.5 for margin next to walls
					GamePhysics::LineOfSightRadius(point + e, point, 10551296, 1.5f, 0.f) &&
					GamePhysics::LineOfSightRadius(Vector3((point + e).x, (point + e).y + 1000, (point + e).z), point + e, 10551296, 1.5f, 0.f) &&
					GamePhysics::LineOfSightRadius(point + e, Vector3((point + e).x, (point + e).y + 1000, (point + e).z), 10551296, 1.5f, 0.f))
					&& (point + e).distance(node.pos) < point.distance(node.pos)
					&& (point + e).distance(point) > 0.99f)
				{
					if (flag) continue;
					bool y = false;
					for (auto z : node.path) //check if new path passes by any points
						if ((point + e).distance(z) < 0.99f)
							y = true;
					if (y) continue;
					ps.push_back(point + e);
				}
			Vector3 best = Vector3::Zero();
			for (auto e : ps)
				if (e.distance(node.pos) < best.distance(node.pos)
					&& dfg(e) < 1.6f)
					best = e;
			old_point = point;
			point = best;
		}
	}
	node.path = path;
}

void do_infjump(PlayerWalkMovement* a1, ModelState* state)
{
	__try
	{
		a1->grounded() = (a1->climbing() = (a1->sliding() = false));
		state->set_ducked(false);
		a1->jumping() = true;
		state->set_jumped(true);
		a1->jumpTime() = Time::time();
		a1->ladder() = nullptr;

		Vector3 curVel = a1->body()->velocity();
		a1->body()->set_velocity({ curVel.x, 10, curVel.z });
	}
	__except (true)
	{
		printf("Exception occured in %s\n", __FUNCTION__);
	}
}


int psteps = 0;
bool needs_jump = false;
void do_autofarm(Vector3& vel)
{
	__try {
		auto lp = LocalPlayer::Entity();
		float speed = (lp->movement()->swimming() || lp->movement()->Ducking() > 0.5) ? 1.7f : 5.5f;

		auto hotspot = BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("OreHotSpot"), lp, 5.f);
		auto marker = (hotspot ? hotspot : BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("OreResourceEntity"), lp, 400.0f));

		auto treespot = BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("TreeEntity"), lp, 25.f);
		
		bool is_tree = false;
		if (treespot)
			if (treespot->transform()->position().distance(lp->eyes()->get_position())
				< marker->transform()->position().distance(lp->eyes()->get_position()))
			{
				marker = treespot;
				is_tree = true;
			}

		//auto marker = entities::closest_node;
		node.ent = marker;
		//auto marker = entities::closest_node;

		if (marker)
		{
			if (node.steps > 0
				&& lp->transform()->position().distance(node.pos) < 0.5f)
			{
				node.path.clear();
				node.pos = Vector3::Zero();
				node.steps = 0;
				vel = Vector3::Zero();
			}

			if (marker->transform())
			{
				if (lp->transform()->position().distance(node.pos) > 0.5f)
				{
					if (node.path.empty() && (node.pos.empty() || node.pos == Vector3::Zero())
						&& lp->transform()->position().distance(node.pos) > 1.f)
						CreatePath(marker->transform()->position());

					Vector3 current_step = node.path[node.steps];
					psteps = node.steps;
					if (current_step.distance(node.pos) <= 0.5f)
					{
						vel = Vector3::Zero();
						node.path.clear();
						node.pos = Vector3::Zero();
						node.steps = 0;
						return;
					}

					//draw path
					if (!node.path.empty())
					{
						for (size_t i = 1; i < node.path.size(); i++)
						{
							if (node.path[i] == current_step)
								DDraw::Line(node.path[i - 1], node.path[i], Color::Color(1, 0, 0, 50), 0.02f, false, true);
							else
								DDraw::Line(node.path[i - 1], node.path[i], Color::Color(_r, _g, _b, 50), 0.02f, false, true);
						}
					}

					if (lp->transform()->position().distance(current_step) < 1.6f)
						node.steps += 1;

					if (node.steps >= node.path.size() - 1)
					{
						vel = Vector3::Zero();
						node.path.clear();
						node.pos = Vector3::Zero();
						node.steps = 0;
						return;
					}

					Vector3 dir = ((Vector3(current_step.x, current_step.y - dfg(current_step) + 0.1f, current_step.z)) - lp->transform()->position()).normalized();
					vel = { (dir.x / dir.length() * speed), vel.y, (dir.z / dir.length() * speed) };

					if (node.path[node.steps].y - lp->transform()->position().y > 1.6f)
					{
						needs_jump = true;
						do_infjump(lp->movement(), lp->modelState());
					}
				}
				else {
					vel = Vector3::Zero();
					node.path.clear();
					node.pos = Vector3::Zero();
					node.steps = 0;
				}
			}
		}
	}
	__except (true)
	{
		printf("Exception occured in %s\n", xorstr_(__FUNCTION__));
	}
}


void UpdateVelocity_hk(PlayerWalkMovement* self) {
	if (aidsware::ui::get_bool(xorstr_("walk to marker"))
		|| (settings::alpha::walk_to_pos && entities::walk_to_pos != Vector3::Zero())
		|| (settings::alpha::follow_master && entities::master))
	{
		float speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
		MapNote* m = LocalPlayer::Entity()->ClientCurrentMapNote();
		if (m || settings::alpha::walk_to_pos || settings::alpha::follow_master)
		{
			Vector3 pos = LocalPlayer::Entity()->transform()->position();
			Vector3 marker_pos = settings::alpha::walk_to_pos ? entities::walk_to_pos : m->worldPosition();
			if (settings::alpha::follow_master)
				marker_pos = entities::master->transform()->position();
			printf("marker_pos: (%ff, %ff, %ff)\n", marker_pos.x, marker_pos.y, marker_pos.z);
			Vector3 dir = (marker_pos - pos).normalized();
			self->TargetMovement() = { (dir.x / dir.length() * speed), dir.y,(dir.z / dir.length() * speed) };
		}
	}

	Vector3 vel = self->TargetMovement();

	if (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible)) {
		float max_speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
		if (vel.length() > 0.f) {
			//self->TargetMovement( ) = Vector3::Zero( );gh
		}
	}

	if (!self->flying()) {
		if (aidsware::ui::get_bool(xorstr_("omnisprint"))) {
			float max_speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
			if (vel.length() > 0.f) {
				Vector3 target_vel = Vector3(vel.x / vel.length() * max_speed, vel.y, vel.z / vel.length() * max_speed);
				self->TargetMovement() = target_vel;
				vel = target_vel;
			}
		}
	}

	if (aidsware::ui::get_bool(xorstr_("auto farm")))
	{
		do_autofarm(vel);
	}
	else
	{
		node.path.clear();
		node.pos = Vector3::Zero();
		node.steps = 0;
	}

	if (aidsware::ui::get_bool(xorstr_("flyhack stop")))
	{
		if (settings::hor_flyhack >= 5.f)
		{
			vel.x = 0.0f;
			vel.z = 0.0f;
		}
		if (settings::flyhack >= 3.0f)
		{
			vel.y = 0.0f;
		}
	}

	if (get_key(aidsware::ui::get_keybind(xorstr_("climb bypass"))))
	{

		if (!bp_flying)
		{
			if (is_noclipping)
				bp_flying = true;
			else
			{
				ConsoleSystem::Run(ConsoleSystem::Option::Client(), String::New(xorstr_("noclip")), nullptr);
				bp_flying = true;
			}
		}
		else
		{
			switch (flyhack_state)
			{
			case 0: //forward state
			{
				if (state_time == 0.0f)
				{
					auto rot = LocalPlayer::Entity()->eyes()->rotation();
					Vector3 dir = rot * Vector3(0, 0, 1);
					if(temp_vel == Vector3::Zero()) 
						temp_vel = { (dir.x / dir.length() * 7.5f), 0,(dir.z / dir.length() * 7.5f) };

					state_time = Time::fixedTime();
					break;
				}
				if (Time::fixedTime() > (state_time + 2.0f))
				{
					state_time = 0.0f;
					flyhack_state = 1;
					break;
				}
				if (Time::fixedTime() < (state_time + 2.0f))
				{
					vel = temp_vel;
					break;
				}
				break;
			}
			case 1: //small backward state
			{
				if (state_time == 0.0f)
				{
					state_time = Time::fixedTime();
				}
				if (Time::fixedTime() > (state_time + 0.1f))
				{
					state_time = 0.0f;
					flyhack_state = 2;
				}
				if (Time::fixedTime() < (state_time + 0.1f))
				{
					vel = { -(temp_vel.x * 0.5f), 0, -(temp_vel.z * 0.5f) };
					break;
				}
				break;
			}
			case 2: //upwards state		
			{
				if (state_time == 0.0f)
				{
					head_last_bp = LocalPlayer::Entity()->eyes()->get_position();
					state_time = Time::fixedTime();
					break;
				}
				if (LocalPlayer::Entity()->eyes()->get_position().y >= head_last_bp.y + 2.f)
				{	
					state_time = 0.0f;
					flyhack_state = 0;
					break;
				}
				if (LocalPlayer::Entity()->eyes()->get_position().y < head_last_bp.y + 2.f)
				{
					auto rot = LocalPlayer::Entity()->eyes()->rotation();
					Vector3 dir = rot * Vector3(0, 0, 1);
					vel = { 0, (dir.y + 1) * 5.f, 0 };

					break;
				}
				break;
			}

			if (settings::flyhack > 1.0f || settings::hor_flyhack > 1.0f)
			{
				state_time = 0.0f;
				flyhack_state = 0;
			}
			}
		}

	}
	else
	{
		flyhack_state = 0;
		temp_vel = Vector3::Zero();
	}

	self->TargetMovement() = vel;
	return self->UpdateVelocity( );
}

Vector3 EyePositionForPlayer_hk(BaseMountable* mount, BasePlayer* player, Quaternion lookRot) {
	if (player->userID( ) == LocalPlayer::Entity( )->userID( )) {
		if (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible)) {
			return mount->EyePositionForPlayer(player, lookRot) + other::m_manipulate;
		}
	}

	return mount->EyePositionForPlayer(player, lookRot);
}

void HandleJumping_hk(PlayerWalkMovement* a1, ModelState* state, bool wantsJump, bool jumpInDirection = false) {
	
	if (aidsware::ui::get_bool(xorstr_("flyhack stop")))
	{
		float threshold = aidsware::ui::get_float(xorstr_("threshold"));
		if (settings::hor_flyhack >= 5.0f
			|| settings::flyhack >= 2.9f)
			return;
	}
	wantsJump == needs_jump;
	if (aidsware::ui::get_bool(xorstr_("infinite jump"))
		|| aidsware::ui::get_bool(xorstr_("auto farm"))) {
		if (!wantsJump)
			return;
		do_infjump(a1, state);
		return;
	}
	return a1->HandleJumping(state, wantsJump, jumpInDirection);
}

void OnLand_hk(BasePlayer* ply, float vel) {
	if (settings::suicide == true)
	{
		ply->OnLand(1000.0f);
		return;
	}
	if (!aidsware::ui::get_bool(xorstr_("no fall")))
		ply->OnLand(vel);
}

bool IsDown_hk(InputState* self, BUTTON btn) {
	if ((aidsware::ui::get_bool(xorstr_("autoshoot")) || (settings::alpha::shoot_same_target && target_ply->userID() == entities::target_id && target_ply->is_visible())) || (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible))) {
		if (btn == BUTTON::FIRE_SECONDARY)
		{
			auto held = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
			if (held->class_name_hash() == STATIC_CRC32("BowWeapon") ||
				held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")) {
				if (held->Empty()) {
					return true;
				}
			}
		}

		if (btn == BUTTON::JUMP && aidsware::ui::get_bool(xorstr_("auto farm")) && needs_jump) {
			needs_jump = false;
			return true;
		}
		if (btn == BUTTON::FIRE_PRIMARY) {
			auto held = LocalPlayer::Entity( )->GetHeldEntity<BaseProjectile>( );
			if (held && !held->Empty( ) && (held->class_name_hash() == STATIC_CRC32("BaseProjectile")
											|| held->class_name_hash() == STATIC_CRC32("BowWeapon")
											|| held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")
											|| held->class_name_hash() == STATIC_CRC32("BaseLauncher")
											|| held->class_name_hash() == STATIC_CRC32("FlintStrikeWeapon")
											|| held->class_name_hash() == STATIC_CRC32("CrossbowWeapon"))) {
				if (target_ply != nullptr && target_ply->isCached( )) {
					auto mpv = target_ply->find_mpv_bone( );
					Vector3 target;
					if (mpv != nullptr)
						target = mpv->position;
					else
						target = target_ply->bones( )->head->position;

					if (LineOfSight(target, LocalPlayer::Entity( )->eyes( )->position( )))
						return true;
				}
			}
		}
	}

	return self->IsDown(btn);
}

void OnAttacked_hk(BaseCombatEntity* self, HitInfo* info) {
	self->OnAttacked(info);

	/*if (self->class_name_hash( ) == STATIC_CRC32("BasePlayer")) {
		if (info->Initiator( ) == LocalPlayer::Entity( )) {
			auto entity = reinterpret_cast<BasePlayer*>(self);

			if(std::filesystem::exists(settings::data_dir + xorstr_("\\hs.wav")))
				play_sound((settings::data_dir + xorstr_("\\hs.wav")).c_str());
		}
	}*/

}

void AIMBOTPrediction(Vector3 local, Vector3& target, Vector3 targetvel, float bulletspeed, float gravity) {
	float Dist = target.distance(local);
	float BulletTime = Dist / bulletspeed;
	Vector3 vel = Vector3(targetvel.x, 0, targetvel.z) * 0.75f;
	Vector3 PredictVel = vel * BulletTime;
	target += PredictVel;
	double height = target.y - local.y;
	Vector3 dir = target - local;
	float DepthPlayerTarget = Vector3::my_sqrt((dir.x * dir.x) + (dir.z * dir.z));
	float drop = CalcBulletDrop(height, DepthPlayerTarget, bulletspeed, gravity);
	target.y += drop;
}

Vector3 get_aim_point(float speed, float gravity) {
	Vector3 ret;
	auto mpv = target_ply->find_mpv_bone();
	if (mpv != nullptr)
		ret = mpv->position;
	else
		ret = target_ply->bones()->head->position;
	AIMBOTPrediction(LocalPlayer::Entity()->eyes()->get_position(), ret, target_ply->playerModel()->newVelocity(), speed, gravity);
	return ret;
}

Vector2 CalcAngleNew(const Vector3& Src, const Vector3& Dst) {
	Vector3 dir = Src - Dst;
	Vector2 ret = Vector2{ RAD2DEG(std::asin(dir.y / dir.Length())), RAD2DEG(-std::atan2(dir.x, -dir.z)) };
	return ret;
}

void Normalize(float& Yaw, float& Pitch) {
	if (Pitch < -89) Pitch = -89;
	else if (Pitch > 89) Pitch = 89;
	if (Yaw < -999) Yaw += 999;
	else if (Yaw > 999) Yaw -= 999;
}

void StepConstant(Vector2& angles) {
	bool smooth = true;
	Vector3 v = LocalPlayer::Entity()->input()->bodyAngles();
	Vector2 va = { v.x, v.y };
	Vector2 angles_step = angles - va;
	Normalize(angles_step.x, angles_step.y);

	if (smooth) {
		float factor_pitch = (aidsware::ui::get_float(xorstr_("smoothing")));
		if (angles_step.x < 0.f) {
			if (factor_pitch > std::fabs(angles_step.x)) {
				factor_pitch = std::fabs(angles_step.x);
			}
			angles.x = va.x - factor_pitch;
		}
		else {
			if (factor_pitch > angles_step.x) {
				factor_pitch = angles_step.x;
			}
			angles.x = va.x + factor_pitch;
		}
	}
	if (smooth) {
		float factor_yaw = (aidsware::ui::get_float(xorstr_("smoothing")));
		if (angles_step.y < 0.f) {
			if (factor_yaw > std::fabs(angles_step.y)) {
				factor_yaw = std::fabs(angles_step.y);
			}
			angles.y = va.y - factor_yaw;
		}
		else {
			if (factor_yaw > angles_step.y) {
				factor_yaw = angles_step.y;
			}
			angles.y = va.y + factor_yaw;
		}
	}
}

void DoAimbot()
{
	BasePlayer* lp = LocalPlayer::Entity();
	if (!lp) return;
	auto held = lp->GetHeldEntity<BaseProjectile>();
	if (!held) return;
	if (!target_ply) return;

	Vector3 local = lp->eyes()->get_position();
	Vector3 target;
	auto mpv = target_ply->find_mpv_bone();
	if (mpv != nullptr)
		target = mpv->position;
	else
		target = target_ply->bones()->head->position;

	if (aidsware::ui::get_bool(xorstr_("bodyaim")))
		target = target_ply->bones()->spine1->position;
	
	auto info = safe_read(held + 0x20, DWORD64);
	auto stats = get_stats(safe_read(info + 0x18, int), held);

	auto mag = held->primaryMagazine();
	auto ammo = mag->ammoType();
	auto ammo_id = ammo->itemid();
	static Type* type = Type::GetType(xorstr_("ItemModProjectile, Assembly-CSharp"));
	auto itemModProjectile = ammo->GetComponent<ItemModProjectile>(type); // 0x3189118 for getting Projectile* ref
	float m_flBulletSpeed = (itemModProjectile->projectileVelocity() * (held->projectileVelocityScale() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.0f)));

	AIMBOTPrediction(local, target, lp->playerModel()->newVelocity(), m_flBulletSpeed, stats.gravity_modifier);

	//first bodyangles then headangles then recoilangles
	Vector3 v = lp->input()->bodyAngles();
	Vector2 va = { v.x, v.y };
	Vector2 offset = CalcAngleNew(local, target) - va;
	Normalize(offset.x, offset.y);
	Vector2 angleToAim = va + offset;
	StepConstant(angleToAim);
	StepConstant(angleToAim);
	Normalize(angleToAim.x, angleToAim.y);
	Vector3 a = { angleToAim.x, angleToAim.y, 0.0f };
	lp->input()->bodyAngles() = a;
	//lp->input()->headAngles() = a;
}

bool debugcam = false;
bool has_intialized_methods = false;

bool r_ = false, g_ = false, b_ = false;

float hue = 0, lum = 60, sat = 240;

int server_fd = 0;

void send_command(char buffer[1024])
{
	char temp[1024];
	memset(temp, '\x00', 1024 * sizeof(*temp));
	entities::slave t = entities::slaves[entities::alpha_index];

	temp[0] = '\xC2';
	for (size_t i = 1; i < 1024; i++)
	{
		if ((i - 1) > t.steam_id.size())
			temp[i] = buffer[i - t.steam_id.size() - 1];
		else
			temp[i] = t.steam_id[i - 1];
	}

	std::string packet(temp); packet += '\x99';
	packet += std::string(buffer);

	memset(buffer, '\x00', 1024 * sizeof(*buffer));

	for (size_t i = 0; i < 1024; i++)
	{
		if (i > packet.size()) break;
		buffer[i] = packet[i];
	}

	send(server_fd, buffer, 1024, 0);
}

void update_slave(std::string name)
{
	char buffer[1024];
	std::string f_un = std::string(settings::auth::username.begin(), settings::auth::username.end());
	std::string _m = '\xA2' + f_un + '\x99' + name + '\x99' + settings::steamid + '\x99' + settings::current_server + '\x99';
	for (size_t i = 0; i < _m.size(); i++)
		buffer[i] = _m[i];
	send(server_fd, buffer, 1024, 0);
}

BasePlayer* last_target = nullptr;
Vector3 last_marker = Vector3::Zero();

void do_light(float s, float a, float amb = 1.f)
{
	__try {
		auto list = TOD_Sky::instances();
		if (list) {
			for (int j = 0; j < list->size; j++) {
				auto sky = (TOD_Sky*)list->get(j);
				if (!sky)
					continue;

				sky->Day()->AmbientMultiplier() = amb == 4.f ? 0.2f : 1.f;
				sky->Night()->AmbientMultiplier() = amb;

				sky->Stars()->Brightness() = s;
				sky->Atmosphere()->RayleighMultiplier() = a;
				//sky->Atmosphere()->Fogginess() = aidsware::ui::get_float(xorstr_("atmosphere"));
			}
		}
	}
	__except (true)
	{
		printf("Error occured inside do_light!\n");
		return;
	}
}

float time_last_upgrade = 0.f;
void ClientInput_hk(BasePlayer* plly, uintptr_t state) {

	if (!plly)
		return plly->ClientInput(state);

	if (!has_intialized_methods) {
		auto il2cpp_codegen_initialize_method = reinterpret_cast<void (*)(unsigned int)>(settings::il_init_methods);

		for (int i = 0; i <
			56204 //56229 for real rust
			; i++) {
			il2cpp_codegen_initialize_method(i);
		}
		has_intialized_methods = true;
	}

	static uintptr_t* serverrpc_projecshoot;
	if (!serverrpc_projecshoot) {
		auto method_serverrpc_projecshoot = *reinterpret_cast<uintptr_t*>(settings::serverrpc_projectileshoot);

		if (method_serverrpc_projecshoot) {
			serverrpc_projecshoot = **(uintptr_t***)(method_serverrpc_projecshoot + 0x30);

			settings::serverrpc_projectileshoot = *serverrpc_projecshoot;

			*serverrpc_projecshoot = reinterpret_cast<uintptr_t>(&serverrpc_projectileshoot_hk);
		}
	}

	if (just_joined_server)
	{
		std::wstring w(LocalPlayer::Entity()->_displayName());
		std::string s(w.begin(), w.end());
		update_slave(s);
		just_joined_server = false;
	}

	if (plly->userID() == LocalPlayer::Entity()->userID()) {
		auto held = plly->GetHeldEntity<BaseProjectile>();

		GLOBAL_TIME = Time::time();
		float desyncTime = (Time::realtimeSinceStartup() - plly->lastSentTickTime()) - 0.03125 * 3;
		settings::tr::desync_time = desyncTime;


		auto mpv = target_ply->find_mpv_bone();
		Vector3 target;
		if (mpv != nullptr)
			target = mpv->position;
		else
			target = target_ply->bones()->head->position;

		if (aidsware::ui::get_bool(xorstr_("rapid fire"))) {
			if (held)
				held->repeatDelay() = 0.07f;
		}

		if (get_key(aidsware::ui::get_keybind(xorstr_("desync on key"))))
			LocalPlayer::Entity()->clientTickInterval() = 0.99f;

		if (hue > 240.0f) hue = 0.0f;
		hue += 0.3f;
		COLORREF rgb = ::ColorHLSToRGB(hue, lum, sat);

		_r = GetRValue(rgb);
		_g = GetGValue(rgb);
		_b = GetBValue(rgb);

		if (aidsware::ui::get_bool(xorstr_("show desync")))
		{
			//DDraw::Line(localPlayer->eyes( )->get_position( ), ret->hitPositionWorld( ), Color(1, 0, 0, 1), 0.05f, false, true);
			DDraw::Sphere(last_head_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //head
			DDraw::Line(last_head_pos, last_neck_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Line(last_neck_pos, last_spine4_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_spine4_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.05f, false); //
			DDraw::Line(last_spine4_pos, last_spine1_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);
			DDraw::Line(last_spine4_pos, last_l_upperarm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);
			DDraw::Line(last_spine4_pos, last_r_upperarm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_spine1_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_spine1_pos, last_pelvis_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);
			DDraw::Line(last_spine1_pos, last_l_upperarm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);
			DDraw::Line(last_spine1_pos, last_r_upperarm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_l_upperarm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_l_upperarm_pos, last_l_forearm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_l_forearm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_l_forearm_pos, last_l_hand_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_l_hand_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //

			DDraw::Sphere(last_r_upperarm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_r_upperarm_pos, last_r_forearm_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_r_forearm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_r_forearm_pos, last_r_hand_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_r_hand_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //

			DDraw::Sphere(last_pelvis_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_pelvis_pos, last_l_knee_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);
			DDraw::Line(last_pelvis_pos, last_r_knee_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_l_knee_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_l_knee_pos, last_l_foot_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_l_foot_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //

			DDraw::Sphere(last_r_knee_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
			DDraw::Line(last_r_knee_pos, last_r_foot_pos, Color::Color(_r, _g, _b, 50), 0.02f, false, true);

			DDraw::Sphere(last_r_foot_pos, 0.05f, Color::Color(_r, _g, _b, 50), 0.02f, false); //
		}

		if (aidsware::ui::get_bool(xorstr_("desync on visible")))
		{
			if (target_ply->bones()->head->visible_(LocalPlayer::Entity()->eyes()->get_position()))
				LocalPlayer::Entity()->clientTickInterval() = 0.99f;
		}

		if (held)
		{
			if (target_ply
				&& aidsware::ui::get_bool(xorstr_("aimbot"))
				&& get_key(aidsware::ui::get_keybind(xorstr_("aimbot key"))))
			{
				DoAimbot();
			}
		}
		
		if (aidsware::ui::get_bool(xorstr_("follow projectile"))
			&& proj
			&& !projv->isAlive())
			proj = false;


		if (aidsware::ui::get_bool(xorstr_("autoshoot")) && aidsware::ui::get_bool(xorstr_("insta kill")) && aidsware::ui::get_bool(xorstr_("with peek assist")))
			settings::can_insta = other::can_manipulate();

		if (aidsware::ui::get_bool(xorstr_("insta kill"))
			&& get_key(aidsware::ui::get_keybind(xorstr_("insta kill key")))
			|| settings::can_insta)
		{
			if (!aidsware::ui::get_bool(xorstr_("with peek assist"))) settings::can_insta = false;
			settings::instakill = true;
			LocalPlayer::Entity()->clientTickInterval() = 0.99f;

			//float desyncTime = (Time::realtimeSinceStartup() - LocalPlayer::Entity()->lastSentTickTime()) - 0.03125 * 3;
			//create and call function to get specific amount of visible players just before shooting to loop through and shoot at

			int ammo_count = held->primaryMagazine()->contents();

			if (held)
				if (ammo_count > 0)
					if (ammo_count > entities::current_visible_players.size())
						if (desyncTime > 0.8f)
						{
							for (auto a : entities::current_visible_players)
							{
								target_ply = a;
								for (int j = 0; j < aidsware::ui::get_float(xorstr_("bullets")); j++)
									held->LaunchProjectile();
								/*
								if (aidsware::ui::get_bool(xorstr_("with peek assist")))
								{
									settings::peek_insta = true;
									other::find_manipulate_angle(desyncTime, target);
									for (int j = 0; j < aidsware::ui::get_float(xorstr_("bullets")); j++)
										held->LaunchProjectile();
									settings::peek_insta = false;
								}
								else*/
							}
							LocalPlayer::Entity()->clientTickInterval() = 0.05f;
							held->primaryMagazine()->contents()--;
							held->UpdateAmmoDisplay();
							held->ShotFired();
							held->DidAttackClientside();
						}
		}
		else
		{
			//LocalPlayer::Entity()->modelState()->set_mounted(false);
			entities::current_visible_players.clear();
			settings::instakill = false;
		}

		settings::tr::manipulated = aidsware::ui::get_bool(xorstr_("peek assist")) && get_key(aidsware::ui::get_keybind(xorstr_("peek assist key")) || settings::instakill);

		if (aidsware::ui::get_bool(xorstr_("auto upgrade")))
		{
			BaseNetworkable* ent = BaseNetworkable::clientEntities()->FindClosest(STATIC_CRC32("BuildingBlock"), plly, 4.2f);
			auto block = ent->GetComponent<BuildingBlock>(Type::BuildingBlock());
			float dist = plly->eyes()->get_position().distance(block->ClosestPoint(plly->eyes()->get_position()));
			if (dist < 4.2f)
			{
				auto grade = block->grade();

				if (block && (Time::fixedTime() > time_last_upgrade + 0.35f))
				{
					switch (aidsware::ui::get_combobox(xorstr_("upgrade tier"))) {
					case 1:
						if (block->CanAffordUpgrade(BuildingBlock::BuildingGrade::Wood)
							&& block->CanChangeToGrade(BuildingBlock::BuildingGrade::Wood)
							&& grade != BuildingBlock::BuildingGrade::Wood)
						{
							block->Upgrade(BuildingBlock::BuildingGrade::Wood);
							time_last_upgrade = Time::fixedTime();
						}
						break;
					case 2:
						if (block->CanAffordUpgrade(BuildingBlock::BuildingGrade::Stone)
							&& block->CanChangeToGrade(BuildingBlock::BuildingGrade::Stone)
							&& grade != BuildingBlock::BuildingGrade::Stone)
						{
							block->Upgrade(BuildingBlock::BuildingGrade::Stone);
							time_last_upgrade = Time::fixedTime();
						}
						break;
					case 3:
						if (block->CanAffordUpgrade(BuildingBlock::BuildingGrade::Metal)
							&& block->CanChangeToGrade(BuildingBlock::BuildingGrade::Metal)
							&& grade != BuildingBlock::BuildingGrade::Metal)
						{
							block->Upgrade(BuildingBlock::BuildingGrade::Metal);
							time_last_upgrade = Time::fixedTime();
						}
						break;
					case 4:
						if (block->CanAffordUpgrade(BuildingBlock::BuildingGrade::TopTier)
							&& block->CanChangeToGrade(BuildingBlock::BuildingGrade::TopTier)
							&& grade != BuildingBlock::BuildingGrade::TopTier)
						{
							block->Upgrade(BuildingBlock::BuildingGrade::TopTier);
							time_last_upgrade = Time::fixedTime();
						}
						break;
					}
				}
			}
		}

		auto wpn = LocalPlayer::Entity()->GetHeldEntity<BaseMelee>();
		if (wpn)
		{
			auto melee_attack = [&](Vector3 pos, BaseEntity* p, BaseMelee* w, bool is_player = false)
			{
				__try
				{
					if (!p->IsValid() || !w) return;
					if (w->nextAttackTime() >= Time::fixedTime()) return;
					if (w->deployDelay() > w->timeSinceDeploy()) return;

					uintptr_t stat = safe_read(game_assembly + 51671352, DWORD64); if (!stat) return;
					uintptr_t test = il2cpp_object_new(stat); if (!test) return;

					auto trans = is_player ? reinterpret_cast<BasePlayer*>(p)->bones()->head->transform : p->transform();
					if (!trans) return;
					Ray r = Ray(plly->eyes()->get_position(), (pos - LocalPlayer::Entity()->eyes()->get_position()).normalized());

					safe_write(test + 0x34, 1000.f, float); //MaxDistance
					safe_write(test + 0x14, r, Ray); //AttackRay
					safe_write(test + 0x66, true, bool); //DidHit
					safe_write(test + 0xB0, trans, Transform*); //HitTransform...
					safe_write(test + 0x88, p, BaseEntity*); //HitEntity...
					safe_write(test + 0x90, trans->InverseTransformPoint(pos), Vector3); //HitPoint
					//safe_write(test + 0x90, pos, Vector3); //HitPoint
					safe_write(test + 0x9C, trans->InverseTransformDirection(pos), Vector3); //HitNormal Vector3(0, 0, 0)
					safe_write(test + 0x68, w->damageProperties(), DamageProperties*); //DamageProperties... safe_read(w + 0x280, uintptr_t)


					w->StartAttackCooldown(w->repeatDelay());
					//return reinterpret_cast<void(*)(BaseMelee*, uintptr_t)>(game_assembly + 3092976)(w, test);

					return w->ProcessAttack((HitTest*)test);
				}
				__except (true)
				{
					printf(xorstr_("Exception occured in melee_attack!\n"));
					return;
				}
			};

			if (aidsware::ui::get_bool(xorstr_("silent melee"))
				&& target_ply->transform()->position().distance(plly->transform()->position()) < aidsware::ui::get_float(xorstr_("reach")))
			{
				//melee_attack(LocalPlayer::Entity()->ClosestPoint(target_ply->bones()->head->position), target_ply, wpn, true);

				auto mpv = target_ply->find_mpv_bone();
				Vector3 target;
				if (mpv != nullptr)
					target = mpv->position;
				else
					target = target_ply->bones()->head->position;

				melee_attack(target, target_ply, wpn, true);
			}
			if (aidsware::ui::get_bool(xorstr_("silent farm")))
			{
				auto lp = plly;
				auto hotspot = BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("OreHotSpot"), lp, 5.f);
				auto node = (hotspot ? hotspot : BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("OreResourceEntity"), lp, 400.0f));

				auto treespot = BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("TreeMarker"), lp, 5.f);
				if (!treespot)
					treespot = BaseNetworkable::clientEntities()->FindClosest<BaseEntity*>(STATIC_CRC32("TreeEntity"), lp, 400.f);

				if (treespot)
					if (treespot->transform()->position().distance(lp->eyes()->get_position())
						< node->transform()->position().distance(lp->eyes()->get_position()))
						node = treespot;

				//auto node = closestnode();

				if (node)
				{
					if (node->transform()->position().distance(LocalPlayer::Entity()->eyes()->get_position()) < 2.f)
					{
						bool is_tree = false;
						auto n = node;
						auto gathering = wpn->gathering();
						auto wstr = std::wstring(n->ShortPrefabName());
						if (FOUNDW(wstr, wxorstr_(L"tree")))
							is_tree = true;
						//doMeleeAttack(Vector3 pos, BaseEntity* ply, BaseMelee* p, bool is_player = false)
						melee_attack(LocalPlayer::Entity()->ClosestPoint(n->transform()->position()), n, wpn);
					}
				}
			}
		}

		if (aidsware::ui::get_bool(xorstr_("peek assist")) && target_ply != nullptr && target_ply->isCached() && get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) && !settings::instakill)
			other::find_manipulate_angle(desyncTime, target);
		else
			if (!other::m_manipulate.empty())
				other::m_manipulate = Vector3::Zero();

		if (aidsware::ui::get_bool(xorstr_("peek assist")) && target_ply != nullptr && target_ply->isCached() && aidsware::ui::get_bool(xorstr_("autoshoot")) && !settings::instakill)
			if (other::can_manipulate()) {
				other::find_manipulate_angle(desyncTime, target);
				settings::tr::manipulate_visible = true;
			}
			else {
				settings::tr::manipulate_visible = false;
			}

		if (aidsware::ui::get_bool(xorstr_("auto med"))
			&& plly->health() < 99)
		{
			auto held = LocalPlayer::Entity()->GetHeldItem();
			auto ent = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();

			if (std::wstring(held->info()->shortname()).find(wxorstr_(L"med")) != std::string::npos
				|| std::wstring(held->info()->shortname()).find(wxorstr_(L"band")) != std::string::npos)
				if (ent->timeSinceDeploy() > ent->deployDelay() && (ent->get_NextAttackTime() * 0.9) > Time::fixedTime())
				{
					printf("auto medding");
					ent->ServerRPC(xorstr_("UseSelf"));
				}
		}

		if (aidsware::ui::get_bool(xorstr_("always reload")))
		{
			entities::t_TimeSinceLastShot = (Time::fixedTime() - t_FixedTimeLastShot);
			if (t_JustShot && (entities::t_TimeSinceLastShot > 0.2f || held->primaryMagazine()->contents())) //0.2f is like cooldown before reload
			{
				held->ServerRPC("StartReload");
				LocalPlayer::Entity()->SendSignalBroadcast(BaseEntity::Signal::Reload);
				t_JustShot = false;
			}

			if (entities::t_TimeSinceLastShot > (held->reloadTime() - (held->reloadTime() / 10)) + 0.2f //faster reload?
				&& !t_DidReload)
			{
				held->ServerRPC("Reload");
				t_DidReload = true;
			}
		}

		if (settings::suicide == true)
		{
			settings::suicide = false;
			OnLand_hk(LocalPlayer::Entity(), 1000.f);
		}

		if ((aidsware::ui::get_bool(xorstr_("long neck")) || get_key(aidsware::ui::get_keybind(xorstr_("long neck key")))) && get_key(aidsware::ui::get_keybind(xorstr_("desync on key")))) {
			float desyncTime = (Time::realtimeSinceStartup() - plly->lastSentTickTime()) - 0.03125 * 3;
			float max_eye_value = (0.1f + ((desyncTime + 2.f / 60.f + 0.125f) * 1.5f) * plly->MaxVelocity()) - 0.05f;

			plly->eyes()->viewOffset() = Vector3(0, max_eye_value, 0);
		}
		else if (aidsware::ui::get_bool(xorstr_("long neck")) || get_key(aidsware::ui::get_keybind(xorstr_("long neck key"))))
		{
			plly->eyes()->viewOffset() = Vector3(0, 1.495f, 0);
		}

		int sb = aidsware::ui::get_combobox(xorstr_("fake lag"));

		switch (sb)
		{
		case 0:
			break;
		case 1: //basic
			plly->clientTickInterval() = 0.4f;
			break;
		case 2: //doubletap

			float v = 0.5f;
			if (held)
				if (held->repeatDelay() > 0.01f)
					v = (held->repeatDelay() * 2.0f);
			if (plly->clientTickInterval() == 0.4f)
				plly->clientTickInterval() = v;
			else
				plly->clientTickInterval() = 0.4f;
			break;
		}

		Physics::IgnoreLayerCollision(4, 12, !aidsware::ui::get_bool(xorstr_("no collisions")));
		Physics::IgnoreLayerCollision(30, 12, aidsware::ui::get_bool(xorstr_("no collisions")));
		Physics::IgnoreLayerCollision(11, 12, aidsware::ui::get_bool(xorstr_("no collisions")));

		if (get_key(aidsware::ui::get_keybind(xorstr_("zoom key"))))
			ConVar::Graphics::_fov() = 15.f;
		else
			ConVar::Graphics::_fov() = aidsware::ui::get_float(xorstr_("fov"));

		if (aidsware::ui::get_bool(xorstr_("fake admin")))
			plly->playerFlags() |= PlayerFlags::IsAdmin;

		if (aidsware::ui::get_bool(xorstr_("can hold items")))
			if (plly->mounted())
				plly->mounted()->canWieldItems() = true;

		if (aidsware::ui::get_combobox(xorstr_("light")) != 0) {
			float amb = 1.f;
			if (aidsware::ui::get_combobox(xorstr_("light")) == 1)
				amb = 4.f;
			else if (aidsware::ui::get_combobox(xorstr_("light")) == 2)
				amb = 6.f;
			do_light(aidsware::ui::get_float(xorstr_("star brightness")), aidsware::ui::get_float(xorstr_("atmosphere")), amb);
		}
	}

#pragma region alpha shoot same target

	if (settings::alpha::master::shoot_same_target_temp_m // start/stop command
		&& entities::alpha_index != -1)
	{
		settings::alpha::master::shoot_same_target_temp_m = false;

		char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
		memset(buffer, '\x00', 1024 * sizeof(*buffer));

		buffer[0] = settings::alpha::master::shoot_same_target_m ? '\xD1' : '\xB1'; //format: '\xD1' + userid + '\x99';

		std::string uid = "";

		if (target_ply)
			uid = std::to_string(target_ply->userID());
		else uid = "7897459873932451";

		for (size_t i = 1; i < uid.size() + 1; i++)
			buffer[i] = uid[i - 1];

		send_command(buffer);
	}

	if (settings::alpha::master::shoot_same_target_m //no target selected
		&& entities::alpha_index != -1)
	{
		if (!target_ply)
		{
			if (last_target != nullptr)
			{
				last_target = nullptr;
				char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
				memset(buffer, '\x00', 1024 * sizeof(*buffer));
				buffer[0] = '\xA1'; //format: '\xD1' + userid + '\x99';
				std::string uid = "9999999999999999";

				printf("Sent null target\n");

				for (size_t i = 1; i < uid.size() + 1; i++)
					buffer[i] = uid[i - 1];

				send_command(buffer);
			}
		}
		else if (target_ply != last_target)
		{
			last_target = target_ply;
			//send updated target id

			char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
			memset(buffer, '\x00', 1024 * sizeof(*buffer));
			buffer[0] = '\xA1'; //format: '\xD1' + userid + '\x99';
			std::string uid = std::to_string(target_ply->userID());

			printf("Updated target\n");

			for (size_t i = 1; i < uid.size() + 1; i++)
				buffer[i] = uid[i - 1];

			send_command(buffer);
		}
	}

#pragma endregion

#pragma region alpha walk to master marker

	if (settings::alpha::master::walk_to_pos_temp_m
		&& entities::alpha_index != -1)
	{
		settings::alpha::master::walk_to_pos_temp_m = false;

		char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		buffer[0] = '\xD2'; //format: '\xD1' + userid + '\x99';

		MapNote* m = LocalPlayer::Entity()->ClientCurrentMapNote();
		if (m)
		{
			Vector3 v = m->worldPosition();
			int x = v.x, y = v.y, z = v.z;
			printf("v: (%ff, %ff, %ff)\n", v.x, v.y, v.z);
			std::string msg = std::to_string(x) + '\x99' + std::to_string(y) + '\x99' + std::to_string(z) + '\x99';

			for (size_t i = 1; i < msg.size(); i++)
				buffer[i] = msg[i - 1];
			
			send_command(buffer);
		}
	}

	if (settings::alpha::master::walk_to_pos_m
		&& entities::alpha_index == -1)
	{
		char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		buffer[0] = '\xA2'; //format: '\xD1' + userid + '\x99';

		MapNote* m = LocalPlayer::Entity()->ClientCurrentMapNote();
		if (m)
		{
			Vector3 v = m->worldPosition();
			int x = v.x, y = v.y, z = v.z;

			if (last_marker != v)
			{
				std::string msg = std::to_string(x) + '\x99' + std::to_string(y) + '\x99' + std::to_string(z) + '\x99';

				for (size_t i = 1; i < msg.size(); i++)
					buffer[i] = msg[i];

				send_command(buffer);
				last_marker = v;
			}
		}
		else last_marker = Vector3::Zero();
	}

#pragma endregion

#pragma region alpha friend with master

	if (settings::alpha::master::friends_m)
	{
		settings::alpha::master::friends_m = !settings::alpha::master::friends_m;
		char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		buffer[0] = '\xD5'; //format: '\xD1' + userid + '\x99';

		std::string iid = std::to_string(LocalPlayer::Entity()->userID());
		printf("iid: %s\n", iid.c_str());
		for (size_t i = 1; i < iid.size() + 1; i++)
			buffer[i] = iid[i - 1];

		send_command(buffer);
	}

#pragma endregion

#pragma region alpha follow master

	if (settings::alpha::master::follow_master_temp_m)
	{
		settings::alpha::master::follow_master_temp_m = false;
		char buffer[1024]; // '\xD1' shoot same target '\xA1' update targetid
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		buffer[0] = '\xD8'; //format: '\xD1' + userid + '\x99';

		std::string iid = std::to_string(LocalPlayer::Entity()->userID());

		printf("follow id: %s\n", iid.c_str());

		for (size_t i = 1; i < iid.size() + 1; i++)
			buffer[i] = iid[i - 1];

		send_command(buffer);
	}

#pragma endregion

	plly->ClientInput(state);

	// before network 
	if (aidsware::ui::get_bool(xorstr_("omnisprint")))
		LocalPlayer::Entity( )->add_modelstate_flag(ModelState::Flags::Sprinting);

	if (get_key(aidsware::ui::get_keybind(xorstr_("model flags key"))))
	{
		switch (aidsware::ui::get_combobox(xorstr_("model flags"))) {
		case 1:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Ducked);
			break;
		case 2:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Jumped);
			break;
		case 3:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::OnGround);
			break;
		case 4:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Sleeping);
			break;
		case 5:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Sprinting);
			break;
		case 6:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::OnLadder);
			break;
		case 7:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Flying);
			break;
		case 8:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Aiming);
			break;
		case 9:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Prone);
			break;
		case 10:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Mounted);
			break;
		case 11:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Relaxed);
			break;
		case 12:
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::OnPhone);
			break;
		}
	}
}

bool targeted = false;
float target_time = 0.f;

//Vector3 projvec = Vector3::Zero();

std::map<int, Projectile*> dodged_projectiles{};

void DoMovement_hk(Projectile* pr, float deltaTime) {
	if (pr->isAuthoritative())
		if (aidsware::ui::get_bool(xorstr_("hitbox attraction")) || aidsware::ui::get_bool(xorstr_("fat bullet")))
			pr->thickness() = aidsware::ui::get_float(xorstr_("bullet size"));//1.f;
		else
			pr->thickness( ) = 0.1f;

	if (pr->owner()->userID() == LocalPlayer::Entity()->userID()
		&& aidsware::ui::get_bool(xorstr_("follow projectile")))
	{
		proj = true;
		projv = pr;
	}

	auto mod = pr->mod();
	auto ammo_type = mod->ammoType();

	bool fast = false;
	switch (ammo_type)
	{
		case 785728077:
			fast = true;
			break;
		case -1691396643:
			fast = true;
			break;
		case 51984655:
			fast = true;
			break;
		case -1211166256:
			fast = true;
			break;
		case 1712070256:
			fast = true;
			break;
		case 605467368:
			fast = true;
			break;
		case -1321651331:
			fast = true;
			break;
		case -1685290200:
			fast = true;
			break;
		case -727717969:
			fast = true;
			break;
		case -1036635990:
			fast = true;
			break;
		case 588596902:
			fast = true;
			break;


	}

	bool f1 = aidsware::ui::get_bool(xorstr_("dodge projectiles"));
	bool f2 = aidsware::ui::get_bool(xorstr_("show dodges"));

	//APrediction(Vector3 local, Vector3 & target, float bulletspeed, float gravity, float drag, float& te, float& distance_to_travel) {
	if (f1
		&& pr->owner()->userID() != LocalPlayer::Entity()->userID()
		&& !map_contains_key(dodged_projectiles, pr->projectileID()))
	{
		Vector3 l_current_pos = pr->currentPosition();

		for (size_t i = 0; i < 50; i++) //Simulate 100 times per bullet? pretty efficient?
		{
			l_current_pos = GetTrajectoryPoint(l_current_pos,
				pr->initialVelocity(),
				0.1f,
				pr->gravityModifier());

			float dist = l_current_pos.distance(LocalPlayer::Entity()->bones()->head->position);

			if ((fast ? dist < 50.0f : dist < 25.0f)) {
				targeted = true;
				target_time = Time::fixedTime();

				if (f2)
				{
					DDraw::Sphere(last_head_pos, 0.1f, Color::Color(_r, _g, _b, 50), 5.f, false);
					DDraw::Line(last_head_pos, last_neck_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Line(last_neck_pos, last_spine4_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_spine4_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_spine4_pos, last_spine1_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);
					DDraw::Line(last_spine4_pos, last_l_upperarm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);
					DDraw::Line(last_spine4_pos, last_r_upperarm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_spine1_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_spine1_pos, last_pelvis_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);
					DDraw::Line(last_spine1_pos, last_l_upperarm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);
					DDraw::Line(last_spine1_pos, last_r_upperarm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_l_upperarm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_l_upperarm_pos, last_l_forearm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_l_forearm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_l_forearm_pos, last_l_hand_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_l_hand_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //

					DDraw::Sphere(last_r_upperarm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_r_upperarm_pos, last_r_forearm_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_r_forearm_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_r_forearm_pos, last_r_hand_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_r_hand_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //

					DDraw::Sphere(last_pelvis_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_pelvis_pos, last_l_knee_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);
					DDraw::Line(last_pelvis_pos, last_r_knee_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_l_knee_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_l_knee_pos, last_l_foot_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_l_foot_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //

					DDraw::Sphere(last_r_knee_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //
					DDraw::Line(last_r_knee_pos, last_r_foot_pos, Color::Color(_r, _g, _b, 50), 5.f, false, true);

					DDraw::Sphere(last_r_foot_pos, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false); //

				}
				break;
			}
		}
	}
	if (dodged_projectiles.size() >= 3)
		dodged_projectiles.clear();

	
	if (aidsware::ui::get_bool(xorstr_("magic heli"))
		&& target_heli)
	{
		//pr->transform()->set_position(target_heli->transform()->position());

		auto hittest = pr->hitTest();
		hittest->HitEntity() = target_heli;
		hittest->HitTransform() = target_heli->transform();
		hittest->HitPoint() = target_heli->transform()->position();

		//bool DoHit_hk(Projectile * prj, HitTest * test, Vector3 point, Vector3 normal)

		pr->DoHit(hittest, target_heli->transform()->position().normalized(), pr->currentPosition());

		//self->currentPosition() = target_ply->transform()->position();
		return pr->DoMovement(deltaTime);
	}

	return pr->DoMovement(deltaTime);
}

float GetRandomVelocity_hk(ItemModProjectile* self) {
	float modifier = 1.f;

	if (aidsware::ui::get_bool(xorstr_("fast bullets")))
		modifier += 0.499f;
	
	return self->GetRandomVelocity( ) * modifier;
}

void AddPunch_hk(HeldEntity* attackEntity, Vector3 amount, float duration) {
	amount *= aidsware::ui::get_float(xorstr_("recoil %")) / 100.0f;

	attackEntity->AddPunch(amount, duration);
}

Vector3 MoveTowards_hk(Vector3 current, Vector3 target, float maxDelta) {
	static auto ptr = METHOD("Assembly-CSharp::BaseProjectile::SimulateAimcone(): Void");
	if (CALLED_BY(ptr, 0x800)) {
		target *= aidsware::ui::get_float(xorstr_("recoil %")) / 100.0f;
		maxDelta *= aidsware::ui::get_float(xorstr_("recoil %")) / 100.0f;
	}

	return Vector3_::MoveTowards(current, target, maxDelta);
}

bool DoHit_hk(Projectile* prj, HitTest* test, Vector3 point, Vector3 normal) {

	auto lol = test->HitEntity();
	if(lol)
	{
		if (lol->IsPlayer())
		{
			auto ply = lol->GetComponent<BasePlayer>(Type::BasePlayer());

			if (ply)
			{
				if (ply->userID() == entities::friend_id
					|| (ply->is_teammate() && aidsware::ui::get_bool(xorstr_("shoot through teammate"))))
					return false;
			}
		}
	}

	if (aidsware::ui::get_bool(xorstr_("pierce"))) {
		if (prj->isAuthoritative( )) {
			auto go = test->gameObject( );

			if (go && !lol) {
				if (go->layer( ) == 0 || go->layer( ) == 24) {
					return false;
				}
			}
			if (lol) {

				if (   lol->class_name_hash( ) == STATIC_CRC32("CargoShip") 
					|| lol->class_name_hash( ) == STATIC_CRC32("BaseOven")
					|| lol->class_name_hash( ) == STATIC_CRC32("TreeEntity") 
					|| lol->class_name_hash( ) == STATIC_CRC32("OreResourceEntity")
					|| lol->class_name_hash( ) == STATIC_CRC32("CH47HelicopterAIController") 
					|| lol->class_name_hash( ) == STATIC_CRC32("MiniCopter")
					|| lol->class_name_hash( ) == STATIC_CRC32("BoxStorage") 
					|| lol->class_name_hash( ) == STATIC_CRC32("Workbench")
					|| lol->class_name_hash( ) == STATIC_CRC32("VendingMachine") 
					|| lol->class_name_hash( ) == STATIC_CRC32("Barricade")
					|| lol->class_name_hash( ) == STATIC_CRC32("BuildingPrivlidge") 
					|| lol->class_name_hash( ) == STATIC_CRC32("LootContainer")
					|| lol->class_name_hash( ) == STATIC_CRC32("HackableLockedCrate") 
					|| lol->class_name_hash( ) == STATIC_CRC32("ResourceEntity")
					|| lol->class_name_hash( ) == STATIC_CRC32("RidableHorse") 
					|| lol->class_name_hash( ) == STATIC_CRC32("MotorRowboat")
					|| lol->class_name_hash( ) == STATIC_CRC32("ScrapTransportHelicopter") 
					|| lol->class_name_hash( ) == STATIC_CRC32("JunkPile")
					|| lol->class_name_hash( ) == STATIC_CRC32("MiningQuarry") 
					|| lol->class_name_hash( ) == STATIC_CRC32("WaterCatcher")
					|| lol->class_name_hash( ) == STATIC_CRC32("RHIB")) 
				{
					return false;
				}
			}
		}
	}
	//check didhit?
	bool r = prj->DoHit(test, point, normal);

	BaseNetworkable* h = nullptr;
	if (!map_contains_key(projectile_targets, prj->projectileID()))
		return prj;
	else
		h = projectile_targets[prj->projectileID()];

	if (test->HitEntity() != h)
	{
		std::wstring name(reinterpret_cast<BasePlayer*>(h)->_displayName());
		wchar_t buffer[255];
		swprintf(buffer, 255, L"missed [%s] due to prediction", name.c_str());
		LogSystem::Log(buffer, 5.f);
	}
	projectile_targets.erase(prj->projectileID());
	if (projectile_targets.size() > 5)
		projectile_targets.clear();
	return r;
}

void SetEffectScale_hk(Projectile* self, float eScale) {
	return self->SetEffectScale((((aidsware::ui::get_bool(xorstr_("psilent")) && aidsware::ui::get_keybind(xorstr_("psilent key")) == 0) || get_key(aidsware::ui::get_keybind(xorstr_("psilent key")))) && self->isAuthoritative( )) ? 1.5f : eScale);
}
System::Object_* StartCoroutine_hk(MonoBehaviour* a1, System::Object_* un2) {
	if (aidsware::ui::get_bool(xorstr_("fast loot"))) {
		static auto v = METHOD("Assembly-CSharp::ItemIcon::SetTimedLootAction(UInt32,Action): Void");
		if (CALLED_BY(v, 0x656)) {
			*reinterpret_cast<float*>(un2 + 0x28) = -0.2f;
		}
	}

	return a1->StartCoroutine(un2);
}

void BobApply_hk(ViewmodelBob* self, uintptr_t vm, float fov) {
	if (!aidsware::ui::get_bool(xorstr_("omnisprint")))
		self->Apply(vm, fov);
}

void SwayApply_hk(ViewmodelSway* self, uintptr_t vm) {
	if (!aidsware::ui::get_bool(xorstr_("omnisprint")))
		self->Apply(vm);
}

void LowerApply_hk(ViewmodelLower* self, uintptr_t vm) {
	if (!aidsware::ui::get_bool(xorstr_("omnisprint")))
		self->Apply(vm);
}

String* ConsoleRun_hk(ConsoleSystem::Option* optiom, String* str, Array<System::Object_*>* args) {
	//aidswa.re alpha stuff xD
	////
	auto string = std::wstring(str->buffer);
	wprintf(wxorstr_(L"Run: %s (From server: %s)\n"), 
		string.c_str(), 
		(optiom->IsFromServer() ? L"Yes" : L"No"));

	if (string.find(wxorstr_(L"connect")) != std::wstring::npos
		&& string.find(wxorstr_(L"disconnect")) == std::wstring::npos)
	{
		auto list = args;
		for (size_t i = 0; i < list->size(); i++)
		{
			if (i == list->size()) continue;
			auto member_str = reinterpret_cast<String*>(list->get(i));
			auto wstr = std::wstring(member_str->buffer);
			//try string
			wprintf(wxorstr_(L"Arg [%i]: %s\n"), i, wstr.c_str());

			if (wstr.find(wxorstr_(L":")) != std::wstring::npos)
			{
				//notify server we have connected to a game server
				settings::current_server = std::string(wstr.begin(), wstr.end());
				just_joined_server = true;
			}
		}

		if (settings::current_server.size() < 8) //did connect in console
		{
			auto wstr = string.substr(string.find(wxorstr_(L" ")), string.size());
			//try string
			wprintf(wxorstr_(L"Arg: %s\n"), wstr.c_str());

			if (wstr.find(wxorstr_(L":")) != std::wstring::npos)
			{
				//notify server we have connected to a game server
				settings::current_server = std::string(wstr.begin(), wstr.end());
				just_joined_server = true;
			}
		}
	}
	////

	if (string.find(wxorstr_(L"debugcamera")) != std::wstring::npos)
		debugcam = !debugcam;
	if (optiom->IsFromServer( )) {
		if (str->buffer) {
			if (string.find(wxorstr_(L"noclip")) != std::wstring::npos ||
				string.find(wxorstr_(L"debugcamera")) != std::wstring::npos ||
				string.find(wxorstr_(L"admintime")) != std::wstring::npos ||
				string.find(wxorstr_(L"camlerp")) != std::wstring::npos ||
				string.find(wxorstr_(L"camspeed")) != std::wstring::npos) {

				if (string.find(wxorstr_(L"noclip")) != std::wstring::npos)
					is_noclipping = !is_noclipping;

				str = String::New(xorstr_(""));
			}
		}
	}

	return ConsoleSystem::Run(optiom, str, args);
}

void set_flying_hk(ModelState* modelState, bool state) {
	modelState->set_flying(false);
}

bool ValidateMove(float deltaTime)
{
	auto lp = LocalPlayer::Entity();

	bool result;
	bool flag = deltaTime > 1.0f;

	//test for flying
	flyhackPauseTime = MAX(0.f, flyhackPauseTime - deltaTime);
	ticks.Reset();
	if (ticks.HasNext())
	{
		bool flag = lp->transform() ? !(!lp->transform()) : false;

		Matrix matrix4x = flag ? Matrix::identityMatrix() : 
			lp->transform()->get_localToWorldMatrix();

		Vector3 oldPos = flag ? ticks.startPoint :
			matrix4x.MultiplyPoint3x4(ticks.startPoint);
		Vector3 vector = flag ? ticks.startPoint :
			matrix4x.MultiplyPoint3x4(ticks.endPoint);
		float num = 0.1f;
		float num2 = 15.0f;
		num = MAX(ticks.len / num2, num);
		while (ticks.MoveNext(num))
		{
			vector = (flag ? ticks.currentPoint
				: matrix4x.MultiplyPoint3x4(ticks.currentPoint));

			TestFlying2(lp, oldPos, vector, true);
			oldPos = vector;
		}
	}
	return true;
}

void FinalizeTick(float deltaTime)
{
	tickDeltaTime += deltaTime;
	bool flag = ticks.startPoint != ticks.endPoint;
	if (flag)
	{
		if (ValidateMove(tickDeltaTime)
			&& aidsware::ui::get_bool(xorstr_("flyhack stop")) || aidsware::ui::get_bool(xorstr_("flyhack indicator")))
		{
			//printf("GOOD\n");
		}
		else
		{
			//printf("BAD\n");
		}
		settings::flyhack = flyhackDistanceVertical;
		settings::hor_flyhack = flyhackDistanceHorizontal;
	}
	ticks.Reset(LocalPlayer::Entity()->transform()->position());
}

void ServerUpdate(float deltaTime, BasePlayer* ply)
{
	desyncTimeRaw = MAX(ply->lastSentTickTime() - deltaTime, 0.f);
	desyncTimeClamped = MAX(desyncTimeRaw, 1.f);
	FinalizeTick(deltaTime);
	return;
}

int jitter = 1;
int jitter_speed = 10;
int spin_speed = 70;
int spin = 0;

Vector3 rotate_point(Vector3 center, Vector3 origin, float angle) {
	float num = angle * 0.0174532924f;
	float num2 = -std::sin(num);
	float num3 = std::cos(num);
	origin.x -= center.x;
	origin.z -= center.z;
	float num4 = origin.x * num3 - origin.z * num2;
	float num5 = origin.x * num2 + origin.z * num3;
	float num6 = num4 + center.x;
	num5 += center.z;
	return Vector3(num6, origin.y, num5);
}

bool d_drawn = false;
void sendclienttick_hk(BasePlayer* self)
{
	if (Time::fixedTime() > (target_time + 0.1f) && target_time != 0.0f && targeted)
	{
		target_time = 0.0f;
		printf("s-target_time: %ff\n", target_time);
		targeted = false;
	}
	int sb = aidsware::ui::get_combobox(xorstr_("anti-aim"));
	auto input = safe_read(self + 0x4E0, uintptr_t);
	auto state = safe_read(input + 0x20, uintptr_t);
	auto current = safe_read(state + 0x10, uintptr_t);
	if (!current)
		return self->SendClientTick();
	Vector3 real_angles = safe_read(current + 0x18, Vector3);
	Vector3 spin_angles = Vector3::Zero();
	switch (sb)
	{
	case 0: //x = yaw (up/down), y = pitch (spin), z = roll?????;
		if(targeted)
			spin_angles = Vector3(-999.f, (rand() % 999 + -999), (rand() % 999 + -999));
		break;
	case 1: //backwards
		spin_angles.y = real_angles.y + (targeted ? (rand() % -180 + 1) : 180.f);
		break;
	case 2: //backwards (down)
		spin_angles.x = (targeted ? 999.f : -999.f);
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y + 180.f;
		break;
	case 3: //backwards (up)
		spin_angles.x = (targeted ? -999.f : 999.f);
		spin_angles.z = (targeted ? -999.f : 999.f);
		spin_angles.y = real_angles.y + 180.f;
		break;
	case 4: //left
		spin_angles.y = real_angles.y + (targeted ? (rand() % -90 + 1) : 90.f);
		break;
	case 5: //left (down)
		spin_angles.x = (targeted ? 999.f : -999.f);
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y + (targeted ? (rand() % -90 + 1) : 90.f);
		break;
	case 6: //left (up)
		spin_angles.x = (targeted ? -999.f : 999.f);
		spin_angles.z = (targeted ? -999.f : 999.f);
		spin_angles.y = real_angles.y + (targeted ? (rand() % -90 + 1) : 90.f);
		break;
	case 7: //right
		spin_angles.y = real_angles.y + (targeted ? (rand() % 90 + 1) : -90.f);
		break;
	case 8: //right (down)
		spin_angles.x = (targeted ? 999.f : -999.f);
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y + (targeted ? (rand() % 90 + 1) : -90.f);
		break;
	case 9: //right (up)
		spin_angles.x = (targeted ? -999.f : 999.f);
		spin_angles.z = (targeted ? -999.f : 999.f);
		spin_angles.y = real_angles.y + (targeted ? (rand() % 90 + 1) : -90.f);
		break;
	case 10: //jitter
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 45 + 1) : -45.f);
		}
		else if(jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 45 + 1) : 45.f);
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 180 + 1) : -180.f);
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.y = real_angles.y;
		break;
	case 11: //jitter (down)
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 45 + 1) : -45.f);
		}
		else if (jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % -45 + 1) : 45.f);
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % -180 + 1) : 180.f);
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.x = (targeted ? (rand() % 999 + 1) : -999.f);
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y;
		break;
	case 12: //jitter (up)
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % -45 + 1) : 45.f);
		}
		else if(jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 45 + 1) : -45.f);
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y + (targeted ? (rand() % 180 + 1) : -180.f);
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.x = (targeted ? -999.f : 999.f);
		spin_angles.z = (targeted ? -999.f : 999.f);
		spin_angles.y = real_angles.y;
		break;
	case 13: //spin
		spin_angles.y = targeted ? -(real_angles.y + (spin_speed * spin++)) : real_angles.y + (spin_speed * spin++);
		if (spin > (360 / spin_speed))
			spin = 1;
		break;
	case 14: //spin (down)
		spin_angles.x = (targeted ? 999.f : -999.f);
		spin_angles.z = 0.f;
		spin_angles.y = targeted ? -(real_angles.y + (spin_speed * spin++)) : real_angles.y + (spin_speed * spin++);
		if (spin > (360 / spin_speed))
			spin = 1;
		break;
	case 15: //spin (up)
		spin_angles.x = (targeted ? -999.f : 999.f);
		spin_angles.y = targeted ? -(real_angles.y + (spin_speed * spin++)) : real_angles.y + (spin_speed * spin++);
		spin_angles.z = (targeted ? -999.f : 999.f);
		if (spin > (360 / spin_speed))
			spin = 1;
		break;
	case 16: //random
		spin_angles = Vector3((rand() % 999 + -999), (rand() % 999 + -999), (rand() % 999 + -999));
		break;
	}

	if(spin_angles != Vector3::Zero())
		safe_write(current + 0x18, spin_angles, Vector3);

	self->SendClientTick();
	test = self->lastSentTick();

	if (aidsware::ui::get_bool(xorstr_("show desync")))
	{
		last_head_pos =			self->bones()->head->position;
		last_neck_pos =			self->bones()->neck->position;
		last_spine4_pos =		self->bones()->spine4->position;
		last_spine1_pos =		self->bones()->spine1->position;
		last_l_upperarm_pos =	self->bones()->l_upperarm->position;
		last_l_forearm_pos =	self->bones()->l_forearm->position;
		last_l_hand_pos =		self->bones()->l_hand->position;
		last_r_upperarm_pos =	self->bones()->r_upperarm->position;
		last_r_forearm_pos =	self->bones()->r_forearm->position;
		last_r_hand_pos =		self->bones()->r_hand->position;
		last_pelvis_pos =		self->bones()->pelvis->position;
		last_l_knee_pos =		self->bones()->l_knee->position;
		last_l_foot_pos =		self->bones()->l_foot->position;
		last_r_knee_pos =		self->bones()->r_knee->position;
		last_r_foot_pos =		self->bones()->r_foot->position;
	}

	cLastTickPos = self->transform()->position();
	ticks.AddPoint(cLastTickPos);

	if (aidsware::ui::get_bool(xorstr_("flyhack indicator"))
		|| aidsware::ui::get_bool(xorstr_("flyhack stop")))
	{
		ServerUpdate(tickDeltaTime, self);
		//CheckFlyhack();
	}

	return;
}

Vector3 playereyes_getpos_hk(PlayerEyes* self)
{
	return self->get_position();
}

void Launch_hk(Projectile* p)
{
	return p->Launch();
}

void DoHitNotify_hk(BaseCombatEntity* e, HitInfo* info)
{
	if (e->IsPlayer())
	{//StringFormat::format(xorstr_("%s Raid"), type.c_str());
		std::wstring name(reinterpret_cast<BasePlayer*>(e)->_displayName());
		std::wstring hitbone = StringPool::Get(info->HitBone())->buffer; 
		float damage = info->damageTypes()->Total();
		wchar_t buffer[255];
		swprintf(buffer, 255, L"[%s] -%2.f (%s) (%.2fm)", name.c_str(), damage, hitbone.c_str(), info->ProjectileDistance());
		//LogSystem::Log(StringFormat::format(wxorstr_(L"[%s] -%2.f (%s)"), reinterpret_cast<BasePlayer*>(e)->_displayName(), StringPool::Get(info->HitBone())->buffer, info->damageTypes()->Total()), 5.f);
		LogSystem::Log(buffer, 5.f);

		if (aidsware::ui::get_bool(xorstr_("custom hitsound")))
		{
			std::string text = aidsware::ui::get_text(xorstr_("hitsound path"));
			text = settings::data_dir + xorstr_("\\sounds\\") + text;
			if (text.empty())
				return e->DoHitNotify(info);
			if (settings::current_hitsound != text)
			{
				if(!entities::exists(text))
					return e->DoHitNotify(info);
				settings::current_hitsound = text;
			}
			PlaySoundA(text.c_str(), NULL, SND_ASYNC);
			return;
		}
	}
	return e->DoHitNotify(info);
}

Projectile* CreateProjectile_hk(BaseProjectile* self, String* prefabPath, Vector3 pos, Vector3 forward, Vector3 velocity)
{
	Projectile* p = self->CreateProjectile(prefabPath, pos, forward, velocity);
	printf("CreateProjectile called\n");

	if (aidsware::ui::get_bool(xorstr_("instant bullet")))
	{
		auto mpv = target_ply->find_mpv_bone();
		Vector3 target;
		if (mpv != nullptr)
			target = mpv->position;
		else
			target = target_ply->bones()->head->position;

		if (LineOfSight(target, LocalPlayer::Entity()->eyes()->position()))
		{
			p->initialDistance() = 999.f;
			p->integrity() = 999.f;
			printf("Set integrity\n");
		}
	}

	return p;
}

bool requested_flag = false;
void OnRequestUserInformation_hk(Network::Client* self, Network::Message* packet)
{
	printf(xorstr_("OnRequestUserInformation_hk called\n"));
	if (aidsware::ui::get_bool(xorstr_("spoof id")))
	{
		requested_flag = true;
	}
	return self->OnRequestUserInformation(packet);
}

bool init1 = false;
void UInt64_hk(Network::NetWrite* self, uint64_t val)
{
	printf(xorstr_("Joining with steamid: %lld\n"), val);
	settings::steamid = std::to_string(val);
	if (aidsware::ui::get_bool(xorstr_("spoof id")) && requested_flag)
	{
		try
		{
			requested_flag = false;
			uint64_t id = std::stoull(aidsware::ui::get_text(xorstr_("steamid")).c_str());
			printf(xorstr_("Spoofing steamid to %lld\n"), id);
			return self->UInt64(id);
		}
		catch (...)
		{
			printf("Error converting steamid\n");
			return self->UInt64(val);
		}
	}

	if (!init1)
	{
		init1 = true;
		aw_assets = AssetBundle::LoadFromFile(const_cast<char*>("aidsware.assets"));
		other::test_bundle(aw_assets);
	}

	return self->UInt64(val);
} 
 
void ProjectileUpdate_hk(Projectile* self)
{
	if (aidsware::ui::get_bool(xorstr_("fat bullet")) //lol uc leak that probs doesn't even work
		&& self->isAuthoritative()
		&& self->isAlive()
		)//&& false)
	{
		auto cpos = self->transform()->position();
		auto zbone = target_ply->model()->find_bone(cpos).first;
		Vector3 target = zbone->position();

		if (target.distance(cpos) < 2.f)
		{
			DDraw::Sphere(self->transform()->position(), 0.05f, Color::Color(1, 0, 0, 50), 5.f, false);
			self->transform()->set_position(Vector3_::MoveTowards(cpos, target, 1.0f));
			DDraw::Sphere(self->transform()->position(), 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false);
			//self->currentPosition() = Vector3_::MoveTowards(cpos, target, 1.0f);
			if (self->transform()->position().distance(target) <= 1.2f)
			{
				DDraw::Sphere(self->transform()->position(), 0.05f, Color::Color(0, 1, 0, 50), 5.f, false);
				Vector3 t = Vector3_::MoveTowards(self->transform()->position(), target, 0.2f);
				DDraw::Sphere(t, 0.05f, Color::Color(0, 0, 1, 50), 5.f, false);

				HitTest* hitTest = self->hitTest();
				hitTest->DidHit() = true;
				hitTest->HitEntity() = (target_ply);
				hitTest->HitTransform() = (zbone->transform());
				hitTest->HitPoint() = zbone->transform()->InverseTransformPoint(self->transform()->position());
				hitTest->HitNormal() = zbone->transform()->InverseTransformDirection(self->transform()->position());
				hitTest->AttackRay() = Ray(self->transform()->position(), t - self->transform()->position());
				self->integrity() = 999.f;
				//self->hitTest()->HitEntity() = target_ply;
				//self->hitTest()->DidHit() = true;
				//self->hitTest()->HitPoint() = t;
				//self->hitTest()->HitTransform() = zbone->transform();
				DDraw::Sphere(target, 0.05f, Color::Color(_r, _g, _b, 50), 5.f, false);
				self->DoHit(hitTest, t, self->hitTest()->HitNormalWorld());
			}
		}
	}
	
	return self->Update();
}

void connect_t()
{
	//185.132.38.210:51069
	WSADATA w;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &w);
	if (iResult != 0) {
		printf(xorstr_("WSAStartup error: %d\n"), iResult);
		exit(-1);
	}
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iResult = getaddrinfo("185.132.38.210", xorstr_("51069"), &hints, &result);
	//iResult = getaddrinfo("127.0.0.1", xorstr_("51069"), &hints, &result);
	server_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	iResult = connect(server_fd, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(server_fd);
		server_fd = INVALID_SOCKET;
		exit(-1); //WHY IS SERVER DOWN?
	}
	return;
}

void master_connection()
{
	printf("Started master connection...\n");
	char buffer[1024]; 
	const char master_key[25] = "\x12\x13\x21\x10\xA5\xFF\xC3\xB1\xf2\x2c\x2c\x3e\x3e\x02\x26\xae\x24\xde\x7c\x65\x74\xc1\x13\xc1";
	
	for (size_t i = 0; i < 25; i++)
		buffer[i] = master_key[i];
	//send first packet 
	send(server_fd, buffer, 1024, 0);
	printf("Authenticated, getting slaves...\n");

	//first connect get all slavesa
	memset(buffer, '\x00', 1024 * sizeof(*buffer));
	recv(server_fd, buffer, 1024, 0); //recieve amount of slaves

	std::string am = "";

	for (size_t i = 0; i < 1024; i++)
	{
		if (buffer[i] == '\x00') break;
		am += buffer[i];
	}

	int amount = std::stoi(am.c_str());

	printf("%i initial slaves, looping...\n", amount);
	entities::slaves.clear();
	for (size_t a = 0; a < amount; a++)
	{
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		//slave packet info = forum username + \x99 + steam username + \x99 + steamid + \x99 + serverip ('none' if none)
		recv(server_fd, buffer, 1024, 0);

		std::string forum = "";
		std::string steam = "";
		std::string id = "";
		std::string ip = "";
		int in = 0;
		entities::slave t;
		for (size_t i = 0; i < 1024; i++)
		{
			if (in == 0)
			{
				if (buffer[i] == '\x99') in = 1;
				forum += buffer[i];
			}
			if (in == 1)
			{
				if (buffer[i] == '\x99') in = 2;
				steam += buffer[i];
			}
			if (in == 2)
			{
				if (buffer[i] == '\x99') in = 3;
				id += buffer[i];
			}
			if (in == 3)
			{
				if (buffer[i] == '\x99') break;
				ip += buffer[i];
			}
		}
		t.forum_name = forum;
		t.steam_name = steam;
		t.steam_id = id;
		t.server_ip = ip;
		entities::slaves.push_back(t);
	}

	memset(buffer, '\x00', 1024 * sizeof(*buffer));

	std::vector<entities::slave> temp{};
	while (true)
	{
		SleepEx(1000, 0);
		buffer[0] = '\xC0'; //request for slaves
		send(server_fd, buffer, 1024, 0); //send request for slaves

		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		int res1 = recv(server_fd, buffer, 1024, 0); //recieve amount of slaves

		if (res1 != 1024)
			connect_t();

		std::string am1 = "";

		for (size_t i = 0; i < 1024; i++)
		{
			if (buffer[i] == '\x00') break;
			am1 += buffer[i];
		}

		if (buffer[0] == '\x00') am1 = "0";

		int amount = std::stoi(am1.c_str());

		for (size_t a = 0; a < amount; a++)
		{							//crum™karel.2™76561199228076470™20.199.66.146:11111™
			memset(buffer, '\x00', 1024 * sizeof(*buffer));
			//slave packet info = forum username + \x99 + steam username + \x99 + steamid + \x99 + serverip ('none' if none)
			recv(server_fd, buffer, 1024, 0);

			std::string forum = "";
			std::string steam = "";
			std::string id = "";
			std::string ip = "";
			int in = 0;
			entities::slave t;
			for (size_t i = 0; i < 1024; i++)
			{
				if (in == 0)
				{
					if (buffer[i] == '\x99') { in = 1; continue; }
					forum += buffer[i];
				}
				if (in == 1)
				{
					if (buffer[i] == '\x99') { in = 2; continue; }
					steam += buffer[i];
				}
				if (in == 2)
				{
					if (buffer[i] == '\x99') { in = 3; continue; }
					id += buffer[i];
				}
				if (in == 3)
				{
					if (buffer[i] == '\x99') break;
					ip += buffer[i];
				}
			}
			//printf("forum name: %s, steam name: %s, steamid: %s, server ip: %s\n", forum.c_str(), steam.c_str(), id.c_str(), ip.c_str());
			t.forum_name = forum;
			t.steam_name = steam;
			t.steam_id = id;
			t.server_ip = ip;
			temp.push_back(t);
		}
		entities::slaves = temp;
		temp.clear();

		memset(buffer, '\x00', 1024 * sizeof(*buffer));
	}
}

void slave_connection()
{
	char buffer[1024];
	memset(buffer, '\x00', 1024 * sizeof(*buffer));

	//first connection from client	
	std::string f_un = std::string(settings::auth::username.begin(), settings::auth::username.end());
	std::string _m = f_un + '\x99' + xorstr_("NOT_SET") + '\x99' + settings::steamid + '\x99' + settings::current_server + '\x99';
	for (size_t i = 0; i < _m.size(); i++)
		buffer[i] = _m[i];
	send(server_fd, buffer, 1024, 0);



	//wait for commands
	while (true)
	{
		SleepEx(1, 0);
		memset(buffer, '\x00', 1024 * sizeof(*buffer));
		int res1 = recv(server_fd, buffer, 1024, 0);

		if (res1 != 1024)
			connect_t();

		printf("buffer: %s\n", buffer);

		char cmd = buffer[0];

		switch (cmd)
		{
		case '\xD1': //shoot same target
		{
			printf("shoot same target init\n");
			settings::alpha::shoot_same_target = true;
			std::string id = "";
			for (size_t i = 1; i < 1024; i++)
			{
				if (buffer[i] == '\x99') { break; }
				id += buffer[i];
			}
			if(id != settings::steamid)
				entities::target_id = std::stoull(id.c_str());
			printf("id=U%u\n", entities::target_id);
			break;
		}
		case '\xB1':
			settings::alpha::shoot_same_target = false;
			break;
		case '\xD2': //walk_to_pos
		{
			printf("walking to pos\n");
			settings::alpha::walk_to_pos = !settings::alpha::walk_to_pos;

			int x = 0, y = 0, z = 0;

			std::string sx = "", sy = "", sz = "";
			int in = 0;
			for (size_t i = 1; i < 1024; i++)
			{
				if (in == 0)
				{
					if (buffer[i] == '\x99') { in = 1; continue; }
					sx += buffer[i];
				}
				if (in == 1)
				{
					if (buffer[i] == '\x99') { in = 2; continue; }
					sy += buffer[i];
				}
				if (in == 2)
				{
					if (buffer[i] == '\x99') { break; }
					sz += buffer[i];
				}
			}

			printf("x: %s, y: %s, z: %s\n", sx.c_str(), sy.c_str(), sz.c_str());

			x = std::stoi(sx.c_str());
			y = std::stoi(sy.c_str());
			z = std::stoi(sz.c_str());

			entities::walk_to_pos = Vector3(x, y, z);
			break;
		}
		case '\xD3': //flyahck
			printf("forced flyhacking\n");
			settings::alpha::flyhack = !settings::alpha::flyhack;
			break;
		case '\xD4': //walk_to_death
			break;
		case '\xD5': //add friends
		{
			settings::alpha::friends = !settings::alpha::friends;
			std::string id = "";
			for (size_t i = 1; i < 1024; i++)
			{
				if (buffer[i] == '\x99') break;
				id += buffer[i];
			}
			entities::friend_id = std::stoull(id.c_str());
			printf("Friend id: %u\n", entities::friend_id);
			break;
		}
		case '\xD6': //control aim angles
			break;
		case '\xD7': //force join server
			break;
		case '\xD8': //follow master
		{
			printf("follow master init\n");
			settings::alpha::follow_master = !settings::alpha::follow_master;
			std::string id = "";
			for (size_t i = 1; i < 1024; i++)
			{
				if (buffer[i] == '\x99') { break; }
				id += buffer[i];
			}
			entities::master_id = std::stoull(id.c_str());
			printf("follow master_id=U%u\n", entities::master_id);
			printf("follow master_id_str=%s\n", id.c_str());
			break;
		}
			break;
		case '\xA1': //update target
		{
			std::string id = "";
			for (size_t i = 1; i < 1024; i++)
			{
				if (buffer[i] == '\x99') { break; }
				id += buffer[i];
			}
			if (id != settings::steamid)
				entities::target_id = std::stoull(id.c_str());
			break;
		}
		case '\xA2': //update walk_to_pos position
		{
			int x, y, z;

			std::string sx = "", sy = "", sz = "";
			int in = 0;
			for (size_t i = 1; i < 1024; i++)
			{
				if (in == 0)
				{
					if (buffer[i] == '\x99') { in = 1; continue; }
					sx += buffer[i];
				}
				if (in == 1)
				{
					if (buffer[i] == '\x99') { in = 2; continue; }
					sy += buffer[i];
				}
				if (in == 2)
				{
					if (buffer[i] == '\x99') { break; }
					sz += buffer[i];
				}
			}

			x = std::stoi(sx.c_str());
			y = std::stoi(sy.c_str());
			z = std::stoi(sz.c_str());

			entities::walk_to_pos = Vector3(x, y, z);
			break;
		}
		}
	}
}

void OnNetworkMessage_hk(Network::Client* client, Network::Message* packet)
{
	if (!aidsware::ui::get_bool(xorstr_("raid esp"))) 
		return client->OnNetworkMessage(packet);

	//auto n = reinterpret_cast<EffectNetwork*>(CLASS("Assembly-CSharp::EffectNetwork"));

	auto n = reinterpret_cast<EffectNetwork*>(init_class(xorstr_("EffectNetwork")));

	printf(xorstr_("OnNetworkMessage called\n")); 
	if (!n) return client->OnNetworkMessage(packet);
	auto e = n->effect();
	if (!e) return client->OnNetworkMessage(packet);
	 
	auto effectName = e->pooledString()->buffer;
	wprintf(wxorstr_(L"Effect: %s\n"), effectName);
	Vector3 position = e->worldPos();
	if (aidsware::ui::get_bool("raid esp") && e && !position.empty()) {
		switch (RUNTIME_CRC32_W(effectName)) {
		case STATIC_CRC32("assets/prefabs/tools/c4/effects/c4_explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("C4"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/satchelcharge/effects/satchel-charge-explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("Satchel"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/rocketlauncher/effects/rocket_explosion_incendiary.prefab"):
			LogSystem::LogExplosion(xorstr_("Incendiary rocket"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/rocketlauncher/effects/rocket_explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("Rocket"), position);
			break;
		}
	}

	return client->OnNetworkMessage(packet);
}

GameObject* CreateEffect_hk(pUncStr strPrefab, Effect* effect)
{
	auto effectName = strPrefab->str;
	printf("Effect: %s\n", effectName);
	Vector3 position = effect->worldPos();
	if (aidsware::ui::get_bool("raid esp") && effect && strPrefab->str && !position.empty()) {
		switch (RUNTIME_CRC32_W(effectName)) {
		case STATIC_CRC32("assets/prefabs/tools/c4/effects/c4_explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("C4"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/satchelcharge/effects/satchel-charge-explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("Satchel"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/rocketlauncher/effects/rocket_explosion_incendiary.prefab"):
			LogSystem::LogExplosion(xorstr_("Incendiary rocket"), position);
			break;
		case STATIC_CRC32("assets/prefabs/weapons/rocketlauncher/effects/rocket_explosion.prefab"):
			LogSystem::LogExplosion(xorstr_("Rocket"), position);
			break;
		}
	}
	return EffectLibrary::CreateEffect_(strPrefab, effect);
	//return original_createeffect(strPrefab, effect);
}

void LaunchProjectile_hk(BaseProjectile* self)
{
	bool z = settings::tr::manipulated;
	Vector3 o = LocalPlayer::Entity()->transform()->position();
	if (z)
	{
		//Vector3 vec = LocalPlayer::Entity()->eyes()->get_position_(LocalPlayer::Entity()->eyes());
		//LocalPlayer::Entity()->transform()->position() = vec;
		//LocalPlayer::Entity()->SendClientTick();
	}

	int sb = aidsware::ui::get_combobox(xorstr_("double tap"));
	float desyncTime = (Time::realtimeSinceStartup() - LocalPlayer::Entity()->lastSentTickTime()) - 0.03125 * 3;

	if(aidsware::ui::get_bool(xorstr_("insta kill"))
		&& (get_key(aidsware::ui::get_keybind(xorstr_("insta kill key")))))
		return self->LaunchProjectile();

	int ammo = self->primaryMagazine()->contents();
	if(ammo <= 0) return self->LaunchProjectile();
	if (aidsware::ui::get_bool(xorstr_("rapid fire"))) sb = 0;
	switch (sb)
	{
	case 0:
		break;
	case 1: //basic

		if (desyncTime > ((self->repeatDelay() * 0.9f) * 2.0f))
		{
			self->LaunchProjectile();
			if (self->primaryMagazine()->contents() > 0)
			{
				self->LaunchProjectile();
				self->primaryMagazine()->contents()--;
				self->UpdateAmmoDisplay();
				self->ShotFired();
				self->DidAttackClientside();
			}
			LocalPlayer::Entity()->SendClientTick();
			return;
		}

		break;
	case 2: //smart
		float f = desyncTime / (self->repeatDelay() * 0.9f);
		int z = (int)f;


		for (size_t i = 0; i < (z > 9 ? 9 : (z < 0 ? 0 : z)); i++)
			if (self->primaryMagazine()->contents() > 0)
			{
				self->LaunchProjectile();
				self->primaryMagazine()->contents()--;
				self->UpdateAmmoDisplay();
				self->ShotFired();
				self->DidAttackClientside();
			}
		
		if(z <= 0)
			self->LaunchProjectile();

		LocalPlayer::Entity()->SendClientTick();
		return;
	}

	//if (z)
	//	LocalPlayer::Entity()->transform()->position() = o;

	return self->LaunchProjectile();
}

void OnGui_hk(DDraw* instance)
{
	return DDraw::OnGui_(instance);
}

bool Refract_hk(Projectile* self, uint64_t& seed, Vector3 point, Vector3 normal, bool resistance)
{
	return self->Refract(seed, point, normal, resistance);
}

void do_hooks( ) {
	//VM_DOLPHIN_BLACK_START

	//VMProtectBeginUltra(xorstr_("hook"));


	hookengine::hook(BasePlayer::ClientUpdate_, ClientUpdate_hk);
	hookengine::hook(BasePlayer::ClientUpdate_Sleeping_, ClientUpdate_Sleeping_hk);
	hookengine::hook(PlayerWalkMovement::UpdateVelocity_, UpdateVelocity_hk);
	hookengine::hook(PlayerWalkMovement::HandleJumping_, HandleJumping_hk);
	hookengine::hook(BasePlayer::CanAttack_, CanAttack_hk);
	hookengine::hook(BasePlayer::OnLand_, OnLand_hk);
	hookengine::hook(Projectile::DoMovement_, DoMovement_hk);
	hookengine::hook(FlintStrikeWeapon::DoAttack_, DoAttack_hk);
	hookengine::hook(ViewmodelBob::Apply_, BobApply_hk);
	hookengine::hook(ViewmodelSway::Apply_, SwayApply_hk);
	hookengine::hook(InputState::IsDown_, IsDown_hk);
	hookengine::hook(BaseCombatEntity::OnAttacked_, OnAttacked_hk);
	hookengine::hook(ConsoleSystem::Run_, ConsoleRun_hk);
	hookengine::hook(ViewmodelLower::Apply_, LowerApply_hk);
	hookengine::hook(ModelState::set_flying_, set_flying_hk);
	hookengine::hook(HitTest::BuildAttackMessage_, BuildAttackMessage_hk);
	hookengine::hook(BaseMelee::ProcessAttack_, ProcessAttack_hk);
	hookengine::hook(Projectile::DoHit_, DoHit_hk);
	hookengine::hook(BaseMountable::EyePositionForPlayer_, EyePositionForPlayer_hk);
	hookengine::hook(MonoBehaviour::StartCoroutine_, StartCoroutine_hk);
	hookengine::hook(Projectile::SetEffectScale_, SetEffectScale_hk);
	hookengine::hook(BasePlayer::ClientInput_, ClientInput_hk);
	hookengine::hook(ItemModProjectile::GetRandomVelocity_, GetRandomVelocity_hk);
	hookengine::hook(PlayerEyes::BodyLeanOffset_, BodyLeanOffset_hk);
	hookengine::hook(AimConeUtil::GetModifiedAimConeDirection_, GetModifiedAimConeDirection_hk);
	hookengine::hook(PlayerEyes::DoFirstPersonCamera_, DoFirstPersonCamera_hk);
	hookengine::hook(Vector3_::MoveTowards_, MoveTowards_hk);
	hookengine::hook(HeldEntity::AddPunch_, AddPunch_hk);
	hookengine::hook(PlayerEyes::get_position_, playereyes_getpos_hk);

	hookengine::hook(Projectile::Launch_, Launch_hk);

	hookengine::hook(BasePlayer::SendClientTick_, sendclienttick_hk);

	hookengine::hook(BaseCombatEntity::DoHitNotify_, DoHitNotify_hk);

	hookengine::hook(BaseProjectile::CreateProjectile_, CreateProjectile_hk);

	hookengine::hook(Network::Client::OnRequestUserInformation_, OnRequestUserInformation_hk);

	hookengine::hook(Network::NetWrite::UInt64_, UInt64_hk);

	hookengine::hook(Network::Client::OnNetworkMessage_, OnNetworkMessage_hk);

	hookengine::hook(BaseProjectile::LaunchProjectile_, LaunchProjectile_hk);

	hookengine::hook(Projectile::Update_, ProjectileUpdate_hk);

	hookengine::hook(DDraw::OnGui_, OnGui_hk);

	hookengine::hook(Projectile::Refract_, Refract_hk);
	//create slave/master connection thread
	//connect_t();
	/*
	if (settings::auth::username == std::wstring(wxorstr_(L"kai")))
	{ //master
		const auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(master_connection), 0, 0, nullptr);
		if (handle != NULL)
			CloseHandle(handle);
	}
	else
	{
		const auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(slave_connection), 0, 0, nullptr);
		if (handle != NULL)
			CloseHandle(handle);
	}
	*/
	//VMProtectEnd();
	//VM_DOLPHIN_BLACK_END
}

void undo_hooks( ) {
	//VM_DOLPHIN_BLACK_START
	//VMProtectBeginUltra(xorstr_("unhook"));

	closesocket(server_fd);


	hookengine::unhook(BaseCombatEntity::OnAttacked_, OnAttacked_hk);
	hookengine::unhook(BaseCombatEntity::DoHitNotify_, DoHitNotify_hk);

	hookengine::unhook(BasePlayer::ClientUpdate_, ClientUpdate_hk);
	hookengine::unhook(BasePlayer::CanAttack_, CanAttack_hk);
	hookengine::unhook(BasePlayer::OnLand_, OnLand_hk);
	hookengine::unhook(BasePlayer::ClientInput_, ClientInput_hk);
	hookengine::unhook(BasePlayer::SendClientTick_, sendclienttick_hk);

	hookengine::unhook(PlayerWalkMovement::UpdateVelocity_, UpdateVelocity_hk);
	hookengine::unhook(PlayerWalkMovement::HandleJumping_, HandleJumping_hk);

	hookengine::unhook(Projectile::DoMovement_, DoMovement_hk);
	hookengine::unhook(Projectile::SetEffectScale_, SetEffectScale_hk);
	hookengine::unhook(Projectile::Launch_, Launch_hk);
	hookengine::unhook(Projectile::Update_, ProjectileUpdate_hk);
	hookengine::unhook(Projectile::DoHit_, DoHit_hk);

	hookengine::unhook(BaseProjectile::CreateProjectile_, CreateProjectile_hk);
	hookengine::unhook(BaseProjectile::LaunchProjectile_, LaunchProjectile_hk);

	hookengine::unhook(FlintStrikeWeapon::DoAttack_, DoAttack_hk);
	hookengine::unhook(ViewmodelBob::Apply_, BobApply_hk);
	hookengine::unhook(ViewmodelSway::Apply_, SwayApply_hk);
	hookengine::unhook(InputState::IsDown_, IsDown_hk);
	hookengine::unhook(ConsoleSystem::Run_, ConsoleRun_hk);
	hookengine::unhook(ViewmodelLower::Apply_, LowerApply_hk);
	hookengine::unhook(ModelState::set_flying_, set_flying_hk);
	hookengine::unhook(HitTest::BuildAttackMessage_, BuildAttackMessage_hk);
	hookengine::unhook(BaseMelee::ProcessAttack_, ProcessAttack_hk);
	hookengine::unhook(BaseMountable::EyePositionForPlayer_, EyePositionForPlayer_hk);
	hookengine::unhook(MonoBehaviour::StartCoroutine_, StartCoroutine_hk);
	hookengine::unhook(ItemModProjectile::GetRandomVelocity_, GetRandomVelocity_hk);
	hookengine::unhook(PlayerEyes::BodyLeanOffset_, BodyLeanOffset_hk);
	hookengine::unhook(AimConeUtil::GetModifiedAimConeDirection_, GetModifiedAimConeDirection_hk);
	hookengine::unhook(PlayerEyes::DoFirstPersonCamera_, DoFirstPersonCamera_hk);
	hookengine::unhook(Vector3_::MoveTowards_, MoveTowards_hk);
	hookengine::unhook(HeldEntity::AddPunch_, AddPunch_hk);

	hookengine::unhook(PlayerEyes::get_position_, playereyes_getpos_hk);

	hookengine::unhook(Network::Client::OnRequestUserInformation_, OnRequestUserInformation_hk);

	hookengine::unhook(Network::NetWrite::UInt64_, UInt64_hk);

	hookengine::unhook(Network::Client::OnNetworkMessage_, OnNetworkMessage_hk);

	hookengine::unhook(DDraw::OnGui_, OnGui_hk);

	hookengine::unhook(Projectile::Refract_, Refract_hk);
	
	//VMProtectEnd();
	//VM_DOLPHIN_BLACK_END
}