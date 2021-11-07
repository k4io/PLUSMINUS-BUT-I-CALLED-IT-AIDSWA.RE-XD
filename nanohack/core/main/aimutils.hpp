namespace aimutils {
	double get_bullet_drop(double height, double aaaa, float speed, float gravity) {
		double pitch = std::atan2(height, aaaa);
		double vel_double = speed * std::cos(pitch);
		double t = aaaa / vel_double;
		double y = (0.4905f * gravity * t * t);
		return y * 10;
	}
	float SimulateTravelTime(Vector3& position, Vector3& velocity, float& partialTime, float travelTime, Vector3 gravity, float drag)
	{
		//float timestep = 0.03125f;
		float timestep = 0.015625f;
		Vector3 origin = position;
		if (partialTime > 0)
		{
			float num2 = timestep - partialTime;
			if (travelTime < num2)
			{
				origin = position;
				position += velocity * travelTime;
				partialTime += travelTime;
				return partialTime;
			}
			origin = position;
			position += velocity * num2;
			velocity += gravity * timestep;
			velocity -= velocity * drag * timestep;
			travelTime -= num2;
		}

		float f = std::floor(travelTime / timestep);
		int num3 = static_cast<float>(f);

		for (int i = 0; i < num3; i++)
		{
			origin = position;
			position += velocity * timestep;
			velocity += gravity * timestep;
			velocity -= velocity * drag * timestep;
		}
		partialTime = travelTime - timestep * (float)num3;
		if (partialTime > 0)
		{
			origin = position;
			position += velocity * partialTime;
		}
		return partialTime;
		//return origin;
	}

	float max(float a, float b) { return a > b ? a : b; }

	Vector3 get_prediction( ) {
		auto mpv = target_ply->find_mpv_bone();
		Vector3 target;
		if (mpv != nullptr)
			target = mpv->position;
		else
			target = target_ply->bones()->head->position;

		Vector3 targetvel = target_ply->playerModel()->newVelocity();

		auto base_projectile = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
		if (base_projectile == nullptr)
			return target;

		static Type* type = Type::GetType(xorstr_("ItemModProjectile, Assembly-CSharp"));
		if (type == nullptr)
			return target;

		auto mag = base_projectile->primaryMagazine();
		if (mag == nullptr)
			return target;

		auto ammo = mag->ammoType();
		if (ammo == nullptr)
			return target;

		auto itemModProjectile = ammo->GetComponent<ItemModProjectile>(type); // 0x3189118 for getting Projectile* ref
		if (itemModProjectile == nullptr)
			return target;

		float bullet_speed = (itemModProjectile->GetRandomVelocity() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.f)) * base_projectile->projectileVelocityScale();

		if (base_projectile->class_name_hash() == STATIC_CRC32("CompoundBowWeapon"))
			bullet_speed = (itemModProjectile->GetRandomVelocity() * (aidsware::ui::get_bool(xorstr_("fast bullets")) ? 1.48f : 1.f)) * reinterpret_cast<CompoundBowWeapon*>(base_projectile)->GetProjectileVelocityScale();

		if (bullet_speed == 0.f)
			return target;

		Projectile* projectile = itemModProjectile->projectileObject()->Get()->GetComponent<Projectile>(Type::Projectile());

		if (projectile == nullptr)
			return target;
		/*
		TraceResult f = traceProjectile(LocalPlayer::Entity()->eyes()->get_position(),
			projectile->initialVelocity(),
			projectile->drag(),
			Vector3(0, -9.1 * projectile->gravityModifier(), 0),
			target);

		LogSystem::AddTraceResult(f);
		*/
		float distance = target.distance(LocalPlayer::Entity()->eyes()->position());
		//float distance = f.hitDist;

		bullet_speed *= 1.f - 0.015625f * projectile->drag();

		float travel_time = distance / bullet_speed;
		//float travel_time = f.hitTime;

		Vector3 vel = Vector3(targetvel.x, 0, targetvel.z) * 0.75f;
		//Vector3 vel = target_ply->playerModel()->velocity();
		Vector3 predicted_velocity = vel * travel_time;
		//Vector3 predicted_velocity = f.outVelocity;

		target.x += predicted_velocity.x;
		target.z += predicted_velocity.z;
		double height = target.y - LocalPlayer::Entity()->eyes()->position().y;
		Vector3 dir = target - LocalPlayer::Entity()->eyes()->position();
		float astronaut = sqrt((dir.x * dir.x) + (dir.z * dir.z));
		float drop = get_bullet_drop(height, astronaut, bullet_speed, projectile->gravityModifier());
		target.y += drop;

		return target;
	}
}