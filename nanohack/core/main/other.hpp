namespace other {
	Vector3 m_manipulate = Vector3::Zero( );

	bool ValidateEyePos(Vector3 position)
	{
		bool flag = false;
		auto loco = LocalPlayer::Entity();
		float num = 1.5f;
		float clientframes = 2.f;
		float serverframes = 2.f;
		float num2 = clientframes / 60.f;
		float deltatime = Time::deltaTime();
		float smooth_Deltatime = Time::smoothDeltaTime();
		float fixed_Deltatime = Time::fixedDeltaTime();
		float num3 = serverframes * MAX(deltatime, MAX(smooth_Deltatime, fixed_Deltatime));
		float num4 = (1.f + num2 + num3) * num;
		float num5 = loco->MaxVelocity() + loco->GetParentVelocity().magnitude();
		float num6 = loco->BoundsPadding() + num4 * num5;
		float num7 = loco->eyes()->position().distance(position);

		float num8 = std::abs(loco->GetParentVelocity().y);
		float num9 = loco->BoundsPadding() + num4 + num8 + loco->GetJumpHeight();
		float num10 = std::abs(loco->eyes()->get_position().y - position.y);

		if (num10 > num9)
		{
			flag = true; //EYE_ALTITUDE
		}
		return flag;
	}

	bool ValidateEyePos_Old(Vector3 position) //returns true if invalid
	{
		auto loco = LocalPlayer::Entity();
		bool flag = false;
		float num = 1.5f;
		float clientframes = 2.f;
		float serverframes = 2.f;
		float num2 = clientframes / 60.f;
		float deltatime = Time::deltaTime();
		float smooth_Deltatime = Time::smoothDeltaTime();
		float fixed_Deltatime = Time::fixedDeltaTime();
		float num3 = serverframes * MAX(deltatime, MAX(smooth_Deltatime, fixed_Deltatime));
		float num4 = (1.f + num2 + num3) * num;
		float num5 = loco->MaxVelocity() + loco->GetParentVelocity().magnitude();
		float num6 = loco->BoundsPadding() + num4 * num5;
		float num7 = loco->eyes()->position().distance(position);

		if (num7 > num6)
			flag = true;

		/*
		if (aidsware::ui::get_bool("debug"))
		{
			printf("num2: %ff\n", num2);
			printf("deltatime: %ff\n", deltatime);
			printf("smooth_Deltatime: %ff\n", smooth_Deltatime);
			printf("fixed_Deltatime: %ff\n", fixed_Deltatime);
			printf("num3: %ff\n", num3);
			printf("num4: %ff\n", num4);
			printf("num6: %ff\n", num6);
			printf("num7: %ff\n", num7);
			printf("flag: %s\n---------------------\n", (flag ? "true" : "false"));
		}
		*/

		return flag;
	}

	void get_sphere_points(float radius, unsigned int sectors, std::vector<Vector3>& re, float max = 1.6f)
	{
		for (float y = -1.6f; y < 1.6f; y += 0.1f) {
			int points = sectors;
			float step = (M_PI_2) / points;
			float x, z, current = 0;
			for (size_t i = 0; i < points; i++)
			{
				x = sin(current) * radius;
				z = cos(current) * radius;

				re.push_back(Vector3(x, y, z));
				re.push_back(Vector3(-x, y, z));
				re.push_back(Vector3(x, y, -z));
				re.push_back(Vector3(-x, y, -z));

				current += step;
			}
		}
	}

	bool can_manipulate()
	{
		auto loco = LocalPlayer::Entity();

		if (!loco) return false;
		if (!target_ply) return false;

		auto held = loco->GetHeldEntity<BaseProjectile>();

		if (!held) return false;

		if (held->Empty() && (held->class_name_hash() == STATIC_CRC32("BaseProjectile")
			|| held->class_name_hash() == STATIC_CRC32("BowWeapon")
			|| held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon")
			|| held->class_name_hash() == STATIC_CRC32("BaseLauncher")
			|| held->class_name_hash() == STATIC_CRC32("CrossbowWeapon")))
		{
			return false;
		}

		Vector3 re_p = loco->transform()->position() + loco->transform()->up() * (PlayerEyes::EyeOffset().y + loco->eyes()->viewOffset().y);

		Vector3 choice = Vector3::Zero();

		if (LineOfSight(re_p, target_ply->find_mpv_bone()->position) || !target_ply->isCached()) {
			return true;
		}

		float desyncTime = (Time::realtimeSinceStartup() - loco->lastSentTickTime()) - 0.03125 * 3;
		//float mm_max_eye = (0.1f + ((desyncTime + 2.f / 60.f + 0.125f) * 1.5f) * loco->MaxVelocity()) - 0.05f;
		float mm_max_eye = aidsware::ui::get_float(xorstr_("max radius"));

		auto _right = loco->eyes()->MovementRight();
		auto forward = loco->eyes()->MovementForward();

		std::vector<Vector3> arr = {};

		if (aidsware::ui::get_bool(xorstr_("with peek assist")))
			mm_max_eye = 8.f;
		if (loco->in_minicopter())
			mm_max_eye = 75.f;
		if (loco->on_horse())
			mm_max_eye = 25.f;
		//autoshoot max visible check = 10 meter

		float _fa = aidsware::ui::get_float(xorstr_("checks")) / 4.0f;
		
		get_sphere_points(mm_max_eye, _fa, arr);

		for (auto s : arr) {
			Vector3 point = re_p + s;
			if (!LineOfSight(point, re_p))
				continue;

			if (aidsware::ui::get_bool(xorstr_("show peek assist checks")))
				DDraw::Sphere(point, 0.05f, Color::Color(52.f / 255.f, 100.f / 255.f, 235.f / 255.f, 1.f), aidsware::ui::get_float("duration"), true);

			if (!target_ply->bones()->head->visible_(point))
				continue;

			if (aidsware::ui::get_bool(xorstr_("with peek assist")) 
				&& (mm_max_eye == 8.f || mm_max_eye == 25.f || mm_max_eye == 75.f))
				settings::can_insta = true;
			else settings::can_insta = false;

			choice = s;
			break;
		}
		if (choice.empty()) {
			return false;
		}
		return true;
	}

	void find_manipulate_angle(float desyncTime) {
		auto loco = LocalPlayer::Entity( );
		auto held = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();
		loco->modelState()->set_mounted(true);
		Vector3 re_p = loco->transform( )->position( ) + loco->transform( )->up( ) * (PlayerEyes::EyeOffset( ).y + loco->eyes( )->viewOffset( ).y);
		
		Vector3 choice = Vector3::Zero( );

		if (LineOfSight(re_p, target_ply->find_mpv_bone( )->position) || !target_ply->isCached()) {
			m_manipulate = Vector3::Zero( );
			return;
		}

		//float desyncTime = (Time::realtimeSinceStartup( ) - loco->lastSentTickTime( )) - 0.03125 * 3;
		float mm_max_eye = ((0.1f + ((desyncTime + 2.f / 60.f + 0.125f) * 1.5f) * loco->MaxVelocity( )));

		std::vector<Vector3> arr = {};

		float _fa = aidsware::ui::get_float(xorstr_("checks")) / 4.0f;

		get_sphere_points(mm_max_eye, _fa, arr);
		
		for (auto s : arr) {
			Vector3 point = re_p + s;

			if (!LineOfSight(point, re_p))
				continue;

			if (aidsware::ui::get_bool(xorstr_("show peek assist checks")))
				DDraw::Sphere(point, 0.05f, Color::Color(52.f / 255.f, 100.f / 255.f, 235.f / 255.f, 1.f), aidsware::ui::get_float("duration"), true);

			if (!target_ply->bones( )->head->visible_(point))
				continue;

			if (ValidateEyePos(point))
				continue;

			if (aidsware::ui::get_bool(xorstr_("show checks"))) {
				DDraw::Sphere(point, 0.05f, Color::Color(0.f, 1.f, 60.f / 255.f, 1.f), aidsware::ui::get_float("duration") + 10.f, true);
			}

			choice = s;
			break;
		}
		
		if (choice.empty( )) {
			m_manipulate = Vector3::Zero( );
			return;
		}

		m_manipulate = choice;
	}

	void test_bundle(AssetBundle* bundle) {
		if (!bundle) {
			std::cout << "bundle nfound\n";
			return;
		}

		auto arr = bundle->GetAllAssetNames( );
		if (!arr) {
			std::cout << "arr nfound\n";
			return;
		}

		for (int j = 0; j < arr->size( ); j++) {
			auto name = arr->get(j);

			printf("%ls\n", name->buffer);
		}

		std::cout << "bundletest - success\n";
	}
}
void dispatch_keybind(KeyCode& s) {

}