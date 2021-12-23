namespace settings {
	std::string data_dir = xorstr_("");
	std::string custom_box_path = xorstr_("");
	std::string current_hitsound = xorstr_("");

	bool cheat_init = false;

	bool open = false;

	bool instakill = false;
	bool can_insta = false;
	bool peek_insta = false;
	
	bool panic = false;
	bool suicide = true;

	HWND console_window = 0;

	float max_flyhack = 0.f;
	float max_hor_flyhack = 0.f;
	float flyhack = 0.f;
	float hor_flyhack = 0.f;

	uintptr_t il_init_methods;
	uintptr_t serverrpc_projectileshoot;

	std::string current_server = "none";
	std::string steamid = "69";

	namespace alpha {
		namespace master {
			bool shoot_same_target_m = false;
			bool shoot_same_target_temp_m = false;

			bool walk_to_pos_m = false; //uses map marker
			bool walk_to_pos_temp_m = false; //uses map marker

			bool flyhack_m = false;

			bool walk_to_death_m = false;

			bool friends_m = false;

			bool control_aim_angles_m = false;

			bool force_join_server_m = false;

			bool follow_master_m = false;
			bool follow_master_temp_m = false;
		}

		bool shoot_same_target = false;

		bool walk_to_pos = false; //uses map marker

		bool flyhack = false;

		bool walk_to_death = false;

		bool friends = false;

		bool control_aim_angles = false;

		bool force_join_server = false;

		bool follow_master = false;
		//control certain features etc
	}

	namespace tr {
		bool manipulated = false;
		bool manipulate_visible = false;
		float desync_time = 0.f;
	}

	namespace auth {
		std::wstring username = wxorstr_(L"dev");
		std::wstring days_left = wxorstr_(L"0");
	}
}

uintptr_t game_assembly = 0;
uintptr_t unity_player = 0;