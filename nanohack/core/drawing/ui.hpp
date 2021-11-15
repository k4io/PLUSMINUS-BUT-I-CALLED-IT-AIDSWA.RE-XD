#pragma once

#include <filesystem>

namespace aidsware::ui
{
	static bool init = false;
	static FGUI::CBuilder ptrnBuilder;

	namespace vars
	{
		inline std::shared_ptr<FGUI::CContainer> Container;
		inline std::shared_ptr<FGUI::CTabPanel> Tabs;

		std::unordered_map<std::string, std::shared_ptr<FGUI::CCheckBox>> checkBoxes;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CButton>> buttons;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CSlider>> sliders;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CComboBox>> comboBoxes;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CColorPicker>> colorPickers;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CContainer>> groupBoxes;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CKeyBinder>> keybinders;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CTextBox>> textBoxes;
		std::unordered_map<std::string, std::shared_ptr<FGUI::CLabel>> labels;
	}

	bool is_menu_open()
	{
		return vars::Container.get()->GetState();
	}

	bool get_bool(std::string name)
	{
		if (vars::checkBoxes.contains(name))
			return vars::checkBoxes[name]->GetState();
		
		//printf("[checkbox] %s\n", name.c_str());

		return false;
	}

	std::size_t get_combobox(std::string name)
	{
		if (vars::comboBoxes.contains(name))
			return vars::comboBoxes[name]->GetIndex();
		
		//printf("[combo] %s\n", name.c_str());

		return 0;
	}

	float get_float(std::string name)
	{
		if(vars::sliders.contains(name))
			return vars::sliders[name]->GetValue();
		
		//printf("[slider] %s\n", name.c_str());

		return 0.0f;
	}

	std::string get_text(std::string name)
	{
		if (vars::textBoxes.contains(name))
			return vars::textBoxes[name]->GetText();

		//printf("[text] %s\n", name.c_str());

		return "";
	}

	Color3 get_color(std::string name)
	{
		if (vars::colorPickers.contains(name))
		{
			auto clr = vars::colorPickers[name]->GetColor();
			return Color3(clr.m_ucRed, clr.m_ucGreen, clr.m_ucBlue, clr.m_ucAlpha);
		}

		//printf("[color] %s\n", name.c_str());

		return Color3(219, 219, 219, 255.0f);
	}

	std::uint32_t get_keybind(std::string name)
	{
		if(vars::keybinders.contains(name))
			return vars::keybinders[name]->GetKey();

		//printf("[keybind] %s\n", name.c_str());

		return 0;
	}

	enum class Tabs : int
	{
		Combat,
		Visual,
		Misc,
		Colors
	};

	namespace wrapper
	{
		static int currentY = -15.0f;
		static int currentX = 10.f;

		void reset_width()
		{
			currentX = 110.f;
		}

		void reset_height()
		{
			currentY = -15.f;
		}

		void create_window(std::string name, Vector2 position, Vector2 size, std::uint32_t menu_key)
		{
			ptrnBuilder.Widget(vars::Container).Title(name).Position(position.x + currentX, position.y).Size(size.x, size.y).Key(menu_key).Font(xorstr_("Courier New"), 12, true, 1);
		}

		void button(std::string name, std::function<void()> callback, Vector2 position, Tabs tab = Tabs::Visual)
		{
			vars::buttons[name] = std::make_shared<FGUI::CButton>();
			ptrnBuilder.Widget(vars::buttons[name]).Title(name).Position(position.x + currentX, currentY + position.y).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).Callback(callback).SpawnIn(vars::Container, false);
			currentY += 25.0f;
		}

		void checkbox(std::string name, Vector2 position, Tabs tab = Tabs::Visual, bool keybind = false, bool colorpick = false, Vector2 cppos = Vector2(0,0))
		{
			auto text_size = Renderer::get_text_size(StringConverter::ToUnicode(name), 11.0f);
			vars::checkBoxes[name] = std::make_shared<FGUI::CCheckBox>();
			auto cb = ptrnBuilder.Widget(vars::checkBoxes[name]).Title(name).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			if (colorpick)
			{
				vars::colorPickers[name] = std::make_shared<FGUI::CColorPicker>();
				ptrnBuilder.Widget(vars::colorPickers[name]).Title(name + xorstr_(" color")).Position((text_size.x * 1.5) + 10.0f + position.x + cppos.x, currentY + position.y).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			}
			currentY += 20.0f;
			if (keybind)
			{
				vars::keybinders[name + xorstr_(" key")] = std::make_shared<FGUI::CKeyBinder>();
				ptrnBuilder.Widget(vars::keybinders[name + xorstr_(" key")]).Title(name + xorstr_(" key")).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
				currentY += 25.0f;
			}
			currentY += 5.0f;
		}

		void label(std::string name, Vector2 position, Tabs tab = Tabs::Visual, bool newline = true, bool fonticon = false, std::string e = "")
		{
			vars::labels[name] = std::make_shared<FGUI::CLabel>();
			ptrnBuilder.Widget(vars::labels[name]).Title(name).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			if(newline)
				currentY += 20.0f;
		}

		void color_picker(std::string name, Vector2 position, Tabs tab = Tabs::Visual)
		{
			vars::colorPickers[name] = std::make_shared<FGUI::CColorPicker>();
			ptrnBuilder.Widget(vars::colorPickers[name]).Title(name).Position(position.x + currentX, currentY + position.y).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			currentY += 20.0f;
		}

		void textbox(std::string name, Vector2 position, Tabs tab = Tabs::Visual, std::string value = "")
		{
			vars::textBoxes[name] = std::make_shared<FGUI::CTextBox>();
			ptrnBuilder.Widget(vars::textBoxes[name]).Title(name).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Text(value).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			currentY += 25.0f;
		}

		void combobox(std::string name, std::vector<std::string> options, Vector2 position, Tabs tab = Tabs::Visual)
		{
			vars::comboBoxes[name] = std::make_shared<FGUI::CComboBox>();
			auto widget = ptrnBuilder.Widget(vars::comboBoxes[name]).Title(name).Position(position.x + currentX, currentY + position.y).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab);
			
			for(auto opt : options)
				widget.Entry(opt);

			widget.SpawnIn(vars::Container, false);
			currentY += 25.0f;
		}

		void keybind(std::string name, Vector2 position, Tabs tab = Tabs::Visual)
		{
			vars::keybinders[name] = std::make_shared<FGUI::CKeyBinder>();
			ptrnBuilder.Widget(vars::keybinders[name]).Title(name).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			currentY += 25.0f;
		}

		void slider(std::string name, Vector2 position, float value, float min, float max, Tabs tab = Tabs::Combat)
		{
			vars::sliders[name] = std::make_shared<FGUI::CSlider>();
			ptrnBuilder.Widget(vars::sliders[name]).Title(name).Range(min, max).Value(value).Position(position.x + currentX, position.y + currentY).Font(xorstr_("Courier New"), 11).Medium(vars::Tabs, (int)tab).SpawnIn(vars::Container, false);
			currentY += 30.0f;
		}

		void tabs(std::vector<std::string> tabs)
		{
			auto builder = ptrnBuilder.Widget(vars::Tabs).Position(0, 0).Font(xorstr_("Courier New"), 16, true, 1);
			for (auto tab : tabs)
				builder.Tab(tab);
			
			builder.SpawnIn(vars::Container, false);
		}
	}
	std::vector<std::string> configs{};
	bool a123_ = false;
	inline void OnSetupDevice()
	{
		VM_EAGLE_BLACK_START

		a123_ = true;
		init = true;
		/*
		configs.clear();
		for (auto& f : std::filesystem::recursive_directory_iterator(settings::data_dir))
			if (f.path().extension() == ".cfg")
				configs.push_back(f.path().filename().string().substr(0, f.path().filename().string().find(".")));
				*/
		vars::checkBoxes = {};
		vars::buttons = {};
		vars::sliders = {};
		vars::comboBoxes = {};
		vars::colorPickers = {};
		vars::groupBoxes = {};
		vars::keybinders = {};

		vars::Container = std::make_shared<FGUI::CContainer>();
		vars::Tabs = std::make_shared<FGUI::CTabPanel>();
		wrapper::create_window(xorstr_("aidswa.re"), Vector2(200, 200), Vector2(530, 450), VK_INSERT);
		wrapper::currentY += 20.0f;
		wrapper::tabs({xorstr_("Combat"), xorstr_("Visuals"), xorstr_("Misc"), xorstr_("Colors") });

		// == Combat == \\
		
		wrapper::reset_width();
		wrapper::reset_height();
		wrapper::currentY -= 5.0f;
		wrapper::checkbox(xorstr_("psilent"), Vector2(0, 0), Tabs::Combat, true);
		wrapper::currentY += 5.0f;
		wrapper::checkbox(xorstr_("peek assist"), Vector2(0, 0), Tabs::Combat, true, false);
		wrapper::currentY += 10.0f;
		//wrapper::checkbox(xorstr_("silent melee"), Vector2(0, 0), Tabs::Combat);
		wrapper::slider(xorstr_("lerp"), Vector2(0, 0), 0.75f, 0.01f, 1.0f, Tabs::Combat);
		wrapper::currentY -= 5.f;	
		wrapper::checkbox(xorstr_("autoshoot"), Vector2(0, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("pierce"), Vector2(0, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("rapid fire"), Vector2(0, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("hitbox attraction"), Vector2(0, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("fat bullet"), Vector2(0, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("fast bullets"), Vector2(0, 0), Tabs::Combat);

		wrapper::checkbox(xorstr_("insta kill"), Vector2(0, 0), Tabs::Combat);
		wrapper::keybind(xorstr_("insta kill key"), Vector2(0, 0.0f), Tabs::Combat);
		wrapper::currentY += 10.f;
		wrapper::slider(xorstr_("counter"), Vector2(0, 0), 5.0f, 0.0f, 9.0f, Tabs::Combat);
		wrapper::slider(xorstr_("bullets"), Vector2(0, 0), 1.0f, 0.0f, 5.0f, Tabs::Combat);
		wrapper::currentY -= 5.f;
		wrapper::checkbox(xorstr_("with peek assist"), Vector2(0, 0), Tabs::Combat);
		wrapper::currentX += 20.f;
		wrapper::reset_height();

		//wrapper::currentY += 5.0f;
		wrapper::combobox(xorstr_("hitbox override"), { xorstr_("none"), xorstr_("body"), xorstr_("head"), xorstr_("random (all)"), xorstr_("random (main)") }, Vector2(180, 0), Tabs::Combat);
		wrapper::currentY += 5.f;
		wrapper::label(xorstr_("weapon properties"), Vector2(170.0f, -10.f), Tabs::Combat);
		wrapper::slider(xorstr_("target fov"), Vector2(180.f, 0.f), 300.0f, 30.0f, 2500.0f, Tabs::Combat);
		wrapper::slider(xorstr_("recoil %"), Vector2(180.0f, 0.0f), 100.0f, 0.0f, 100.0f, Tabs::Combat);
		wrapper::slider(xorstr_("spread %"), Vector2(180.0f, 0), 100.0f, 0.0f, 100.0f, Tabs::Combat);

		wrapper::currentY -= 10.0f;
		wrapper::checkbox(xorstr_("insta eoka"), Vector2(180.0f, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("automatic"), Vector2(180.0f, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("no sway"), Vector2(180.0f, 0), Tabs::Combat);

		wrapper::currentY += 10.0f;
		wrapper::label(xorstr_("peek assist properties"), Vector2(170.0f, -10.f), Tabs::Combat);
		wrapper::slider(xorstr_("max radius"), Vector2(180.0f, 10.0f), 7.f, 1.0f, 20.0f, Tabs::Combat);
		wrapper::slider(xorstr_("rings"), Vector2(180.0f, 10.0f), 15.f, 1.f, 360.0f, Tabs::Combat);
		wrapper::slider(xorstr_("checks"), Vector2(180.0f, 10.0f), 30.f, 1.f, 360.0f, Tabs::Combat);
		wrapper::slider(xorstr_("duration"), Vector2(180.0f, 10.0f), 0.05f, 0.001f, 60.0f, Tabs::Combat);

		wrapper::currentY += 5.0f;
		wrapper::checkbox(xorstr_("always heli weakspot"), Vector2(170, 0), Tabs::Combat);
		wrapper::checkbox(xorstr_("insta charge compound"), Vector2(170, 0), Tabs::Combat);
		wrapper::currentY += 10.0f;
		//float r = aidsware::ui::get_float(xorstr_("rings")) * aidsware::ui::get_float(xorstr_("checks")) * 20.f;
		//wrapper::label(std::to_string(r).c_str(), Vector2(170, 0), Tabs::Combat);

		
		wrapper::reset_height();
		wrapper::reset_width();

		// == Visuals == \\

		wrapper::checkbox(xorstr_("players"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("sleepers"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("npc"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("looking direction"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("distance"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("skeleton"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("box"), Vector2(0, 0), Tabs::Visual);
		wrapper::combobox(xorstr_("box type"), { xorstr_("cornered"), xorstr_("full"), xorstr_("rounded"), xorstr_("3 dimension")}, Vector2(10.f, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("hpbar"), Vector2(0, 0), Tabs::Visual);
		wrapper::combobox(xorstr_("hpbar type"), { xorstr_("sidebar"), xorstr_("bottombar") }, Vector2(10.f, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("reload indicator"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("target player belt"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("ores"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("stashes"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("corpses"), Vector2(0, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("traps"), Vector2(0, 0), Tabs::Visual);
		wrapper::reset_height();
		wrapper::currentX += 20.f;
		wrapper::checkbox(xorstr_("hemp"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("weapons"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("supply"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("workbench"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("crates"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("tool cupboards"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("storage"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("vehicles"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("flyhack indicator"), Vector2(170, 0), Tabs::Visual);
		//wrapper::checkbox(xorstr_("raid esp"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("chams"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("logs"), Vector2(170, 0), Tabs::Visual);
		wrapper::checkbox(xorstr_("debug"), Vector2(170, 0), Tabs::Visual);
		wrapper::currentY += 30.f;
		wrapper::slider(xorstr_("esp dist"), Vector2(170, 0), 100.0f, 0.0f, 400.0f, Tabs::Visual);
		wrapper::checkbox(xorstr_("draw targeting fov"), Vector2(170, 0.0f), Tabs::Visual);
		wrapper::checkbox(xorstr_("show peek assist checks"), Vector2(170, 0), Tabs::Visual);
		wrapper::reset_height();
		wrapper::reset_width();

		// == Misc == \\

		wrapper::currentY += 10.0f;
		wrapper::slider(xorstr_("fov"), Vector2(0, 0.0f), 90.0f, 30.0f, 160.0f, Tabs::Misc);
		wrapper::currentY -= 15.0f;
		wrapper::keybind(xorstr_("zoom key"), Vector2(0, 0.0f), Tabs::Misc);
		wrapper::currentY += 5.0f;
		wrapper::checkbox(xorstr_("bullet tracers"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("fake admin"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("fast loot"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("long hand"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("farm assist"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("no collisions"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("fake lag"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("spiderman"), Vector2(0, 0), Tabs::Misc);

		//wrapper::checkbox(xorstr_("spinbot"), Vector2(0, 0), Tabs::Misc);
		wrapper::combobox(xorstr_("anti-aim"),
			{
				xorstr_("none"),
				xorstr_("backwards"),
				xorstr_("backwards (down)"),
				xorstr_("backwards (up)"),
				xorstr_("left"),
				xorstr_("left (down)"),
				xorstr_("left (up)"),
				xorstr_("right"),
				xorstr_("left (down)"),
				xorstr_("left (up)"),
				xorstr_("jitter"),
				xorstr_("jitter (down)"),
				xorstr_("jitter (up)"),
				xorstr_("spin"),
				xorstr_("spin (down)"),
				xorstr_("spin (up)"),
				xorstr_("random")
			}, Vector2(0, 0), Tabs::Misc);

		wrapper::checkbox(xorstr_("infinite jump"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("can hold items"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("omnisprint"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("no fall"), Vector2(0, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("fake shots"), Vector2(0, 0), Tabs::Misc, true, false);
		wrapper::reset_height();
		wrapper::currentX += 20.f;
		wrapper::combobox(xorstr_("light"), { xorstr_("default"), xorstr_("dark"), xorstr_("light") }, Vector2(170, 0), Tabs::Misc);
		wrapper::keybind(xorstr_("timescale key"), Vector2(170, 0), Tabs::Misc);
		wrapper::keybind(xorstr_("desync on key"), Vector2(170, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("long neck"), Vector2(170, 0), Tabs::Misc);
		wrapper::combobox(xorstr_("crosshair"), { xorstr_("none"), xorstr_("plusminus"), xorstr_("evilcheats"), xorstr_("circle"), xorstr_("swastika") }, Vector2(170, 0), Tabs::Misc);

		wrapper::textbox(xorstr_("config name"), Vector2(170, 0), Tabs::Misc, xorstr_("default"));
		//wrapper::combobox(xorstr_("config"), configs, Vector2(170, 0), Tabs::Misc);

		wrapper::button(xorstr_("Save Config"), [&]() {
			if (!get_text(xorstr_("config name")).empty())
				vars::Container->SaveToFile(settings::data_dir + xorstr_("\\") + get_text(xorstr_("config name")) + xorstr_(".cfg"));
		}, Vector2(170, 0), Tabs::Misc);

		wrapper::button(xorstr_("Load Config"), [&]() {
			if (!get_text(xorstr_("config name")).empty())
				vars::Container->LoadFromFile(settings::data_dir + xorstr_("\\") + get_text(xorstr_("config name")) + xorstr_(".cfg"));
			/*
			for (size_t i = 0; i < configs.size(); i++)
				if (get_combobox(xorstr_("config")) == i)
					vars::Container->LoadFromFile(settings::data_dir + xorstr_("\\") + configs[i] + xorstr_(".cfg"));*/
		}, Vector2(170, 0), Tabs::Misc);

		wrapper::button(xorstr_("Panic"), [&]() {
			settings::panic = true;
		}, Vector2(170, 0), Tabs::Misc);

		//wrapper::checkbox(xorstr_("invisible gun"), Vector2(325.0f, 0), Tabs::Misc);
		//wrapper::keybind(xorstr_("silent walk key"), Vector2(325.f, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("flyhack stop"), Vector2(170, 0), Tabs::Misc);
		wrapper::currentY += 20.f;
		wrapper::slider(xorstr_("threshold"), Vector2(170, 0), 100.0f, 0.0f, 400.0f, Tabs::Misc);
		wrapper::checkbox(xorstr_("walk to marker"), Vector2(170, 0), Tabs::Misc);
		wrapper::checkbox(xorstr_("custom box"), Vector2(170, 0), Tabs::Misc);
		wrapper::textbox(xorstr_("custom box path"), Vector2(170, 0), Tabs::Misc, xorstr_("box name"));
		wrapper::checkbox(xorstr_("custom hitsound"), Vector2(170, 0), Tabs::Misc);
		wrapper::textbox(xorstr_("hitsound path"), Vector2(170, 0), Tabs::Misc, xorstr_("hitsound"));
		wrapper::checkbox(xorstr_("weapon spam"), Vector2(170, 0), Tabs::Misc);
		//wrapper::checkbox(xorstr_("test"), Vector2(170, 0), Tabs::Combat);
		wrapper::reset_height();
		wrapper::reset_width();

		// == Colors == \\

		wrapper::label(xorstr_("players"), Vector2(0, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible"), Vector2(30, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible players"), Vector2(75, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(30, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible chams"), Vector2(75, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible players"), Vector2(80, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(30, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible chams"), Vector2(80, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible teammate"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible teammate"), Vector2(125, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible teammate chams"), Vector2(125, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible teammate"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible teammate"), Vector2(135, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible teammate chams"), Vector2(135, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible skeleton"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible skeleton players"), Vector2(115, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible skeleton"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible skeleton players"), Vector2(135, 0), Tabs::Colors);

		wrapper::label(xorstr_("sleepers"), Vector2(0, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible sleepers"), Vector2(65, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible sleepers"), Vector2(80, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible skeleton"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible skeleton"), Vector2(125, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible skeleton"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible skeleton"), Vector2(135, 0), Tabs::Colors);

		wrapper::label(xorstr_("misc"), Vector2(0, 0), Tabs::Colors);
		wrapper::label(xorstr_("weapon"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("weapon color"), Vector2(65, 0), Tabs::Colors);
		wrapper::label(xorstr_("supply"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("supply color"), Vector2(65, 0), Tabs::Colors);
		wrapper::label(xorstr_("workbench"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("workbench color"), Vector2(75, 0), Tabs::Colors);
		wrapper::label(xorstr_("crates"), Vector2(10, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("crate color"), Vector2(65, 0), Tabs::Colors);
		wrapper::currentY -= 75.f;
		wrapper::label(xorstr_("tc"), Vector2(100, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("tc color"), Vector2(145, 0), Tabs::Colors);
		wrapper::label(xorstr_("boxes"), Vector2(100, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("box color"), Vector2(145, 0), Tabs::Colors);
		wrapper::reset_height();
		wrapper::currentX += 20.f;
		wrapper::label(xorstr_("looking direction"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("looking direction color"), Vector2(250, 0), Tabs::Colors);
		wrapper::label(xorstr_("target fov"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("targeting fov color"), Vector2(230, 0), Tabs::Colors);
		wrapper::label(xorstr_("npcs"), Vector2(130, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible npcs"), Vector2(195, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible npcs chams"), Vector2(195, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible npcs"), Vector2(210, 0), Tabs::Colors);
		wrapper::label(xorstr_("chams"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible npcs chams"), Vector2(210, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible skeleton"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible skeleton npc"), Vector2(255, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible skeleton"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible skeleton npc"), Vector2(275, 0), Tabs::Colors);
		wrapper::label(xorstr_("ores"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("ores color"), Vector2(170, 0), Tabs::Colors);
		wrapper::label(xorstr_("stashes"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("stashes color"), Vector2(180, 0), Tabs::Colors);
		wrapper::label(xorstr_("corpses"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("corpses color"), Vector2(180, 0), Tabs::Colors);
		wrapper::label(xorstr_("traps"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("traps color"), Vector2(170, 0), Tabs::Colors);
		wrapper::label(xorstr_("hemp"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("hemp color"), Vector2(170, 0), Tabs::Colors);
		wrapper::label(xorstr_("vehicles"), Vector2(130, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("vehicles color"), Vector2(190, 0), Tabs::Colors);
		wrapper::label(xorstr_("boxes"), Vector2(130, 0), Tabs::Colors);
		wrapper::label(xorstr_("visible"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("visible boxes"), Vector2(190, 0), Tabs::Colors);
		wrapper::label(xorstr_("invisible"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("invisible boxes"), Vector2(210, 0), Tabs::Colors);
		wrapper::label(xorstr_("targeted"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("targeted boxes"), Vector2(205, 0), Tabs::Colors);
		wrapper::label(xorstr_("los"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("los checks"), Vector2(205.f, 0), Tabs::Colors);
		wrapper::label(xorstr_("insta kill"), Vector2(140, 0), Tabs::Colors, false);
		wrapper::color_picker(xorstr_("insta kill indicator"), Vector2(205.f, 0), Tabs::Colors);
		VM_EAGLE_BLACK_END
	}
}