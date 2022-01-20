namespace other {
	Vector3 m_manipulate = Vector3::Zero( );

	bool TestNoClipping(BasePlayer* ply, Vector3 oldPos, Vector3 newPos, bool sphereCast, float deltaTime = 0.f)
	{
		float nbacktrack = 0.01f;
		float nmargin = 0.09f;
		float radius = ply->GetRadius();
		float height = ply->GetHeight();
		Vector3 normalized = (newPos - oldPos).normalized();
		float num2 = radius - nmargin;
		Vector3 vector = oldPos + Vector3(0.f, height - radius, 0.f) - normalized * nbacktrack;
		float magnitude = (newPos + Vector3(0.f, height - radius, 0.f) - vector).magnitude();
		RaycastHit hitInfo;

		Ray z = Ray(vector, normalized);

		bool flag = Physics::Raycast(z, magnitude + num2, 429990145);
		if (!flag)
		{
			flag = Physics::SphereCast(z, num2, magnitude, 429990145);
		}
		//return false;g
		return flag;//&& GamePhysics::Verify(&hitInfo);
	}

	bool ValidateEyePos(Vector3 position)
	{
		//protection > 3
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

		float num8 = std::fabs(loco->GetParentVelocity().y);
		float num9 = loco->BoundsPadding() + num4 + num8 + loco->GetJumpHeight();
		float num10 = std::fabs(loco->eyes()->get_position().y - position.y);

		if (num10 > num9)
		{
			if (aidsware::ui::get_bool(xorstr_("debug")))
				printf("position: (%ff, %ff, %ff) caused eye_altitude!\n",
					position.x, position.y, position.z);
			flag = true; //EYE_ALTITUDE
		}

		//protection > 4
		Vector3 position2 = LocalPlayer::Entity()->transform()->position();
		Vector3 vector = position - Vector3(0.f, 1.5f, 0.f) - Vector3(0.f, -0.6f, 0.f);
		
		if (vector.distance(position2) > 0.01f
			&& TestFlying2(loco, position2, vector, true))
		{
			if (aidsware::ui::get_bool(xorstr_("debug")))
				printf("position: (%ff, %ff, %ff) caused eye_noclip!\n",
					position.x, position.y, position.z);

			flag = true; //EYE_NOCLIP
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

	std::vector<Vector3> ext = { Vector3(0.752444, -2.067324, 0.000000), Vector3(0.576406, -2.067324, 0.483662), Vector3(0.130661, -2.067324, 0.741013), Vector3(-0.376222, -2.067324, 0.651636), Vector3(-0.707066, -2.067324, 0.257351), Vector3(-0.707066, -2.067324, -0.257351), Vector3(-0.376222, -2.067324, -0.651636), Vector3(0.130661, -2.067324, -0.741013), Vector3(0.576406, -2.067324, -0.483662), Vector3(0.752444, -2.067324, 0.000000), Vector3(1.414133, -1.685298, 0.000000), Vector3(1.083289, -1.685298, 0.908987), Vector3(0.245561, -1.685298, 1.392649), Vector3(-0.707066, -1.685298, 1.224675), Vector3(-1.328850, -1.685298, 0.483662), Vector3(-1.328850, -1.685298, -0.483662), Vector3(-0.707066, -1.685298, -1.224675), Vector3(0.245562, -1.685298, -1.392649), Vector3(1.083289, -1.685298, -0.908987), Vector3(1.414133, -1.685298, 0.000000), Vector3(1.905256, -1.100000, 0.000000), Vector3(1.459511, -1.100000, 1.224675), Vector3(0.330844, -1.100000, 1.876311), Vector3(-0.952628, -1.100000, 1.650000), Vector3(-1.790355, -1.100000, 0.651636), Vector3(-1.790355, -1.100000, -0.651636), Vector3(-0.952628, -1.100000, -1.650000), Vector3(0.330845, -1.100000, -1.876311), Vector3(1.459511, -1.100000, -1.224675), Vector3(1.905256, -1.100000, 0.000000), Vector3(2.166577, -0.382026, 0.000000), Vector3(1.659694, -0.382026, 1.392649), Vector3(0.376222, -0.382026, 2.133662), Vector3(-1.083289, -0.382026, 1.876311), Vector3(-2.035917, -0.382026, 0.741013), Vector3(-2.035916, -0.382026, -0.741014), Vector3(-1.083288, -0.382026, -1.876311), Vector3(0.376223, -0.382026, -2.133662), Vector3(1.659695, -0.382026, -1.392648), Vector3(2.166577, -0.382026, 0.000000), Vector3(2.166577, 0.382026, 0.000000), Vector3(1.659694, 0.382026, 1.392649), Vector3(0.376222, 0.382026, 2.133662), Vector3(-1.083289, 0.382026, 1.876311), Vector3(-2.035917, 0.382026, 0.741013), Vector3(-2.035916, 0.382026, -0.741014), Vector3(-1.083288, 0.382026, -1.876311), Vector3(0.376223, 0.382026, -2.133662), Vector3(1.659695, 0.382026, -1.392648), Vector3(2.166577, 0.382026, 0.000000), Vector3(1.905256, 1.100000, 0.000000), Vector3(1.459511, 1.100000, 1.224675), Vector3(0.330844, 1.100000, 1.876311), Vector3(-0.952628, 1.100000, 1.650000), Vector3(-1.790355, 1.100000, 0.651636), Vector3(-1.790355, 1.100000, -0.651636), Vector3(-0.952628, 1.100000, -1.650000), Vector3(0.330845, 1.100000, -1.876311), Vector3(1.459511, 1.100000, -1.224674), Vector3(1.905256, 1.100000, 0.000000), Vector3(1.414132, 1.685298, 0.000000), Vector3(1.083288, 1.685298, 0.908987), Vector3(0.245561, 1.685298, 1.392649), Vector3(-0.707066, 1.685298, 1.224675), Vector3(-1.328850, 1.685298, 0.483662), Vector3(-1.328850, 1.685298, -0.483662), Vector3(-0.707066, 1.685298, -1.224675), Vector3(0.245562, 1.685298, -1.392648), Vector3(1.083289, 1.685298, -0.908987), Vector3(1.414132, 1.685298, 0.000000), Vector3(0.752444, 2.067324, 0.000000), Vector3(0.576406, 2.067324, 0.483662), Vector3(0.130660, 2.067324, 0.741013), Vector3(-0.376222, 2.067324, 0.651636), Vector3(-0.707066, 2.067324, 0.257351), Vector3(-0.707066, 2.067324, -0.257351), Vector3(-0.376222, 2.067324, -0.651636), Vector3(0.130661, 2.067324, -0.741013), Vector3(0.576406, 2.067324, -0.483662), Vector3(0.752444, 2.067324, 0.000000), Vector3(-0.000000, 2.200000, -0.000000) };

	void get_sphere_points_z(std::vector<Vector3>& re, float radius = 2.2f, unsigned int sectors = 20, unsigned int rings = 20)
	{
		float const R = 1. / (float)(rings - 1);
		float const S = 1. / (float)(sectors - 1);
		int r, s;

		for (r = 0; r < rings; r++)
			for (s = 0; s < sectors; s++)
			{
				float y = sin(-(M_PI / 2) + M_PI * r * R);
				float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
				float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

				x *= radius;
				y *= radius;
				z *= radius;

				re.push_back(Vector3(x, y, z));
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

	bool manipulate_fat = false;
	Vector3 fat_target = Vector3::Zero();

	void find_manipulate_angle(float desyncTime, Vector3 target_pos) {
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
		std::vector<Vector3> fat = {};

		float _fa = aidsware::ui::get_float(xorstr_("checks")) / 4.0f;

		get_sphere_points(mm_max_eye, _fa, arr);
		//fat = ext;

		bool fatq = false;
		Vector3 fat_choice = Vector3::Zero();

		for (auto s : arr) {
			Vector3 point = re_p + s;

			if (!LineOfSight(point, re_p))
				continue;

			if (aidsware::ui::get_bool(xorstr_("show peek assist checks")))
			{
				DDraw::Sphere(point, 0.05f, Color::Color(52.f / 255.f, 100.f / 255.f, 235.f / 255.f, 1.f), aidsware::ui::get_float("duration"), true);
			}

			if (!target_ply->bones()->head->visible_(point))
			{
				/*
				//check fat bullet?
				if (aidsware::ui::get_bool(xorstr_("fat bullet"))) {
					for (auto z : fat)
					{
						DDraw::Sphere(target_pos + z, 0.05f, Color::Color(52.f / 255.f, 100.f / 255.f, 235.f / 255.f, 1.f), 0.02f, true);

						if (LineOfSight(point, target_pos + z))
						{
							fat_choice = s;
							other::fat_target = target_pos + z;
							fatq = true; //can see their fatbullet radius
							break;
						}
					}
				}
				
				if(!fatq) */continue;
			}

			if (ValidateEyePos(point))
				continue;

			if (aidsware::ui::get_bool(xorstr_("show checks"))) {
				DDraw::Sphere(point, 0.05f, Color::Color(0.f, 1.f, 60.f / 255.f, 1.f), aidsware::ui::get_float("duration") + 10.f, true);
			}

			choice = s;
			break;
		}
		
		if (choice.empty( )) {
			if (fatq)
			{
				other::manipulate_fat = true;
				m_manipulate = fat_choice;
				return;
			}
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