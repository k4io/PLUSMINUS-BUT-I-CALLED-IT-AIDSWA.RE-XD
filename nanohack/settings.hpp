namespace settings {
	std::vector<uintptr_t> current_visible_players{};
	std::string data_dir = xorstr_("");

	bool instakill = false;

	bool cheat_init = false;

	bool open = false;

	bool panic = false;

	HWND console_window = 0;

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