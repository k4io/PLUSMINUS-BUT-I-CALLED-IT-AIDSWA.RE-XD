#include <intrin.h>
#include <vector>

#define CALLED_BY(func,off) (reinterpret_cast<std::uint64_t>(_ReturnAddress()) > func && reinterpret_cast<std::uint64_t>(_ReturnAddress()) < func + off)

void ClientUpdate_hk(BasePlayer* plly) {
	auto local = LocalPlayer::Entity( );
	if (local) {
		if (aidsware::ui::get_bool(xorstr_("players")) || aidsware::ui::get_bool(xorstr_("sleepers"))) {
			bonecache::cachePlayer(plly);
		}

		if (plly->userID() != LocalPlayer::Entity()->userID()) {
			return plly->ClientUpdate( );
		}
		
		/*
		if (aidsware::ui::get_bool(xorstr_("chams")))
		{
			auto list = plly->playerModel()->_multiMesh()->Renderers();

			if (list) {
				for (int i = 0; i < list->size; i++) {
					if (i == list->size) continue;
					auto member = reinterpret_cast<Renderer_*>(list->get(i));
					if (!member) continue;

					member->set_material(0);
				}
			}
		}
*/
		if (get_key(aidsware::ui::get_keybind(xorstr_("timescale key"))))
			Time::set_timeScale(2.f);
		else Time::set_timeScale(1.f);

		if (aidsware::ui::get_bool(xorstr_("fake lag")))
			plly->clientTickInterval() = 0.4f;
		else plly->clientTickInterval() = 0.05f;

		if (aidsware::ui::get_bool(xorstr_("spiderman")))
		{
			plly->movement()->groundAngle() = 0.f;
			plly->movement()->groundAngleNew() = 0.f;
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

			if (aidsware::ui::get_bool(xorstr_("autoshoot")) || (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible))) {
				if (!held->Empty() && (held->class_name_hash() == STATIC_CRC32("BaseProjectile")
					|| held->class_name_hash() == STATIC_CRC32("BowWeapon")
					|| held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")
					|| held->class_name_hash() == STATIC_CRC32("BaseLauncher")
					|| held->class_name_hash() == STATIC_CRC32("CrossbowWeapon"))) {
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

Vector3 GetModifiedAimConeDirection_hk(float aimCone, Vector3 inputVec, bool anywhereInside = true) {

	aimCone *= aidsware::ui::get_float(xorstr_("spread %")) / 100.0f;
	return AimConeUtil::GetModifiedAimConeDirection(aimCone, inputVec, anywhereInside);

	bool flag = false;
	if (get_key(aidsware::ui::get_keybind(xorstr_("psilent key"))))
	{
		flag = true;
	}

	if ((aidsware::ui::get_bool(xorstr_("psilent")) || flag) && target_ply != nullptr && target_ply->isCached( )) {
		//inputVec = (aimutils::get_prediction( ) - LocalPlayer::Entity( )->eyes( )->position( )).normalized( );
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

void APrediction(Vector3 local, Vector3& target, float bulletspeed, float gravity, float drag) {
	float Dist = local.distance(target);

	/*
	//put bullet drop here

	//float m_flBulletSpeed = (bulletvelocity * projectileVelocityScale);

	float m_flBulletSpeed = (projectileVelocity * projectileVelocityScale);

	float bullet_time = Dist / m_flBulletSpeed;

	const float m_flTimeStep = 0.005f;
	float m_flYTravelled{}, m_flYSpeed{}, m_flBulletTime{}, m_flDivider{};

	float m_flDistanceTo = Dist;//Calc3D_Dist(From, aimpoint);//from.distance(aimpoint);
	//float m_flDistanceTo = fVrom.distance(aimpoint);

	for (float distance_to_travel = 0.f; distance_to_travel < m_flDistanceTo;)
	{
		float speed_modifier = 1.0f - m_flTimeStep * drag;
		m_flBulletSpeed *= speed_modifier;

		if (m_flBulletSpeed <= 0.f || m_flBulletSpeed >= 10000.f || m_flYTravelled >= 10000.f || m_flYTravelled < 0.f)
			break;

		if (m_flBulletTime > 8.f)
			break;

		m_flYSpeed += (9.81f * gravity) * m_flTimeStep;
		m_flYSpeed *= speed_modifier;

		distance_to_travel += m_flBulletSpeed * m_flTimeStep;
		m_flYTravelled += m_flYSpeed * m_flTimeStep;
		m_flBulletTime += m_flTimeStep;
	}

	double height = target.y - local.y;
	Vector3 dir = target - local;
	float DepthPlayerTarget = Vector3::my_sqrt(powFFFFFFFFFFFFFFFFFFFFFF(dir.x) + powFFFFFFFFFFFFFFFFFFFFFF(dir.z));
	float drop = CalcBulletDrop(height, DepthPlayerTarget, m_flBulletSpeed, gravity);
*/
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

	float bullet_speed = (itemModProjectile->GetRandomVelocity() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.f)) * base_projectile->projectileVelocityScale();

	if (base_projectile->class_name_hash() == STATIC_CRC32("CompoundBowWeapon"))
		bullet_speed = (itemModProjectile->GetRandomVelocity() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.f)) * reinterpret_cast<CompoundBowWeapon*>(base_projectile)->GetProjectileVelocityScale();

	if (bullet_speed == 0.f)
		return;
	Projectile* projectile = itemModProjectile->projectileObject()->Get()->GetComponent<Projectile>(Type::Projectile());

	if (projectile == nullptr)
		return;


	float m_flBulletSpeed = (itemModProjectile->projectileVelocity() * (base_projectile->projectileVelocityScale() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.0f)));
	float distance = target.distance(LocalPlayer::Entity()->eyes()->position());
	float bullet_time = distance / m_flBulletSpeed;
	const float m_flTimeStep = 0.005f;
	float m_flYTravelled{}, m_flYSpeed{}, m_flBulletTime{}, m_flDivider{};

	//float m_flDistanceTo = fVrom.distance(aimpoint);

	for (float distance_to_travel = 0.f; distance_to_travel < distance;)
	{
		//float speed_modifier = (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.0f) - m_flTimeStep * projectile->drag();
		float speed_modifier = 1.0f - m_flTimeStep * projectile->drag();
		m_flBulletSpeed *= speed_modifier;

		if (m_flBulletSpeed <= 0.f || m_flBulletSpeed >= 50000.f || m_flYTravelled >= 50000.f || m_flYTravelled < 0.f)
			break;

		if (m_flBulletTime > 60.f)
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

	Vector3 p_target = target;

	target.y += m_flYTravelled;
	target += velocity * m_flBulletTime;

	target = Vector3::Lerp(p_target, target, aidsware::ui::get_float(xorstr_("lerp")));

	TraceResult f = traceProjectile(local,
		target,
		drag,
		Vector3(0, -9.81 * gravity, 0),
		target);
	LogSystem::AddTraceResult(f);

	/*
	//printf("initialVel: (%ff, %ff, %ff)\n", initialVel.x, initialVel.y, initialVel.z);

	bulletspeed *= 1.f - stepRate * drag;
	//float BulletTime = Dist / bulletspeed;
	float BulletTime = f.hitTime;

	Vector3 vel = Vector3(targetvel.x, 0, targetvel.z) * 0.75f;
	//Vector3 vel = Vector3(f.outVelocity.x, 0, f.outVelocity.z) * 0.75f;
	Vector3 PredictVel = vel * BulletTime;
	//Vector3 PredictVel = f.outVelocity;
	target += PredictVel;
	target.y += drop;
	*/
}

void serverrpc_projectileshoot_hk(int64_t rcx, int64_t rdx, int64_t r9, int64_t ProjectileShoot, int64_t arg5)
{
	while (1)
	{
		if (!ProjectileShoot)
			break;
		auto loco = LocalPlayer::Entity();
		if (!loco) break;

		auto baseprojectile = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
		if (!baseprojectile) break;
		auto wep_class_name = baseprojectile->class_name(); 

		if (!baseprojectile->Empty() && (baseprojectile->class_name_hash() != STATIC_CRC32("BaseProjectile")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("BowWeapon")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("CompoundBowWeapon")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("BaseLauncher")
			&& baseprojectile->class_name_hash() != STATIC_CRC32("CrossbowWeapon"))) {
			break;
		}

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


		for (size_t i = 0; i < sz; i++)
		{
			auto projectile = *(uintptr_t*)(shoot_list + 0x20 + i * 0x8); // 

			if (target_ply)
			{
				int id = *reinterpret_cast<int*>(projectile + 0x14);
				if (!map_contains_key(projectile_targets, id))
					projectile_targets.insert(std::make_pair(id, target_ply));
				else
					projectile_targets[id] = target_ply;
			}

			rpc_position = *reinterpret_cast<Vector3*>(projectile + 0x18); //
			auto original_vel = *reinterpret_cast<Vector3*>(projectile + 0x24); //
			//auto itemmod = *reinterpret_cast<uintptr_t*>(projectile + 0xE8); //
			//auto itemmodvel = *reinterpret_cast<float*>(itemmod + 0x34); //
			//auto scale = *reinterpret_cast<float*>(projectile + 0x284); //

			//auto itemmod = safe_read(projectile + 0xE8, uintptr_t);
			//auto itemmodvel = safe_read(itemmod + 0x34, float);
			//(itemModProjectile->projectileVelocity() * base_projectile->projectileVelocityScale());

			if (target_ply/* && !target.teammate*/) { //Vector3 local, Vector3& target, float bulletspeed, float gravity, float drag
				//APrediction(v, bonepos, vel, original_vel.Length(), stats.gravity_modifier, stats.drag, itemmodvel, scale);
				APrediction(v, bonepos, original_vel.Length(), stats.gravity_modifier, stats.drag);
				aim_angle = /*get_aim_angle(rpc_position, target.pos, target.velocity, false, stats)*/bonepos - rpc_position;

				aimbot_velocity = (aim_angle).normalized() * original_vel.Length();

				*reinterpret_cast<Vector3*>(projectile + 0x24) = aimbot_velocity;
				//*reinterpret_cast<Vector3*>(projectile + 0x24) = aimbot_velocity;
			}
		}

		auto sz2 = safe_read(projectile_list + 0x18, int);


		for (int i = 0; i < sz2; i++)
		{
			auto projectile = *(uintptr_t*)((uintptr_t)projectile_list + 0x20 + i * 0x8);

			if (!projectile)
				continue;

			if (aidsware::ui::get_bool(xorstr_("psilent"))) {
				if (target_ply) {
					safe_write(projectile + 0x118, aimbot_velocity, Vector3);
					//p->currentVelocity() = aimbot_velocity;
				}
			}
		}


		break;
	}
	reinterpret_cast<void (*)(int64_t, int64_t, int64_t, int64_t, int64_t)>(settings::serverrpc_projectileshoot)(rcx, rdx, r9, ProjectileShoot, arg5);
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
			if (aidsware::ui::get_bool(xorstr_("bullet tracers"))) {
				DDraw::Line(localPlayer->eyes( )->get_position( ), ret->hitPositionWorld( ), Color(1, 0, 0, 1), 10.f, false, true);
			}

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
	if (aidsware::ui::get_bool(xorstr_("peek assist"))) {
		Vector3 re_p = LocalPlayer::Entity( )->transform( )->position( ) + LocalPlayer::Entity( )->transform( )->up( ) * (PlayerEyes::EyeOffset( ).y + LocalPlayer::Entity( )->eyes( )->viewOffset( ).y);
		cam->transform( )->set_position(re_p);
	}
}

bool CanAttack_hk(BasePlayer* self) {
	if (aidsware::ui::get_bool(xorstr_("can hold items")))
		return true;

	return self->CanAttack( );
}

void UpdateVelocity_hk(PlayerWalkMovement* self) {

	if (aidsware::ui::get_bool(xorstr_("walk to marker")))
	{
		float speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
		MapNote* m = LocalPlayer::Entity()->ClientCurrentMapNote();
		if (m)
		{
			Vector3 pos = LocalPlayer::Entity()->transform()->position();
			Vector3 marker_pos = m->worldPosition();
			Vector3 dir = (marker_pos - pos).normalized();
			self->TargetMovement() = { (dir.x / dir.length() * speed), dir.y,(dir.z / dir.length() * speed) };
		}
	}

	Vector3 vel = self->TargetMovement();

	if (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible)) {
		float max_speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
		if (vel.length() > 0.f) {
			//self->TargetMovement( ) = Vector3::Zero( );
		}
	}

	if (aidsware::ui::get_bool(xorstr_("flyhack stop")))
	{
		Vector3 fvel = vel;
		float threshold = aidsware::ui::get_float(xorstr_("threshold"));
		if (settings::hor_flyhack * 100.f >= threshold)
		{
			fvel.x = 0.0f;
			fvel.z = 0.0f;
		}
		if (settings::flyhack * 100.f >= threshold)
		{
			fvel.y = 0.0f;
		}
		self->TargetMovement() = fvel;
	}

	if (!self->flying()) {
		float threshold = aidsware::ui::get_float(xorstr_("threshold"));
		if (settings::hor_flyhack * 100.f < threshold
			&& settings::flyhack * 100.f < threshold)
		{
			if (aidsware::ui::get_bool(xorstr_("omnisprint"))) {
				float max_speed = (self->swimming() || self->Ducking() > 0.5) ? 1.7f : 5.5f;
				if (vel.length() > 0.f) {
					Vector3 target_vel = Vector3(vel.x / vel.length() * max_speed, vel.y, vel.z / vel.length() * max_speed);
					self->TargetMovement() = target_vel;
				}
			}
		}
	}
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
		if (settings::hor_flyhack * 100.f >= threshold
			|| settings::flyhack * 100.f >= threshold)
			return;
	}
	if (aidsware::ui::get_bool(xorstr_("infinite jump"))) {
		if (!wantsJump)
			return;

		a1->grounded( ) = (a1->climbing( ) = (a1->sliding( ) = false));
		state->set_ducked(false);
		a1->jumping( ) = true;
		state->set_jumped(true);
		a1->jumpTime( ) = Time::time( );
		a1->ladder( ) = nullptr;

		Vector3 curVel = a1->body( )->velocity( );
		a1->body( )->set_velocity({ curVel.x, 10, curVel.z });
		return;
	}

	return a1->HandleJumping(state, wantsJump, jumpInDirection);
}

void OnLand_hk(BasePlayer* ply, float vel) {
	if (!aidsware::ui::get_bool(xorstr_("no fall")))
		ply->OnLand(vel);
}

bool IsDown_hk(InputState* self, BUTTON btn) {
	if (aidsware::ui::get_bool(xorstr_("autoshoot")) || (aidsware::ui::get_bool(xorstr_("peek assist")) && (get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) || settings::tr::manipulate_visible))) {
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

		if (btn == BUTTON::FIRE_PRIMARY) {
			auto held = LocalPlayer::Entity( )->GetHeldEntity<BaseProjectile>( );
			if (held && !held->Empty( ) && (held->class_name_hash() == STATIC_CRC32("BaseProjectile")
											|| held->class_name_hash() == STATIC_CRC32("BowWeapon")
											|| held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")
											|| held->class_name_hash() == STATIC_CRC32("BaseLauncher")
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

GameObject* CreateEffect_hk(pUncStr strPrefab, Effect* effect)
{
	auto effectName = strPrefab->str;
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

bool has_intialized_methods = false;
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

	if (plly->userID( ) == LocalPlayer::Entity( )->userID( )) {
		auto held = plly->GetHeldEntity<BaseProjectile>();

		GLOBAL_TIME = Time::time(); 
		float desyncTime = (Time::realtimeSinceStartup() - plly->lastSentTickTime()) - 0.03125 * 3;
		settings::tr::desync_time = desyncTime;


		if (aidsware::ui::get_bool(xorstr_("rapid fire"))) {
			if (held)
				held->repeatDelay() = 0.07f;
		}
		
		if (get_key(aidsware::ui::get_keybind(xorstr_("desync on key"))))
			LocalPlayer::Entity()->clientTickInterval() = 0.99f;

		if (held)
		{	
			auto wep_class_name = held->class_name();

			if (aidsware::ui::get_bool(xorstr_("long hand")) && *(int*)(wep_class_name + 4) == 'eleM') {
				safe_write(held + 0x290 /*maxDistance*/, 5.f, float);
			}
		}

		if (aidsware::ui::get_bool(xorstr_("stack tcs")))
		{
			auto inv = LocalPlayer::Entity()->inventory();
			auto container = inv->containerMain();
			auto itemlist = container->itemList();
			auto sz = itemlist->size();
			for (size_t i = 0; i < sz; i++)
			{
				auto member = itemlist->get(i);
				auto d = *member;

				if (d->info()->class_name_hash() == STATIC_CRC32("Wood"))
				{
					auto stack_sz = d->amount();
					if (amount == 1000)
					{
						//start crafting tc???

						
					}
				}
			}
		}

		//todo:
		/*	

			instant jackhammer refill
			stack crafting tcs
			weapon spam
			legit recoil
		*/

		if (aidsware::ui::get_bool(xorstr_("flyhack indicator"))
			|| aidsware::ui::get_bool(xorstr_("flyhack stop")))
		{
			CheckFlyhack();
			//printf("settings::flyhack: %ff\n", settings::flyhack);
			//printf("settings::hor_flyhack: %ff\n", settings::hor_flyhack);
		}

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

								if (aidsware::ui::get_bool(xorstr_("with peek assist")))
								{
									settings::peek_insta = true;
									other::find_manipulate_angle();
									for (int j = 0; j < aidsware::ui::get_float(xorstr_("bullets")); j++)
										held->LaunchProjectile();
									settings::peek_insta = false;
								}
								else
									for (int j = 0; j < aidsware::ui::get_float(xorstr_("bullets")); j++)
										held->LaunchProjectile();
							}
						}
		}
		else
		{
			//LocalPlayer::Entity()->modelState()->set_mounted(false);
			entities::current_visible_players.clear();
			settings::instakill = false;
		}


		settings::tr::manipulated = aidsware::ui::get_bool(xorstr_("peek assist")) && get_key(aidsware::ui::get_keybind(xorstr_("peek assist key")) || settings::instakill);

		
		if (aidsware::ui::get_bool(xorstr_("peek assist")) && target_ply != nullptr && target_ply->isCached() && get_key(aidsware::ui::get_keybind(xorstr_("peek assist key"))) && !settings::instakill)
			other::find_manipulate_angle();
		else
			if (!other::m_manipulate.empty())
				other::m_manipulate = Vector3::Zero();

		if (aidsware::ui::get_bool(xorstr_("peek assist")) && target_ply != nullptr && target_ply->isCached() && aidsware::ui::get_bool(xorstr_("autoshoot")) && !settings::instakill)
			if (other::can_manipulate()) {
				other::find_manipulate_angle();
				settings::tr::manipulate_visible = true;
			}
			else {
				settings::tr::manipulate_visible = false;
			}

		if (aidsware::ui::get_bool(xorstr_("long neck")) && get_key(aidsware::ui::get_keybind(xorstr_("desync on key")))) {
			float desyncTime = (Time::realtimeSinceStartup() - plly->lastSentTickTime()) - 0.03125 * 3;
			float max_eye_value = (0.1f + ((desyncTime + 2.f / 60.f + 0.125f) * 1.5f) * plly->MaxVelocity()) - 0.05f;

			plly->eyes()->viewOffset() = Vector3(0, max_eye_value, 0);
		}
		else if (aidsware::ui::get_bool(xorstr_("long neck")))
		{
			plly->eyes()->viewOffset() = Vector3(0, 1.495f, 0);
		}

		Physics::IgnoreLayerCollision(4, 12, !aidsware::ui::get_bool(xorstr_("no collisions")));
		Physics::IgnoreLayerCollision(30, 12, aidsware::ui::get_bool(xorstr_("no collisions")));
		Physics::IgnoreLayerCollision(11, 12, aidsware::ui::get_bool(xorstr_("no collisions")));

		if (get_key(aidsware::ui::get_keybind(xorstr_("zoom key"))))
			ConVar::Graphics::_fov( ) = 15.f;
		else
			ConVar::Graphics::_fov( ) = aidsware::ui::get_float(xorstr_("fov"));

		if (aidsware::ui::get_bool(xorstr_("fake admin")))
			plly->playerFlags( ) |= PlayerFlags::IsAdmin;

		if (aidsware::ui::get_bool(xorstr_("test")))
			LocalPlayer::Entity()->add_modelstate_flag(ModelState::Flags::Mounted);

		if (aidsware::ui::get_bool(xorstr_("can hold items")))
			if (plly->mounted( ))
				plly->mounted( )->canWieldItems( ) = true;

		if (aidsware::ui::get_combobox(xorstr_("light")) != 0) {
			auto list = TOD_Sky::instances( );
			if (list) {
				for (int j = 0; j < list->size; j++) {
					auto sky = (TOD_Sky*)list->get(j);
					if (!sky)
						continue;

					float amb = 1.f;
					if (aidsware::ui::get_combobox(xorstr_("light")) == 1)
						amb = 4.f;
					else if (aidsware::ui::get_combobox(xorstr_("light")) == 2)
						amb = 6.f;

					sky->Day( )->AmbientMultiplier( ) = amb == 4.f ? 0.2f : 1.f;
					sky->Night( )->AmbientMultiplier( ) = amb;
				}
			}
		}
	}

	plly->ClientInput(state);

	// before network 

	if (aidsware::ui::get_bool(xorstr_("omnisprint")))
		LocalPlayer::Entity( )->add_modelstate_flag(ModelState::Flags::Sprinting);
}

void DoMovement_hk(Projectile* pr, float deltaTime) {
	if (pr->isAuthoritative( ))
		if (aidsware::ui::get_bool(xorstr_("hitbox attraction")) || aidsware::ui::get_bool(xorstr_("fat bullet")))
			pr->thickness( ) = 1.f;
		else
			pr->thickness( ) = 0.1f;

	return pr->DoMovement(deltaTime);
}

float GetRandomVelocity_hk(ItemModProjectile* self) {
	float modifier = 1.f;

	if (aidsware::ui::get_bool(xorstr_("fast bullets")))
		modifier += 0.48f;

	return self->GetRandomVelocity( ) * modifier;
}

void ProcessAttack_hk(BaseMelee* self, HitTest* hit) {
	auto entity = hit->HitEntity( );

	//if (target_ply != nullptr) {
	//	auto l1 = target_ply->playerModel( )->_multiMesh( )->Renderers( );
	//	if (l1) {
	//		for (int i = 0; i < l1->size; i++) {
	//			auto rend = (Renderer_*)l1->get(i);
	//			if (!rend)
	//				continue;

	//			std::cout << *reinterpret_cast<uintptr_t*>(rend->material( ) + 0xB0) << std::endl;
	//			std::cout << reinterpret_cast<uintptr_t>(rend->material( )->shader( )) << std::endl << std::endl;

	//			if (plusminus::ui::get_float(xorstr_("target fov")) > 1000) {
	//				auto list = *reinterpret_cast<Array<Material*>**>(rend + 0x140);
	//				if (list) {
	//					for (int j = 0; j < list->size( ); j++) {
	//						auto g = list->get(j);
	//						if (!g)
	//							continue;

	//						*reinterpret_cast<Material**>(g) = nullptr;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	if (!aidsware::ui::get_bool(xorstr_("farm assist")) || !entity)
		return self->ProcessAttack(hit);

	if (entity->class_name_hash( ) == STATIC_CRC32("OreResourceEntity")) {
		BaseNetworkable* marker = BaseNetworkable::clientEntities( )->FindClosest(STATIC_CRC32("OreHotSpot"), entity, 5.0f);
		if (marker) {
			entity = marker;
			hit->HitTransform( ) = marker->transform( );
			hit->HitPoint( ) = marker->transform( )->InverseTransformPoint(marker->transform( )->position( ));
			hit->HitMaterial( ) = String::New(xorstr_("MetalOre"));
		}
	}
	else if (entity->class_name_hash( ) == STATIC_CRC32("TreeEntity")) {
		BaseNetworkable* marker = BaseNetworkable::clientEntities( )->FindClosest(STATIC_CRC32("TreeMarker"), entity, 5.0f);
		if (marker) {
			hit->HitTransform( ) = marker->transform( );
			hit->HitPoint( ) = marker->transform( )->InverseTransformPoint(marker->transform( )->position( ));
			hit->HitMaterial( ) = String::New(xorstr_("Wood"));
		}
	}

	return self->ProcessAttack(hit);
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
	if (aidsware::ui::get_bool(xorstr_("pierce"))) {
		if (prj->isAuthoritative( )) {
			auto lol = test->HitEntity( );
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
					|| lol->class_name_hash( ) == STATIC_CRC32("RHIB")) {
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
	if (optiom->IsFromServer( )) {
		if (str->buffer) {
			auto string = std::wstring(str->buffer);
			if (string.find(wxorstr_(L"noclip")) != std::wstring::npos ||
				string.find(wxorstr_(L"debugcamera")) != std::wstring::npos ||
				string.find(wxorstr_(L"admintime")) != std::wstring::npos ||
				string.find(wxorstr_(L"client.camlerp")) != std::wstring::npos ||
				string.find(wxorstr_(L"client.camspeed")) != std::wstring::npos) {

				str = String::New(xorstr_(""));
			}
		}
	}

	return ConsoleSystem::Run(optiom, str, args);
}

void set_flying_hk(ModelState* modelState, bool state) {
	modelState->set_flying(false);
}
/*
void FinalizeTick(float deltaTime)
{
	tickDeltaTime += deltaTime;
	bool flag = tickInterpolator.startPoint != tickInterpolator.endPoint;
	if (flag)
	{
		if (antihack::ValidateMove(tickDeltaTime)
			&& aidsware::ui::get_bool(xorstr_("flyhack stop")) || aidsware::ui::get_bool(xorstr_("flyhack indicator")))
		{
			//printf("GOOD\n");
		}
		else
		{
			//printf("BAD\n");
		}
	}
	tickInterpolator.Reset(LocalPlayer::Entity()->eyes()->transform()->position());
}*/
/*
void ServerUpdate(float deltaTime, BasePlayer* ply)
{
	ply->desyncTimeRaw = MAX(ply->lastSentTickTime() - deltaTime, 0.f);
	ply->desyncTimeClamped = MAX(ply->desyncTimeRaw, 1.f);
	printf("ply->desyncTimeRaw: %.3f\n", ply->desyncTimeRaw);
	printf("ply->desyncTimeClamped: %.3f\n", ply->desyncTimeClamped);
	FinalizeTick(deltaTime, ply);
	return;
}
*/

int jitter = 1;
int jitter_speed = 10;
int spin_speed = 70;
int spin = 0;

void sendclienttick_hk(BasePlayer* self)
{
	printf("jitter: %i\n", jitter);
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
		break;
	case 1: //backwards
		spin_angles.y = real_angles.y - 180.f;
		break;
	case 2: //backwards (down)
		spin_angles.x = 0.f;
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y - 180.f;
		break;
	case 3: //backwards (up)
		spin_angles.x = 999.f;
		spin_angles.z = 999.f;
		spin_angles.y = real_angles.y - 180.f;
		break;
	case 4: //left
		spin_angles.y = real_angles.y - 90.f;
		break;
	case 5: //left (down)
		spin_angles.x = 0.f;
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y - 90.f;
		break;
	case 6: //left (up)
		spin_angles.x = 999.f;
		spin_angles.z = 999.f;
		spin_angles.y = real_angles.y - 90.f;
		break;
	case 7: //right
		spin_angles.y = real_angles.y + 90.f;
		break;
	case 8: //right (down)
		spin_angles.x = 0.f;
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y + 90.f;
		break;
	case 9: //right (up)
		spin_angles.x = 999.f;
		spin_angles.z = 999.f;
		spin_angles.y = real_angles.y + 90.f;
		break;
	case 10: //jitter
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + 45.f;
		}
		else if(jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y - 45.f;
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y - 180.f;
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.y = real_angles.y;
		break;
	case 11: //jitter (down)
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + 45.f;
		}
		else if (jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y - 45.f;
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y - 180.f;
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.x = 0.f;
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y;
		break;
	case 12: //jitter (up)
		if (jitter <= jitter_speed * 1)
		{
			spin_angles.y = real_angles.y + 45.f;
		}
		else if(jitter <= jitter_speed * 2)
		{
			spin_angles.y = real_angles.y - 45.f;
		}
		else if (jitter <= jitter_speed * 3)
		{
			spin_angles.y = real_angles.y - 180.f;
			jitter = 1;
		}
		jitter = jitter + 1;
		spin_angles.x = 999.f;
		spin_angles.z = 999.f;
		spin_angles.y = real_angles.y;
		break;
	case 13: //spin
		spin_angles.y = real_angles.y + (spin_speed * spin++);
		if (spin > (360 / spin_speed))
			spin = 1;
		break;
	case 14: //spin (down)
		spin_angles.x = 0.f;
		spin_angles.z = 0.f;
		spin_angles.y = real_angles.y + (spin_speed * spin++);
		if (spin > (360 / spin_speed))
			spin = 1;
		break;
	case 15: //spin (up)
		spin_angles.x = 999.f;
		spin_angles.y = real_angles.y + (spin_speed * spin++);
		spin_angles.z = 999.f;
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


	cLastTickPos = self->transform()->position();
	return;
}

Vector3 playereyes_getpos_hk(PlayerEyes* self)
{
	if (settings::peek_insta)
		return self->get_position() + other::m_manipulate;
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
	if (!target_ply) return p;

	return p;
}

void do_hooks( ) {
	VM_DOLPHIN_BLACK_START

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


	VM_DOLPHIN_BLACK_END
}

void undo_hooks( ) {
	VM_DOLPHIN_BLACK_START
	hookengine::unhook(BasePlayer::ClientUpdate_, ClientUpdate_hk);
	hookengine::unhook(PlayerWalkMovement::UpdateVelocity_, UpdateVelocity_hk);
	hookengine::unhook(PlayerWalkMovement::HandleJumping_, HandleJumping_hk);
	hookengine::unhook(BasePlayer::CanAttack_, CanAttack_hk);
	hookengine::unhook(BasePlayer::OnLand_, OnLand_hk);
	hookengine::unhook(Projectile::DoMovement_, DoMovement_hk);
	hookengine::unhook(FlintStrikeWeapon::DoAttack_, DoAttack_hk);
	hookengine::unhook(ViewmodelBob::Apply_, BobApply_hk);
	hookengine::unhook(ViewmodelSway::Apply_, SwayApply_hk);
	hookengine::unhook(InputState::IsDown_, IsDown_hk);
	hookengine::unhook(BaseCombatEntity::OnAttacked_, OnAttacked_hk);
	hookengine::unhook(ConsoleSystem::Run_, ConsoleRun_hk);
	hookengine::unhook(ViewmodelLower::Apply_, LowerApply_hk);
	hookengine::unhook(ModelState::set_flying_, set_flying_hk);
	hookengine::unhook(HitTest::BuildAttackMessage_, BuildAttackMessage_hk);
	hookengine::unhook(BaseMelee::ProcessAttack_, ProcessAttack_hk);
	hookengine::unhook(Projectile::DoHit_, DoHit_hk);
	hookengine::unhook(BaseMountable::EyePositionForPlayer_, EyePositionForPlayer_hk);
	hookengine::unhook(MonoBehaviour::StartCoroutine_, StartCoroutine_hk);
	hookengine::unhook(Projectile::SetEffectScale_, SetEffectScale_hk);
	hookengine::unhook(BasePlayer::ClientInput_, ClientInput_hk);
	hookengine::unhook(ItemModProjectile::GetRandomVelocity_, GetRandomVelocity_hk);
	hookengine::unhook(PlayerEyes::BodyLeanOffset_, BodyLeanOffset_hk);
	hookengine::unhook(AimConeUtil::GetModifiedAimConeDirection_, GetModifiedAimConeDirection_hk);
	hookengine::unhook(PlayerEyes::DoFirstPersonCamera_, DoFirstPersonCamera_hk);
	hookengine::unhook(Vector3_::MoveTowards_, MoveTowards_hk);
	hookengine::unhook(HeldEntity::AddPunch_, AddPunch_hk);

	hookengine::unhook(BasePlayer::SendClientTick_, sendclienttick_hk);

	hookengine::unhook(PlayerEyes::get_position_, playereyes_getpos_hk);

	hookengine::unhook(Projectile::Launch_, Launch_hk);

	hookengine::unhook(BaseCombatEntity::DoHitNotify_, DoHitNotify_hk);

	hookengine::unhook(BaseProjectile::CreateProjectile_, CreateProjectile_hk);

	VM_DOLPHIN_BLACK_END
}