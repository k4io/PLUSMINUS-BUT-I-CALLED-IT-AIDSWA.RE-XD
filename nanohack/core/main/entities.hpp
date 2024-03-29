namespace entities {

	Shader* og_shader = nullptr;
	std::vector<BasePlayer*> current_visible_players;
	BaseEntity* closest_node;
	bool new_esp = false;

	struct slave
	{
		std::string forum_name;
		std::string steam_name;
		std::string steam_id;
		std::string server_ip;
		int sockfd;
	};

	std::vector<slave> slaves{};

	float t_TimeSinceLastShot = -0.1f;

	uint64_t target_id = 0;
	uint64_t friend_id = 0;
	uint64_t master_id = 0;
	Vector3 walk_to_pos = Vector3::Zero();
	Vector3 closest_peek_pos = Vector3::Zero();
	Vector3 aim_angles = Vector3::Zero();

	BasePlayer* master = nullptr;

	inline bool exists(const std::string& name) {
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}
	namespace belt {
		Vector2 pos = Vector2(200, 200);
		bool should_drag = false;
		bool should_move = false;

		POINT cursor;
		POINT cursor_corrected;
		void belt_tab_mov(Vector2 size) {
			GetCursorPos(&cursor);

			if (GetAsyncKeyState(VK_LBUTTON) && (cursor.x > pos.x && cursor.y > pos.y && cursor.x < pos.x + size.x && cursor.y < pos.y + size.y)) {
				should_drag = true;
				if (!should_move) {
					cursor_corrected.x = cursor.x - pos.x;
					cursor_corrected.y = cursor.y - pos.y;
					should_move = true;
				}
			}

			if (should_drag) {
				pos.x = cursor.x - cursor_corrected.x;
				pos.y = cursor.y - cursor_corrected.y;
			}

			if (GetAsyncKeyState(VK_LBUTTON) == 0) {
				should_drag = false;
				should_move = false;
			}
		}
	}

	float dfc(BaseEntity* entity, Vector2 p)
	{
		if (!entity) return 1000.f;
		return screen_center.distance_2d(p);
	}

	float dfc(BasePlayer* player) {
		if (!player)
			return 1000.f;

		if (!player->isCached())
			return 1000.f;

		if (player->bones()->dfc.empty())
			return 1000.f;

		return screen_center.distance_2d(player->bones()->dfc);
	}

	Color3 get_color(BasePlayer* player, bool boxes = false, bool flag1 = false) {
		if (aidsware::ui::get_bool("insta kill") && flag1)
			return aidsware::ui::get_color("insta kill indicator");
		if (!boxes) {
			if (player->HasPlayerFlag(PlayerFlags::Sleeping)) {
				if (player->is_visible())
					return aidsware::ui::get_color("visible sleepers");
				else
					return aidsware::ui::get_color("invisible sleepers");
			}
			if (!player->playerModel()->isNpc()) {
				if (player->is_target())
					if (player->is_visible()) {
						return aidsware::ui::get_color("visible players");
					}
					else
						return aidsware::ui::get_color("invisible players");
				else
					if (player->is_teammate())
						if (player->is_visible())
							return aidsware::ui::get_color("visible teammate");
						else
							return aidsware::ui::get_color("invisible teammate");
					else
						if (player->is_visible())
							return aidsware::ui::get_color("visible players");
						else
							return aidsware::ui::get_color("invisible players");
			}
			else {
				if (!player->is_target())
					if (player->is_visible())
						return aidsware::ui::get_color("visible npcs");
					else
						return aidsware::ui::get_color("invisible npcs");
				else
					if (player->is_visible())
						return aidsware::ui::get_color("visible npcs");
					else
						return aidsware::ui::get_color("invisible npcs");
			}
		}

		if (boxes) {
			if (player->is_target())
				return aidsware::ui::get_color("targeted boxes");
			else
				if (player->is_visible())
					return aidsware::ui::get_color("visible boxes");
				else
					return aidsware::ui::get_color("invisible boxes");
		}
	}

	float BOG_TO_GRD(float BOG) {
		return (180 / M_PI) * BOG;
	}

	float GRD_TO_BOG(float GRD) {
		return (M_PI / 180) * GRD;
	}

	__forceinline uint32_t RandomInteger(uint32_t Min, uint32_t Max)
	{
		std::random_device rd;
		std::mt19937 eng(rd());
		const std::uniform_int_distribution<uint32_t> distr(Min, Max);
		return distr(eng);
	}

	Color get_c(Color3 c) { return Color(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f); }
	//Color get_c(Color3 c) { return Color(c.r, c.g, c.b, c.a); }

	float lastChamsUpdate = 0.0f;

	Color chams_col(int c)
	{
		switch (c)
		{
		case 0:
			return get_c(aidsware::ui::get_color("visible chams"));
		case 1:
			return get_c(aidsware::ui::get_color("invisible chams"));
		}
	}

	void func(Material* s, Shader* c)
	{
		ULONG ExceptionCode = 0;
		PEXCEPTION_POINTERS ExceptionPointers = 0;
		__try
		{
			if (s)
			{
				if (c) {
					if (c != s->shader()) {
						s->set_shader(c);
					}
					else
					{
						//Color c1 = get_c(Color3(190, 190, 210));
						Color c1 = chams_col(0);
						Color c2 = chams_col(1);
						s->SetColor(xorstr_("_ColorVisible"), c1);
						s->SetColor(xorstr_("_ColorBehind"), c2);
					}
					s->SetInt(xorstr_("_ZTest"), 8);
				}
			}
		}
		__except (
			ExceptionCode = GetExceptionCode(),
			ExceptionPointers = GetExceptionInformation(),
			EXCEPTION_EXECUTE_HANDLER
			) {
			if (ExceptionPointers)
			{
				printf("Exception (%lx) caught in %s @ (%p)",
					ExceptionCode, __FUNCTION__,
					ExceptionPointers->ExceptionRecord->ExceptionAddress
				);
			}
		}
	}

	float lastbuild = 0.f;
	void do_chams(BasePlayer* player)
	{
		if (Time::fixedTime() > lastbuild + 15.f)
		{
			//player->playerModel()->RebuildAll();
			lastbuild = Time::fixedTime();
		}

		//if (!chams)
		//	chams = aw_assets->LoadAsset<Shader>(xorstr_("assets/assets/resources/chamsshader.shader"), Type::Shader());
		
		/*
		auto mesh = player->playerModel()->_multiMesh();
		if (mesh)
		{
			auto list = mesh->Renderers();
			for (size_t i = 0; i < list->size; i++)
			{
				if (i == list->size) continue;
				auto renderer = reinterpret_cast<Renderer_*>(list->get(i));
				if (renderer)
				{
					const auto material = renderer->material();
					if (material)
					{
						if (chams) {
							if (chams != material->shader()) {
								material->set_shader(chams);
							}
							
							else
							{
								//Color c1 = get_c(Color3(190, 190, 210));
								Color c1 = chams_col(0);
								Color c2 = chams_col(1);
								material->SetColor(xorstr_("_ColorVisible"), c1);
								material->SetColor(xorstr_("_ColorBehind"), c2);
							}
							material->SetInt(xorstr_("_ZTest"), 8);
						}
					}
				}
			}
		}
		*/
		
		auto list = player->playerModel()->_multiMesh()->Renderers();
		if (list) {
			for (int i = 0; i < list->size; i++) {
				if (i == list->size) continue;
				auto _renderer = reinterpret_cast<Renderer_*>(list->get(i));
				if (_renderer)
				{
					const auto material = _renderer->material();
					
					if (material)
					{
						if (og_shader != material->shader())
						{
							if (!og_shader)
								og_shader = Shader::Find(xorstr_("Hidden/Internal-Colored"));
							material->set_shader(entities::og_shader);

							auto info = player->bones()->head;
							if (player->playerModel()->isNpc() && player->is_visible())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("visible npc chams")));
								//material->SetColor(Shader::PropertyToID(xorstr_("_ColorVisible")), get_c(aidsware::ui::get_color("visible npc chams")));
							else if (player->playerModel()->isNpc() && !player->is_visible())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("invisible npc chams")));

							if (!player->playerModel()->isNpc() && player->is_visible())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("visible chams")));
							else if (!player->playerModel()->isNpc() && !player->is_visible())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("invisible chams")));

							if (!player->playerModel()->isNpc() && player->is_visible() && player->is_teammate())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("visible teammate chams")));
							else if (!player->playerModel()->isNpc() && !player->is_visible() && player->is_teammate())
								material->SetColor(Shader::PropertyToID(xorstr_("_Color")), get_c(aidsware::ui::get_color("invisible teammate chams")));

							material->SetInt(xorstr_("_ZTest"), 8); //maybe try _ZTest Less & _ZTest Greater
						}
					}
					
				}
			}
		}
	}

	bool FindPoint(int x1, int y1, int x2,
		int y2, int x, int y)
	{
		if (x > x1 and x < x2 and y > y1 and y < y2)
			return true;

		return false;
	}

	int alpha_index = -1;

	std::vector<BaseEntity*> nodelist{};

	void loop() {
		bool masterflag = false;
		static Color3 clr = Color3(RandomInteger(100, 255), RandomInteger(100, 255), RandomInteger(100, 255), 255);
		static float faken_rot = 0.0f;
		static int gamerjuice = 0;
		float a = screen_center.y / 30.0f;
		float gamma = atan(a / a);

		switch (aidsware::ui::get_combobox(xorstr_("crosshair"))) {
		case 1:
			Renderer::circle(screen_center - Vector2(2, 2), clr, 4.f, 0.75f);
			Renderer::circle(screen_center + Vector2(2, 2), Color3(219, 219, 219), 4.f, 0.75f);
			break;
		case 2:
			Renderer::line(screen_center, Vector2{ screen_center.x + 4, screen_center.y + 4 }, Color3(219, 219, 219), false, 0.7f);
			Renderer::line(screen_center, Vector2{ screen_center.x + 4, screen_center.y - 4 }, Color3(219, 219, 219), false, 0.7f);
			Renderer::line(screen_center, Vector2{ screen_center.x - 4, screen_center.y - 4 }, Color3(219, 219, 219), false, 0.7f);
			Renderer::line(screen_center, Vector2{ screen_center.x - 4, screen_center.y + 4 }, Color3(219, 219, 219), false, 0.7f);

			Renderer::line(Vector2{ screen_center.x + 4, screen_center.y + 4 }, Vector2{ screen_center.x + 4 + 4, screen_center.y + 4 + 4 }, Color3(0, 0, 0), false, 0.7f);
			Renderer::line(Vector2{ screen_center.x + 4, screen_center.y - 4 }, Vector2{ screen_center.x + 4 + 4, screen_center.y - 4 - 4 }, Color3(0, 0, 0), false, 0.7f);
			Renderer::line(Vector2{ screen_center.x - 4, screen_center.y - 4 }, Vector2{ screen_center.x - 4 - 4, screen_center.y - 4 - 4 }, Color3(0, 0, 0), false, 0.7f);
			Renderer::line(Vector2{ screen_center.x - 4, screen_center.y + 4 }, Vector2{ screen_center.x - 4 - 4, screen_center.y + 4 + 4 }, Color3(0, 0, 0), false, 0.7f);
			break;
		case 3:
			Renderer::circle(screen_center, Color3(0, 0, 0), 2.f, 1.f);
			Renderer::circle(screen_center, Color3(0, 0, 0), 4.f, 1.f);
			Renderer::circle(screen_center, { 219, 222, 60 }, 3.f, 1.f);
			break;
		case 4:

			if ((int)faken_rot > 89) { faken_rot = (float)0; }
			faken_rot++;

			if (gamerjuice > 30)
			{
				gamerjuice = 0;
				clr = Color3(RandomInteger(0, 255), RandomInteger(0, 255), RandomInteger(0, 255), 255);
			}
			else
				gamerjuice++;

			for (int i = 0; i < 4; i++)
			{
				std::vector <int> p;
				p.push_back(a * sin(GRD_TO_BOG(faken_rot + (i * 90))));									//p[0]		p0_A.x
				p.push_back(a * cos(GRD_TO_BOG(faken_rot + (i * 90))));									//p[1]		p0_A.y
				p.push_back((a / cos(gamma)) * sin(GRD_TO_BOG(faken_rot + (i * 90) + BOG_TO_GRD(gamma))));	//p[2]		p0_B.x
				p.push_back((a / cos(gamma)) * cos(GRD_TO_BOG(faken_rot + (i * 90) + BOG_TO_GRD(gamma))));	//p[3]		p0_B.y


				Renderer::line(screen_center, { screen_center.x + p[0], screen_center.y - p[1] }, clr, true);
				Renderer::line({ screen_center.x + p[0], screen_center.y - p[1] }, { screen_center.x + p[2], screen_center.y - p[3] }, clr, true);
			}
		default:
			break;
		}


		if (aidsware::ui::get_bool(xorstr_("debug")))
			ShowWindow(settings::console_window, SW_SHOW);
		else
			ShowWindow(settings::console_window, SW_HIDE);

		if (aidsware::ui::get_bool(xorstr_("raid esp")))
		{
			LogSystem::RenderExplosions();
		}

		/*
		if (aidsware::ui::get_bool(xorstr_("target player belt"))) {
			int w = 200, h = 102;

			belt::belt_tab_mov(Vector2(w, h + 20.0f));

			Renderer::rounded_rectangle_filled(belt::pos.x, belt::pos.y, w, 20, Color3(25, 25, 25), 5.f);
			Renderer::rounded_rectangle_filled(belt::pos.x, belt::pos.y + 20.0f, w, h, Color3(45, 83, 122), 5.f);
			Renderer::rounded_rectangle_filled(belt::pos.x + 5.0f, belt::pos.y + 20.0f + 5.0f, w - 10, h - 10, Color3(25, 25, 25), 5.f);
			//draw_image(float x, float y, float w, float h, const wchar_t* filename)
			//Renderer::draw_image(100, 100, 200, 200, wxorstr_(L"C:\\awlogo.png"));

		}
		*/
		if (aidsware::ui::get_bool(xorstr_("show users")))
		{
			float x = 1684, y = 391, w = 200, h = 300;
			Renderer::rounded_rectangle_filled(x, y, w, h, { 24,28,28 }, 10.f);
			Renderer::rounded_box(x, y, w, h, { 150, 150, 167 }, 10.f);

			int x_i = 1;
			x += 10.0f;
			int z_ = 0;
			if (slaves.size() < 1)
				z_ = -1;
			for (auto c : slaves)
			{
				POINT p;

				if (GetCursorPos(&p))
				{
					if (FindPoint(x, (y + (10 * (x_i++ + 1))), x + w, (y + (10 * (x_i++ + 2))) + 10, p.x, p.y))
					{
						Renderer::text({ x, (y + (10 * x_i++)) }, (alpha_index == z_ ? Color3(255, 120, 255) : Color3(210, 120, 210)), 14.f, false, true, wxorstr_(L"%s [%s] (%s)"),
							std::wstring(c.forum_name.begin(), c.forum_name.end()).c_str(),
							std::wstring(c.steam_name.begin(), c.steam_name.end()).c_str(),
							std::wstring(c.server_ip.begin(), c.server_ip.end()).c_str());
						if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0
							&& alpha_index == z_)
							alpha_index = -1;
						else if((GetKeyState(VK_LBUTTON) & 0x8000) != 0
							&& alpha_index != z_)
							alpha_index = z_;
					}
					else
					{
						Renderer::text({ x, (y + (10 * x_i++)) }, (alpha_index == z_ ? Color3(255, 120, 255) : Color3(219, 219, 219)), 14.f, false, true, wxorstr_(L"%s [%s] (%s)"),
							std::wstring(c.forum_name.begin(), c.forum_name.end()).c_str(),
							std::wstring(c.steam_name.begin(), c.steam_name.end()).c_str(),
							std::wstring(c.server_ip.begin(), c.server_ip.end()).c_str());
					}
				}
				z_ += 1;
			}
		}

		if (aidsware::ui::get_bool(xorstr_("flyhack indicator"))
			&& LocalPlayer::Entity())
		{
			//settings::flyhack = (settings::flyhack * 100.f) > threshold ? threshold : settings::flyhack;
			//settings::hor_flyhack = (settings::hor_flyhack * 100.f) > threshold ? threshold : settings::hor_flyhack;

			if (settings::flyhack >= 3.f)
			{
				Renderer::ProgressBar({ screen_center.x - 300, screen_center.y - 500 },
					{ screen_center.x + 300, screen_center.y - 500 },
					{ 51, 88, 181 }, { 38, 38, 60 },
					600.0f,
					600.0f,
					settings::flyhack);
			}
			else
			{
				Renderer::ProgressBar({ screen_center.x - 300, screen_center.y - 500 },
					{ screen_center.x + 300, screen_center.y - 500 },
					{ 51, 88, 181 }, { 38, 38, 60 },
					(600.0f * (3.0f / settings::flyhack)),
					600.0f,
					settings::flyhack);
			}

			if (settings::hor_flyhack >= 6.5f)
			{
				Renderer::ProgressBar({ screen_center.x - 300, screen_center.y - 470 },
					{ screen_center.x + 300, screen_center.y - 470 },
					{ 51, 88, 181 }, { 38, 38, 60 },
					600.0f,
					600.0f,
					settings::hor_flyhack);
			}
			else
			{
				Renderer::ProgressBar({ screen_center.x - 300, screen_center.y - 470 },
					{ screen_center.x + 300, screen_center.y - 470 },
					{ 51, 88, 181 }, { 38, 38, 60 },
					(600.f * (6.5f / settings::hor_flyhack)),
					600.f,
					settings::hor_flyhack);
			}
		}

		auto local = LocalPlayer::Entity();
		if (local == nullptr) {
			target_ply = nullptr;
			return;
		}

		float desyncTime = (Time::realtimeSinceStartup() - LocalPlayer::Entity()->lastSentTickTime()) - 0.03125 * 3;

		//LogSystem::RenderTraceResults();
		int y = 1;
		if (aidsware::ui::get_bool(xorstr_("crosshair indicators"))
			&& aidsware::ui::get_bool(xorstr_("insta kill")) || aidsware::ui::get_bool(xorstr_("peek assist")) || get_key(aidsware::ui::get_keybind(xorstr_("desync on key")))
			&& desyncTime > 0.0f)
		{
			y += 1;
			Renderer::ProgressBar({ screen_center.x - 30, screen_center.y + (10 * y) },
				{ screen_center.x + 30, screen_center.y + (10 * y) },
				{ 51, 88, 181 },
				{ 38, 38, 60 },
				60.0f * (1.0 / (desyncTime < 0.f ? 0.f : (desyncTime > 1.0f ? 1.0f : desyncTime))),
				60,
				desyncTime);
		}

		auto entz = LocalPlayer::Entity()->GetHeldEntity<BaseProjectile>();

		if (aidsware::ui::get_bool(xorstr_("crosshair indicators"))
			&& entz->get_NextAttackTime() < Time::fixedTime())
		{
			y += 1;
			float time = entz->get_NextAttackTime();
			//printf("%ff - %ff = %ff\n", time, Time::fixedTime(), time - Time::fixedTime());
			Renderer::ProgressBar({ screen_center.x - 30, screen_center.y + (10 * y) },
				{ screen_center.x + 30, screen_center.y + (10 * y) },
				{ 51, 88, 181 },
				{ 38, 38, 60 },
				60.0f * (1.0 / (time < 0.f ? 0.f : time)),
				60,
				time);
		}

		auto held = local->GetHeldEntity<BaseProjectile>();
		if (aidsware::ui::get_bool(xorstr_("reload indicator")) && !aidsware::ui::get_bool(xorstr_("always reload"))) {
			if (held) {
				if (held->HasReloadCooldown() && held->class_name_hash() != STATIC_CRC32("BowWeapon") && held->class_name_hash() != STATIC_CRC32("CompoundBowWeapon")) { // im sorry for my sins
					float time_left = held->nextReloadTime() - GLOBAL_TIME;
					float time_full = held->CalculateCooldownTime(held->nextReloadTime(), held->reloadTime()) - GLOBAL_TIME;
					y += 1;
					Renderer::rectangle_filled({ screen_center.x - 26, screen_center.y + (10 * y) }, { 51, 5 }, Color3(0, 0, 0));
					Renderer::rectangle_filled({ screen_center.x - 25, screen_center.y + (10 * y) + 1}, {50 * (time_left / time_full), 4}, Color3(0, 255, 0));
					Renderer::text({ (screen_center.x - 25) + (50 * (time_left / time_full)), screen_center.y + 31 + 2 }, Color3(219, 219, 219), 14.f, true, true, wxorstr_(L"%d"), (int)ceil(time_left));
				}	
				if (held->class_name_hash() == STATIC_CRC32("BaseProjectile") ||
					held->class_name_hash() == STATIC_CRC32("BowWeapon") ||
					held->class_name_hash() == STATIC_CRC32("CompoundBowWeapon") ||
					held->class_name_hash() == STATIC_CRC32("BaseLauncher") ||
					held->class_name_hash() == STATIC_CRC32("CrossbowWeapon")) {
					if (held->Empty()) {
						Renderer::text({ screen_center.x, screen_center.y + 48 }, Color3(89, 227, 255), 14.f, true, true, wxorstr_(L"[empty weapon]"));
					}
				}
			}
		}

		if (aidsware::ui::get_bool(xorstr_("always reload"))
			&& t_TimeSinceLastShot < held->reloadTime())
		{
			if (held) {
				if (held->class_name_hash() != STATIC_CRC32("BowWeapon")
					&& held->class_name_hash() != STATIC_CRC32("CompoundBowWeapon"))
				{ // im sorry for my sins
					float time_left = (t_TimeSinceLastShot);
					float time_full = (held->reloadTime() - (held->reloadTime() / 10));

					float result = time_full - time_left;

					if (result > (time_full - 0.15f))
						result = time_full;
					if (result < 0.1f) {
						result = 0.0f;
						time_left = time_full;
					}
					
					if (aidsware::ui::get_bool(xorstr_("reload indicator")))
					{
						Renderer::rectangle_filled({ screen_center.x - 51, screen_center.y + 38 }, { 101, 6 }, Color3(38, 38, 60));
						Renderer::rectangle_filled({ screen_center.x - 50, screen_center.y + 39 }, { 99 * (time_left / time_full), 4 }, Color3(51, 88, 181));
						Renderer::text({ (screen_center.x - 50) + (99 * (time_left / time_full)), screen_center.y + 41 + 2 }, Color3(219, 219, 219), 14.f, true, true, wxorstr_(L"%d"), (int)ceil(result));
					}
				}
			}
		}

		if (closest_node)
		{
			CBounds bounds = CBounds();

			Color3 box_col = aidsware::ui::get_color("targeted boxes");;
			float y = math::euler_angles(LocalPlayer::Entity()->bones()->eye_rot).y;
			Vector3 center = closest_node->transform()->position();
			center.y = center.y + 0.5;
			Vector3 extents = Vector3(1.5, 1.0f, 1.5f);
			Vector3 frontTopLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z), y);
			Vector3 frontTopRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z), y);
			Vector3 frontBottomLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z), y);
			Vector3 frontBottomRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z), y);
			Vector3 backTopLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z), y);
			Vector3 backTopRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z), y);
			Vector3 backBottomLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z), y);
			Vector3 backBottomRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z), y);

			Vector2 frontTopLeft_2d, frontTopRight_2d, frontBottomLeft_2d, frontBottomRight_2d, backTopLeft_2d, backTopRight_2d, backBottomLeft_2d, backBottomRight_2d;
			if (Camera::world_to_screen(frontTopLeft, frontTopLeft_2d) &&
				Camera::world_to_screen(frontTopRight, frontTopRight_2d) &&
				Camera::world_to_screen(frontBottomLeft, frontBottomLeft_2d) &&
				Camera::world_to_screen(frontBottomRight, frontBottomRight_2d) &&
				Camera::world_to_screen(backTopLeft, backTopLeft_2d) &&
				Camera::world_to_screen(backTopRight, backTopRight_2d) &&
				Camera::world_to_screen(backBottomLeft, backBottomLeft_2d) &&
				Camera::world_to_screen(backBottomRight, backBottomRight_2d)) {

				Renderer::line(frontTopLeft_2d, frontTopRight_2d, box_col, true, 1.5f);
				Renderer::line(frontTopRight_2d, frontBottomRight_2d, box_col, true, 1.5f);
				Renderer::line(frontBottomRight_2d, frontBottomLeft_2d, box_col, true, 1.5f);
				Renderer::line(frontBottomLeft_2d, frontTopLeft_2d, box_col, true, 1.5f);
				Renderer::line(backTopLeft_2d, backTopRight_2d, box_col, true, 1.5f);
				Renderer::line(backTopRight_2d, backBottomRight_2d, box_col, true, 1.5f);
				Renderer::line(backBottomRight_2d, backBottomLeft_2d, box_col, true, 1.5f);
				Renderer::line(backBottomLeft_2d, backTopLeft_2d, box_col, true, 1.5f);
				Renderer::line(frontTopLeft_2d, backTopLeft_2d, box_col, true, 1.5f);
				Renderer::line(frontTopRight_2d, backTopRight_2d, box_col, true, 1.5f);
				Renderer::line(frontBottomRight_2d, backBottomRight_2d, box_col, true, 1.5f);
				Renderer::line(frontBottomLeft_2d, backBottomLeft_2d, box_col, true, 1.5f);
			}
		}

		if (aidsware::ui::get_bool(xorstr_("players")))
		{
			auto entityList = BaseNetworkable::clientEntities()->entityList();
			if (!entityList) {
				target_ply = nullptr;
				return;
			}

			if (entityList->vals->size <= 1) {
				target_ply = nullptr;
				return;
			}

			viewMatrix = Camera::getViewMatrix();

			
			if (target_ply != nullptr) {
				if (!target_ply->IsValid() || target_ply->health() <= 0 || target_ply->is_teammate() || target_ply->HasPlayerFlag(PlayerFlags::Sleeping) || entities::dfc(target_ply) > aidsware::ui::get_float(xorstr_("target fov")) || (target_ply->playerModel()->isNpc() && !aidsware::ui::get_bool(xorstr_("npc")))) {
					target_ply = nullptr;
				}
				else {
					if (target_ply->isCached()) {
						auto bounds = target_ply->bones()->bounds;
						if (!bounds.empty())
						{
							Color3 col = get_color(target_ply, false, true);
							switch (aidsware::ui::get_combobox(xorstr_("target snapline"))) {
							case 1: //top
								Renderer::line({ bounds.left + ((bounds.right - bounds.left) / 2), bounds.bottom },
									{ screen_center.x, 0 },
									col,
									true);
								break;
							case 2: //center
								Renderer::line({ bounds.left + ((bounds.right - bounds.left) / 2), bounds.bottom },
									{ screen_center.x, screen_size.y / 2 },
									col,
									true);
								break;
							case 3: //bottom
								Renderer::line({ bounds.left + ((bounds.right - bounds.left) / 2), bounds.bottom }, 
									{ screen_center.x, screen_size.y }, 
									col,
									true);
								break;
							case 4: //penis
								Vector2 w;
								Camera::world_to_screen(LocalPlayer::Entity()->bones()->penis->position, w);
								Renderer::line({ bounds.left + ((bounds.right - bounds.left) / 2), bounds.bottom },
									w,
									col,
									true);
								break;
							}
						}
							
						auto mpv = target_ply->find_mpv_bone();
						Bone* target;
						if (mpv != nullptr)
							target = mpv;
						else
							target = target_ply->bones()->head;

						if (target->visible)
							Renderer::boldtext({ screen_center.x + 20, screen_center.y - 20 }, Color3(66, 135, 245), 14.f, true, true, wxorstr_(L"[s]"));
						//Renderer::boldtext({ screen_center.x - 20, screen_center.y + 20 }, Color3(255, 0, 0), 14.f, true, true, wxorstr_(L"[t]"));

						if (aidsware::ui::get_bool(xorstr_("target player belt")) && !aidsware::ui::is_menu_open()) {
							int w = 200, h = 102;

							belt::belt_tab_mov(Vector2(w, -20));

							Renderer::rounded_rectangle_filled( belt::pos.x, belt::pos.y - 20.0f, w, 20, Color3(25, 25, 25), 10.f);
							Renderer::rounded_rectangle_filled(belt::pos.x, belt::pos.y, w, h, Color3(45, 83, 122), 10.f);
							Renderer::rounded_rectangle_filled(belt::pos.x + 5.0f, belt::pos.y + 5.0f, w - 10, h - 10, Color3(25, 25, 25), 10.f);

							Renderer::text({ belt::pos.x + 7.0f, belt::pos.y - 16.0f }, Color3(219, 219, 219), 14.f, false, false, target_ply->_displayName());

							auto list = target_ply->inventory()->containerBelt()->itemList();
							if (list) {
								if (list->size) {
									int y = 0;
									for (int i = 0; i < list->size; i++) {
										auto item = (Item*)list->get(i);
										if (!item)
											continue;

										Color3 col = item->uid() == target_ply->clActiveItem() ? Color3(114, 31, 204) : Color3(219, 219, 219);

										Renderer::text({ belt::pos.x + 7.0f, belt::pos.y + 7.0f + y }, col, 14.f, false, false, wxorstr_(L"%s [x%d]"), item->info()->displayName()->english(), item->amount());

										y += 15;
									}
								}
							}


							/*
							Color defaultcol = DDraw::get_color();
							Color defaultbgcol(0.96862745f, 0.92156863f, 0.88235295f, 0.15f);
							Color selectedcol(0.12156863f, 0.41960785f, 0.627451f, 0.75f);

							float info_y = 0;

							for (size_t i = 0; i < list->size; i++)
							{
								auto item = (Item*)list->get(i);
								if (!item) continue;
								DDraw::set_color(defaultbgcol);

								if (item->uid() == target_ply->clActiveItem())
									DDraw::set_color(selectedcol);
								Rect r(660.f + info_y, 860.f, 90.f, 92.f);
								auto t = Texture2D::white();

								DDraw::DrawTexture(r, t);

								if (item)
								{
									auto sprite = item->iconSprite();
									auto tex = sprite->texture();
									auto rect = sprite->textureRect();

									Rect w; 
									w.x = 660.0f + info_y;
									w.y = 860.f; 
									w.wid = rect.wid / 3.f;
									w.hei = rect.hei / 3.f;

									DDraw::set_color(defaultcol);

									DDraw::DrawTexture(w, tex);
								}
								info_y += 96;
							}
							*/
						}
					}
				}
			}

			if (target_ply == nullptr)
				settings::tr::manipulate_visible = false;

			if (aidsware::ui::get_bool(xorstr_("crosshair indicators")))
			{
				if (settings::tr::manipulate_visible)
					Renderer::boldtext({ screen_center.x + 20, screen_center.y + 20 }, Color3(52, 235, 97), 14.f, true, true, wxorstr_(L"[v]"));
				if (settings::tr::manipulated)
					Renderer::boldtext({ screen_center.x + 20, screen_center.y - 20 }, Color3(90, 19, 128), 14.f, true, true, wxorstr_(L"[p]"));
				if (settings::can_insta)
					Renderer::boldtext({ screen_center.x - 20, screen_center.y + 20 }, Color3(230, 180, 44), 14.f, true, true, wxorstr_(L"[i]"));
			}
			std::vector<BasePlayer*> temp_target_list{};

			if (aidsware::ui::get_bool(xorstr_("logs")))
				LogSystem::Render();

			if (aidsware::ui::get_bool(xorstr_("show prediction")))
				LogSystem::RenderTraceResults();

			int helis = 0;

			bool zflag = aidsware::ui::get_bool(xorstr_("players")) || aidsware::ui::get_bool(xorstr_("sleepers"));
			
			int entlistsz = entityList->vals->size;

			BaseEntity* best = nullptr;

			for (int i = 0; i < entlistsz; i++) {

				
				auto entity = *reinterpret_cast<BaseEntity**>(std::uint64_t(entityList->vals->buffer) + (0x20 + (sizeof(void*) * i)));
				if (!entity)
				{
					//SleepEx(1, 0);
					continue;
				}
				if (!entity->IsValid()) {
					continue;
				}
				/*
				if (aidsware::ui::get_bool(xorstr_("debug"))) {
					if (entity->transform()->position().distance(LocalPlayer::Entity()->transform()->position()) <= 25.f) {
						Vector2 screen;
						if (Camera::world_to_screen(entity->transform()->position(), screen)) {
							Renderer::text(screen, Color3(0, 255, 0), 14.f, true, true, wxorstr_(L"%s"), StringConverter::ToUnicode(entity->class_name()).c_str());
							Renderer::text(screen + Vector2(0, 15), Color3(0, 255, 0), 14.f, true, true, wxorstr_(L"%s"), entity->ShortPrefabName());
							Renderer::text(screen + Vector2(0, 30), Color3(0, 255, 0), 14.f, true, true, wxorstr_(L"%s"), entity->gameObject()->name());
						}
					}
					//printf("%s - - - - - - %s\n", StringConverter::ToUnicode(entity->class_name()).c_str(), entity->ShortPrefabName());
				}
				*/
				auto pl1 = reinterpret_cast<BasePlayer*>(entity);
				if (settings::alpha::shoot_same_target)
					if (pl1->userID() == target_id)
						target_ply = pl1;
				if (master_id > 1000000)
					if (pl1->userID() == master_id)
						master = pl1;


				Vector2 screen;


				float d = entity->transform()->position().distance(LocalPlayer::Entity()->transform()->position());
				auto prefab = entity->ShortPrefabName_hash();

				if (Camera::world_to_screen(entity->transform()->position(), screen))
				{
					switch (prefab)
					{
					case STATIC_CRC32("minicopter.entity"):
						if (aidsware::ui::get_bool(xorstr_("vehicles")))
						{
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Mini"));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
						}
					case STATIC_CRC32("supply_drop"):
						if (aidsware::ui::get_bool(xorstr_("supply")))
						{
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"supply drop"));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
						}
					case STATIC_CRC32("present"):
						if (aidsware::ui::get_bool(xorstr_("supply")))
						{
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"supply drop"));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
						}
					case STATIC_CRC32("presentdrop"):
						if (aidsware::ui::get_bool(xorstr_("supply")))
						{
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"supply drop"));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
						}
					case STATIC_CRC32("present_drop"):
						if (aidsware::ui::get_bool(xorstr_("supply")))
						{
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"supply drop"));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("supply color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
						}
					}

					switch (entity->class_name_hash())
					{
					case STATIC_CRC32("BaseHelicopter"):
						if (aidsware::ui::get_bool(xorstr_("patrol-heli")))
						{
							if (aidsware::ui::get_bool("distance"))
								Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
							Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Patrol-heli"));

							if (aidsware::ui::get_bool("target heli"))
							{
								if (dfc(entity, screen) < aidsware::ui::get_float(xorstr_("target fov"))) {
									if (target_heli == nullptr)
										target_heli = entity;
									else
										if (dfc(target_heli, screen) > dfc(entity, screen))
											target_heli = entity;
									Renderer::line(screen, { screen_center.x, screen_size.y }, aidsware::ui::get_color(xorstr_("vehicles color")), true);
								}
								else
									target_heli = nullptr;
							}
							else target_heli = nullptr;
						}
					}
				}

				
				if (aidsware::ui::get_bool(xorstr_("auto farm")))
				{
					switch (prefab)
					{
					case STATIC_CRC32("stone-ore"):
						if (entity->transform()->position().distance(LocalPlayer::Entity()->transform()->position()) < closest_node->transform()->position().distance(LocalPlayer::Entity()->transform()->position()))
							nodelist.push_back(entity);
					case STATIC_CRC32("sulfur-ore"):
						if (entity->transform()->position().distance(LocalPlayer::Entity()->transform()->position()) < closest_node->transform()->position().distance(LocalPlayer::Entity()->transform()->position()))
							nodelist.push_back(entity);
					case STATIC_CRC32("metal-ore"):
						if (entity->transform()->position().distance(LocalPlayer::Entity()->transform()->position()) < closest_node->transform()->position().distance(LocalPlayer::Entity()->transform()->position()))
							nodelist.push_back(entity);
					}
				}
				 
				if (d < aidsware::ui::get_float(xorstr_("esp dist")))
				{
					if (Camera::world_to_screen(entity->transform()->position(), screen))
					{
						switch (prefab)
						{
						case STATIC_CRC32("stone-ore"):
							if (aidsware::ui::get_bool(xorstr_("stone ore")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"Stone Ore"));
								continue;
							}
							break;
						case STATIC_CRC32("sulfur-ore"):
							if (aidsware::ui::get_bool(xorstr_("sulfur ore")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"Sulfur Ore"));
								continue;
							}
							break;
						case STATIC_CRC32("metal-ore"):
							if (aidsware::ui::get_bool(xorstr_("metal ore")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("ores color")), 14.f, true, true, wxorstr_(L"Metal Ore"));
								continue;
							}
							break;
						case STATIC_CRC32("small_stash_deployed"):
							if (aidsware::ui::get_bool(xorstr_("stashes")))
							{
								if (entity->HasFlag(BaseEntity::Flags::Reserved5)) {
									if (aidsware::ui::get_bool("distance"))
										Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("stashes color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
									Renderer::text(screen, aidsware::ui::get_color(xorstr_("stashes color")), 14.f, true, true, wxorstr_(L"Stash (Hidden)"));
									continue;
								}
								else {
									if (aidsware::ui::get_bool("distance"))
										Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("stashes color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
									Renderer::text(screen, aidsware::ui::get_color(xorstr_("stashes color")), 14.f, true, true, wxorstr_(L"Stash"));
									continue;
								}
							}
							break;
						case STATIC_CRC32("cupboard.tool.deployed"):
							if (aidsware::ui::get_bool(xorstr_("tool cupboards")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("tc color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("tc color")), 14.f, true, true, wxorstr_(L"tool cupboard"));
								continue;
							}
							break;
						case STATIC_CRC32("box.wooden.large"):
							if (aidsware::ui::get_bool(xorstr_("storage")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"large wood box"));
								continue;
							}
							break;
						case STATIC_CRC32("woodbox_deployed"):
							if (aidsware::ui::get_bool(xorstr_("storage")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"small wood box"));
								continue;
							}
							break;
						case STATIC_CRC32("coffinstorage"):
							if (aidsware::ui::get_bool(xorstr_("storage")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"coffin"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_normal"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"military box"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_mine"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"mine box"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_elite"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"elite crate"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_normal_2_food"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"food crate"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_tools"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"toolbox"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_normal_2_medical"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"medical box"));
								continue;
							}
							break;
						case STATIC_CRC32("crate_normal_2"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"lootbox"));
								continue;
							}
							break;
						case STATIC_CRC32("codelockedhackablecrate"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"locked crate"));
								continue;	
							}
							break;
						case STATIC_CRC32("codelockedhackablecrate_oilrig"):
							if (aidsware::ui::get_bool(xorstr_("crates")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("crate color")), 14.f, true, true, wxorstr_(L"locked crate"));
								if (aidsware::ui::get_bool("magic"))
									entity->ServerRPC("RPC_Hack");
								continue;
							}
							break;
						case STATIC_CRC32("workbench1.deployed"):
							if (aidsware::ui::get_bool(xorstr_("workbench")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"workbench 1"));
								continue;
							}
							break;
						case STATIC_CRC32("workbench2.deployed"):
							if (aidsware::ui::get_bool(xorstr_("workbench")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"workbench 2"));
								continue;
							}
							break;
						case STATIC_CRC32("workbench3.deployed"):
							if (aidsware::ui::get_bool(xorstr_("workbench")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("workbench color")), 14.f, true, true, wxorstr_(L"workbench 3"));
								continue;
							}
							break;
						case STATIC_CRC32("generic_world"):
						{
							if (!aidsware::ui::get_bool(xorstr_("weapons")))
								break;
							const wchar_t* weapon_name = entity->gameObject()->name();
							if (wcsstr(weapon_name, wxorstr_(L"lmg.m249")) != nullptr) {
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"m249"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.ak")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"assault rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.l96")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"l96 rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.lr300")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"lr300 rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.bolt")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"bolt action rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.semiauto")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"semi-auto rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"rifle.m39")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"m39 rifle"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"smg.mp5")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"mp5a4"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"smg.2")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"custom-smg"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"smg.thompson")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"thompson"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.eoka")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"eoka"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.m92")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"m92 pistol"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.nailgun")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"nailgun"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.python")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"python"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.revolver")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"revolver"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"pistol.semiauto")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"semi-auto pistol"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"shotgun.double")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"double-barrel shotgun"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"shotgun.pump")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"pump shotgun"));
								continue;
							}
							else if (wcsstr(weapon_name, wxorstr_(L"shotgun.spas12")) != nullptr)
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("weapon color")), 14.f, true, true, wxorstr_(L"spas-12"));
								continue;
							}
							break;
						}
						case STATIC_CRC32("LootContainer"):
						{
							const wchar_t* name = entity->gameObject()->name();
							if (aidsware::ui::get_bool(xorstr_("present"))
								&& std::wstring(name).find(wxorstr_(L"giftbox")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"present"));
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("box color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
							}
							break;
						}
						case STATIC_CRC32("rhib"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"RHIB"));
								continue;
							}
							break;
						case STATIC_CRC32("rowboat"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Boat"));
								continue;
							}
							break;
						case STATIC_CRC32("hotairballoon"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Hot Air Balloon"));
								continue;
							}
							break;
						case STATIC_CRC32("scraptransporthelicopter"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Scrap Heli"));
								continue;
							}
							break;
						case STATIC_CRC32("submarineduo.entity"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Submarine (Duo)"));
								continue;
							}
							break;
						case STATIC_CRC32("submarinesolo.entity"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Submarine (Solo)"));
								continue;
							}
							break;
						case STATIC_CRC32("testridablehorse"):
							if (aidsware::ui::get_bool(xorstr_("vehicles")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"Horse"));
								continue;
							}
							break;
						case STATIC_CRC32("hemp-collectable"):
							if (aidsware::ui::get_bool(xorstr_("hemp")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("vehicles color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("hemp color")), 14.f, true, true, wxorstr_(L"Hemp"));
								continue;
							}
							break;
						}

						switch (entity->class_name_hash())
						{
						case STATIC_CRC32("PlayerCorpse"):
							if (aidsware::ui::get_bool(xorstr_("corpses")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"Corpse"));
								continue;
							}
							break;
						case STATIC_CRC32("NPCPlayerCorpse"):
							if (aidsware::ui::get_bool(xorstr_("corpses")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"Corpse"));
								continue;
							}
							break;
						case STATIC_CRC32("item_drop_backpack"):
							if (aidsware::ui::get_bool(xorstr_("corpses")))
							{
								if (aidsware::ui::get_bool("distance"))
									Renderer::text({ screen.x, screen.y + 10 }, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"%dm"), (int)d);
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("corpses color")), 14.f, true, true, wxorstr_(L"Backpack"));
								continue;
							}
							break;
						case STATIC_CRC32("AutoTurret"):
							if (aidsware::ui::get_bool(xorstr_("traps")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("traps color")), 14.f, true, true, wxorstr_(L"Auto Turret"));
								continue;
							}
							break;
						case STATIC_CRC32("Landmine"):
							if (aidsware::ui::get_bool(xorstr_("traps")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("traps color")), 14.f, true, true, wxorstr_(L"Landmine"));
								continue;
							}
							break;
						case STATIC_CRC32("BearTrap"):
							if (aidsware::ui::get_bool(xorstr_("traps")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("traps color")), 14.f, true, true, wxorstr_(L"Bear Trap"));
								continue;
							}
							break;
						case STATIC_CRC32("SamSite"):
							if (aidsware::ui::get_bool(xorstr_("traps")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("traps color")), 14.f, true, true, wxorstr_(L"SAM Site"));
								continue;
							}
							break;
						case STATIC_CRC32("GunTrap"):
							if (aidsware::ui::get_bool(xorstr_("traps")))
							{
								Renderer::text(screen, aidsware::ui::get_color(xorstr_("traps color")), 14.f, true, true, wxorstr_(L"Shotgun Trap"));
								continue;
							}
							break;
						}
					}
				}
				

				if (zflag)
				{
					//continue; these if statements fk it up
					auto c = entity->class_name_hash();
					if (c == STATIC_CRC32("BasePlayer") ||
						c == STATIC_CRC32("NPCPlayerApex") ||
						c == STATIC_CRC32("NPCMurderer") ||
						c == STATIC_CRC32("NPCPlayer") ||
						c == STATIC_CRC32("HumanNPC") ||
						c == STATIC_CRC32("ScientistNPC") ||
						c == STATIC_CRC32("HTNPlayer") ||
						c == STATIC_CRC32("HumanNPCNew") ||
						c == STATIC_CRC32("ScientistNPCNew") ||
						c == STATIC_CRC32("TunnelDweller") ||
						c == STATIC_CRC32("BanditGuard"))
					{
						auto player = reinterpret_cast<BasePlayer*>(entity);

						if (!player->isCached()) continue;
						if (player->health() <= 0.1f || player->lifestate() != BaseCombatEntity::Lifestate::Alive) continue;
						if (player->HasPlayerFlag(PlayerFlags::Sleeping) && !aidsware::ui::get_bool(xorstr_("sleepers"))) continue;
						if (player->playerModel()->isNpc() && !aidsware::ui::get_bool(xorstr_("npc"))) continue;
						if (player->userID() == LocalPlayer::Entity()->userID()) continue;



						if (aidsware::ui::get_bool(xorstr_("chams"))
							&& Time::fixedTime() > lastChamsUpdate + 0.05f)
						{
							do_chams(player);
							lastChamsUpdate = Time::fixedTime();
						}

						auto bounds = player->bones()->bounds;
						if (true) {
							int y_ = 0;

							float box_width = bounds.right - bounds.left;
							float box_height = bounds.bottom - bounds.top;
							Vector2 footPos = { bounds.left + (box_width / 2), bounds.bottom + 7.47f };
							Vector2 headPos = { bounds.left + (box_width / 2), bounds.top - 9.54f };

							bool flag1 = false;

							for (int i = 0; i < current_visible_players.size(); i++)
							{
								auto a = current_visible_players[i];
								if (a->userID() == player->userID())
								{
									Color3 col = get_color(player, false, true);
									Renderer::line({ bounds.left + ((bounds.right - bounds.left) / 2), bounds.bottom }, { screen_center.x, screen_size.y }, col, true);
									if (aidsware::ui::get_bool(xorstr_("debug")))
										Renderer::text({ headPos.x, headPos.y - 20 }, col, 14.f, true, true, wxorstr_(L"i: %i"), i);
									flag1 = true;
								}
							}

				
							Color3 col = get_color(player, false, flag1);
							Color3 box_col = get_color(player, true, flag1);

							float dist = player->bones()->head->position.distance(local->bones()->head->position);
							if (aidsware::ui::get_bool(xorstr_("skeleton"))) {
								if (dist < 175.f) {

									auto info = player->bones();

									auto head_b = info->head;
									auto spine4_b = info->spine4;
									auto l_upperarm_b = info->l_upperarm;
									auto l_forearm_b = info->l_forearm;
									auto l_hand_b = info->l_hand;
									auto r_upperarm_b = info->r_upperarm;
									auto r_forearm_b = info->r_forearm;
									auto r_hand_b = info->r_hand;
									auto pelvis_b = info->pelvis;
									auto l_hip_b = info->l_hip;
									auto l_knee_b = info->l_knee;
									auto l_foot_b = info->l_foot;
									auto r_hip_b = info->r_hip;
									auto r_knee_b = info->r_knee;
									auto r_foot_b = info->r_foot;
									auto r_toe_b = info->r_toe;
									auto l_toe_b = info->l_toe;

									Vector2 head, spine, l_upperarm, l_forearm, l_hand, r_upperarm, r_forearm, r_hand, pelvis, l_hip, l_knee, l_foot, r_hip, r_knee, r_foot, l_toe, r_toe;
									if (Camera::world_to_screen(head_b->position, head) &&
										Camera::world_to_screen(spine4_b->position, spine) &&
										Camera::world_to_screen(l_upperarm_b->position, l_upperarm) &&
										Camera::world_to_screen(l_forearm_b->position, l_forearm) &&
										Camera::world_to_screen(l_hand_b->position, l_hand) &&
										Camera::world_to_screen(r_upperarm_b->position, r_upperarm) &&
										Camera::world_to_screen(r_forearm_b->position, r_forearm) &&
										Camera::world_to_screen(r_hand_b->position, r_hand) &&
										Camera::world_to_screen(pelvis_b->position, pelvis) &&
										Camera::world_to_screen(l_hip_b->position, l_hip) &&
										Camera::world_to_screen(l_knee_b->position, l_knee) &&
										Camera::world_to_screen(l_foot_b->position, l_foot) &&
										Camera::world_to_screen(r_hip_b->position, r_hip) &&
										Camera::world_to_screen(r_knee_b->position, r_knee) &&
										Camera::world_to_screen(r_toe_b->position, r_toe) &&
										Camera::world_to_screen(l_toe_b->position, l_toe) &&
										Camera::world_to_screen(r_foot_b->position, r_foot)) {

										Color3 viscol = player->playerModel()->isNpc() ? Color3(38, 255, 0) : Color3(0, 250, 255);
										Color3 inviscol = player->playerModel()->isNpc() ? Color3(22, 145, 0) : Color3(0, 152, 156);

										if (player->HasPlayerFlag(PlayerFlags::Sleeping)) {
											viscol = aidsware::ui::get_color("visible skeleton");
											inviscol = aidsware::ui::get_color("invisible skeleton");
										}
										else if (!player->playerModel()->isNpc()) {
											viscol = aidsware::ui::get_color("visible skeleton players");
											inviscol = aidsware::ui::get_color("invisible skeleton players");
										}
										else {
											viscol = aidsware::ui::get_color("visible skeleton npc");
											inviscol = aidsware::ui::get_color("invisible skeleton npc");
										}

										Renderer::line(head, spine, (head_b->visible || spine4_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(spine, l_upperarm, (spine4_b->visible || l_upperarm_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(l_upperarm, l_forearm, (l_upperarm_b->visible || l_forearm_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(l_forearm, l_hand, (l_forearm_b->visible || l_hand_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(spine, r_upperarm, (spine4_b->visible || r_upperarm_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(r_upperarm, r_forearm, (r_upperarm_b->visible || r_forearm_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(r_forearm, r_hand, (r_forearm_b->visible || r_hand_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(spine, pelvis, (spine4_b->visible || pelvis_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(pelvis, l_hip, (pelvis_b->visible || l_hip_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(l_hip, l_knee, (l_hip_b->visible || l_knee_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(l_knee, l_foot, (l_knee_b->visible || l_foot_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(pelvis, r_hip, (pelvis_b->visible || r_hip_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(r_hip, r_knee, (r_hip_b->visible || r_knee_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(r_knee, r_foot, (r_knee_b->visible || r_foot_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(r_foot, r_toe, (r_foot_b->visible || r_toe_b->visible) ? viscol : inviscol, 3.f);
										Renderer::line(l_foot, l_toe, (l_foot_b->visible || l_toe_b->visible) ? viscol : inviscol, 3.f);
									}
								}
							}

							Vector2 namePos(headPos.x, headPos.y - 20);
							Vector2 distPos(headPos.x, headPos.y - 10);

							Renderer::text(headPos, col, 14.f, true, true, wxorstr_(L"%s"), player->_displayName(), (int)ceil(dist), (int)ceil(player->health()));
							if (aidsware::ui::get_bool("distance"))
								Renderer::text(distPos, col, 14.f, true, true, wxorstr_(L"%dm"), (int)ceil(dist));
							//Renderer::text(headPos, col, 14.f, true, true, wxorstr_(L"%dhp"), (int)ceil(player->health()));

							if (aidsware::ui::get_bool(xorstr_("looking direction")) && !player->HasPlayerFlag(PlayerFlags::Sleeping))
								Renderer::line(player->bones()->dfc, player->bones()->forward, aidsware::ui::get_color(xorstr_("looking direction color")), true);

							


							if (aidsware::ui::get_bool(xorstr_("hpbar")))
							{
								int health = (int)player->health();
								float max = 100.f;
								if (player->playerModel()->isNpc())
									max = player->maxHealth();
								float max_dist = 201.f;
								switch (aidsware::ui::get_combobox(xorstr_("hpbar type"))) {
								case 1:
									if (LocalPlayer::Entity()->transform()->position().distance(player->transform()->position()) > max_dist)
										break;
									if (health <= 33)
									{
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width), 3.f), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width * (health / max)), 3.f), Color3(186, 48, 13, 255));
									}
									if (health >= 34 && health <= 66)
									{
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width), 3.f), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width * (health / max)), 3.f), Color3(196, 196, 22, 255));
									}
									if (health >= 67)
									{
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width), 3.f), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left, bounds.bottom), Vector2((box_width * (health / max)), 3.f), Color3(22, 196, 39, 255));
									}
									break;
								case 0:
									if (LocalPlayer::Entity()->transform()->position().distance(player->transform()->position()) > max_dist)
										break;
									if (health <= 33)
									{
										Renderer::rectangle_filled(Vector2(bounds.left - 6, bounds.bottom), Vector2(4.f, -box_height), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left - 5, bounds.bottom + 1), Vector2(2.f, -((box_height - 1) * (health / max))), Color3(186, 48, 13, 255));
									}
									if (health >= 34 && health <= 66)
									{
										Renderer::rectangle_filled(Vector2(bounds.left - 6, bounds.bottom), Vector2(4.f, -box_height), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left - 5, bounds.bottom + 1), Vector2(2.f, -((box_height - 1) * (health / max))), Color3(196, 196, 22, 255));
									}
									if (health >= 67)
									{
										Renderer::rectangle_filled(Vector2(bounds.left - 6, bounds.bottom), Vector2(4.f, -box_height), Color3(0, 0, 0, 255));
										Renderer::rectangle_filled(Vector2(bounds.left - 5, bounds.bottom + 1), Vector2(2.f, -((box_height - 1) * (health / max))), Color3(22, 196, 39, 255));
									}
									break;
								}
							}

							if (aidsware::ui::get_bool(xorstr_("custom box")))
							{
								std::string ap = aidsware::ui::get_text(xorstr_("custom box path"));
								std::string _path = ap; _path = settings::data_dir + "\\" + _path;
								if (!ap.empty())
								{
									if (exists(_path))
									{
										if (_path != settings::custom_box_path)
										{
											settings::custom_box_path = _path;
											std::wstring w(settings::custom_box_path.begin(), settings::custom_box_path.end());
											Renderer::set_custom_box(w);
											//printf("set box path\n");
											//set custom box bitmap in renderer
											//call draw image instead of drawing box
										}
										else
											Renderer::custom_box(bounds.left, bounds.top, box_width, box_height);
									}
								}
							}

							if (aidsware::ui::get_bool(xorstr_("box")) && !aidsware::ui::get_bool(xorstr_("custom box")))
							{
								switch (aidsware::ui::get_combobox(xorstr_("box type"))) {
								case 0: // cornered
									Renderer::line({ bounds.left, bounds.top }, { bounds.left + (box_width / 3.5f), bounds.top }, box_col, true, 1.5f);
									Renderer::line({ bounds.right, bounds.top }, { bounds.right - (box_width / 3.5f), bounds.top }, box_col, true, 1.5f);

									Renderer::line({ bounds.left, bounds.bottom }, { bounds.left + (box_width / 3.5f), bounds.bottom }, box_col, true, 1.5f);
									Renderer::line({ bounds.right, bounds.bottom }, { bounds.right - (box_width / 3.5f), bounds.bottom }, box_col, true, 1.5f);

									Renderer::line({ bounds.left, bounds.top }, { bounds.left, bounds.top + (box_width / 3.5f) }, box_col, true, 1.5f);
									Renderer::line({ bounds.right, bounds.top }, { bounds.right, bounds.top + (box_width / 3.5f) }, box_col, true, 1.5f);

									Renderer::line({ bounds.left, bounds.bottom }, { bounds.left, bounds.bottom - (box_width / 3.5f) }, box_col, true, 1.5f);
									Renderer::line({ bounds.right, bounds.bottom }, { bounds.right, bounds.bottom - (box_width / 3.5f) }, box_col, true, 1.5f);
									break;
								case 1: // 2d
									Renderer::line({ bounds.left, bounds.top }, { bounds.right, bounds.top }, box_col, true, 1.5f);
									Renderer::line({ bounds.left, bounds.bottom }, { bounds.right, bounds.bottom }, box_col, true, 1.5f);

									Renderer::line({ bounds.left, bounds.top }, { bounds.left, bounds.bottom }, box_col, true, 1.5f);
									Renderer::line({ bounds.right, bounds.top }, { bounds.right, bounds.bottom }, box_col, true, 1.5f);
									break;
								case 2:
									Renderer::rounded_box(bounds.left, bounds.top, box_width, box_height, box_col, 10.f);
									break;
								case 3: // 3d (hippity hoppity your 3d box is now my property)
									CBounds bounds = CBounds();

									if (player->IsDucked()) {
										bounds.center = player->midPoint() + Vector3(0.0f, 0.55f, 0.0f);
										bounds.extents = Vector3(0.4f, 0.65f, 0.4f);
									}
									else {
										if (player->HasPlayerFlag(PlayerFlags::Wounded) || player->HasPlayerFlag(PlayerFlags::Sleeping)) {
											bounds.center = player->bones()->pelvis->position;
											bounds.extents = Vector3(0.9f, 0.2f, 0.4f);
										}
										else {
											bounds.center = player->midPoint() + Vector3(0.0f, 0.85f, 0.0f);
											bounds.extents = Vector3(0.4f, 0.9f, 0.4f);
										}
									}


									float y = math::euler_angles(player->bones()->eye_rot).y;
									Vector3 center = bounds.center;
									Vector3 extents = bounds.extents;
									Vector3 frontTopLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y + extents.y, center.z - extents.z), y);
									Vector3 frontTopRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y + extents.y, center.z - extents.z), y);
									Vector3 frontBottomLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y - extents.y, center.z - extents.z), y);
									Vector3 frontBottomRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y - extents.y, center.z - extents.z), y);
									Vector3 backTopLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y + extents.y, center.z + extents.z), y);
									Vector3 backTopRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y + extents.y, center.z + extents.z), y);
									Vector3 backBottomLeft = math::rotate_point(center, Vector3(center.x - extents.x, center.y - extents.y, center.z + extents.z), y);
									Vector3 backBottomRight = math::rotate_point(center, Vector3(center.x + extents.x, center.y - extents.y, center.z + extents.z), y);

									Vector2 frontTopLeft_2d, frontTopRight_2d, frontBottomLeft_2d, frontBottomRight_2d, backTopLeft_2d, backTopRight_2d, backBottomLeft_2d, backBottomRight_2d;
									if (Camera::world_to_screen(frontTopLeft, frontTopLeft_2d) &&
										Camera::world_to_screen(frontTopRight, frontTopRight_2d) &&
										Camera::world_to_screen(frontBottomLeft, frontBottomLeft_2d) &&
										Camera::world_to_screen(frontBottomRight, frontBottomRight_2d) &&
										Camera::world_to_screen(backTopLeft, backTopLeft_2d) &&
										Camera::world_to_screen(backTopRight, backTopRight_2d) &&
										Camera::world_to_screen(backBottomLeft, backBottomLeft_2d) &&
										Camera::world_to_screen(backBottomRight, backBottomRight_2d)) {

										Renderer::line(frontTopLeft_2d, frontTopRight_2d, box_col, true, 1.5f);
										Renderer::line(frontTopRight_2d, frontBottomRight_2d, box_col, true, 1.5f);
										Renderer::line(frontBottomRight_2d, frontBottomLeft_2d, box_col, true, 1.5f);
										Renderer::line(frontBottomLeft_2d, frontTopLeft_2d, box_col, true, 1.5f);
										Renderer::line(backTopLeft_2d, backTopRight_2d, box_col, true, 1.5f);
										Renderer::line(backTopRight_2d, backBottomRight_2d, box_col, true, 1.5f);
										Renderer::line(backBottomRight_2d, backBottomLeft_2d, box_col, true, 1.5f);
										Renderer::line(backBottomLeft_2d, backTopLeft_2d, box_col, true, 1.5f);
										Renderer::line(frontTopLeft_2d, backTopLeft_2d, box_col, true, 1.5f);
										Renderer::line(frontTopRight_2d, backTopRight_2d, box_col, true, 1.5f);
										Renderer::line(frontBottomRight_2d, backBottomRight_2d, box_col, true, 1.5f);
										Renderer::line(frontBottomLeft_2d, backBottomLeft_2d, box_col, true, 1.5f);
									}
									break;
								}
							}

							if (player->GetHeldItem() && !player->HasPlayerFlag(PlayerFlags::Sleeping)) {
								Renderer::text(footPos, col, 14.f, true, true, player->GetHeldItem()->info()->displayName()->english());
								y_ += 16;
							}

							if (player->HasPlayerFlag(PlayerFlags::Wounded)) {
								Renderer::boldtext(footPos + Vector2(0, y_), Color3(255, 0, 0, 255), 14.f, true, true, wxorstr_(L"*wounded*"));
								y_ += 16;
							}

							//printf("ID COMPARE > %u : %u\n", player->userID(), target_id);
							if (player->is_visible() && settings::alpha::shoot_same_target && player->userID() == target_id)
							{
								temp_target_list.push_back(player);
								continue;
							}

							if (entities::dfc(player) < aidsware::ui::get_float(xorstr_("target fov")))
								temp_target_list.push_back(player);
							continue;
						}
					}
				}
				else {
					if (target_ply != nullptr)
						target_ply = nullptr;
				}
			}
			if (helis < 1)
				target_heli = nullptr;

			if (settings::instakill)
			{
				current_visible_players.clear();
				std::vector<BasePlayer*> best = {};

				BasePlayer* p_tmp = nullptr;
				int max = (int)aidsware::ui::get_float(xorstr_("counter"));
				for (int i = 0; i < max; i++)
				{
					if (temp_target_list.size() <= max)
					{
						current_visible_players = temp_target_list;
						return;
					}
					for (auto player : temp_target_list)
					{
						if (!player->is_teammate() && !player->HasPlayerFlag(PlayerFlags::Sleeping))
							if (p_tmp == nullptr)
								p_tmp = player;
							else
							{
								bool f = false;
								if (best.size() == 0) {
									if (entities::dfc(p_tmp) > entities::dfc(player))
										p_tmp = player;
									f = true;
								}
								else
									for (auto b : best)
										if (b->userID() == player->userID())
											f = true;

								if (!f)
									if (entities::dfc(p_tmp) > entities::dfc(player))
										p_tmp = player;

							}
					}
					best.push_back(p_tmp);
				}
				current_visible_players = best;
			}
			else {
				for (auto player : temp_target_list)
				{
					//shoot same target as master
					if (settings::alpha::shoot_same_target)
					{
						if (player->userID() == target_id
							&& player->is_visible())
						{
							target_ply = player;
							break;
						}
					}
					if (!player->is_teammate() && !player->HasPlayerFlag(PlayerFlags::Sleeping)
						|| player->userID() != friend_id || player->userID() != master_id) {
						if (dfc(player) < aidsware::ui::get_float(xorstr_("target fov"))) {
							if (target_ply == nullptr)
								target_ply = player;
							else
								if (dfc(target_ply) > dfc(player))
									target_ply = player;
						}
					}
				}
			}
		}
	}
}