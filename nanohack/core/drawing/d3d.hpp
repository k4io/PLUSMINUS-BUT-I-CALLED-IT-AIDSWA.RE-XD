#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
bool menu_init = false;

void undo_hooks( );

namespace d3d {
	HRESULT present_hook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
		static Vector2 text_size = Vector2(0, 0);
		if (!device) {
			swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
			device->GetImmediateContext(&immediate_context);

			ID3D11Texture2D* renderTarget = nullptr;
			swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
			device->CreateRenderTargetView(renderTarget, nullptr, &render_target_view);
			renderTarget->Release( );

			Renderer::Init(swapChain);
			FGUI_INPUT_WIN32::OnEntryPoint();
			FGUI_D3D11::OnEntryPoint();

		}
		immediate_context->OMSetRenderTargets(1, &render_target_view, nullptr);
		immediate_context->RSGetViewports(&vps, &viewport);
		screen_size = { viewport.Width, viewport.Height };
		screen_center = { viewport.Width / 2.0f, viewport.Height / 2.0f };

		if (GetAsyncKeyState(VK_INSERT) & 1)
			settings::open = !settings::open;
		if (!settings::panic) {
			if (Renderer::new_frame(swapChain)) {

				float width = 210; //pos actually not width
				for (size_t i = 0; i < settings::auth::username.size(); i++)
					width += 6;

				Renderer::rounded_rectangle_filled(55, 15, width + 40, 20, Color3(43, 35, 61), 3.f);
				Renderer::rounded_box(54, 14, width + 39, 19, Color3(1, 26, 51), 3.f);

				Renderer::text(Vector2(55 + ((width + 40) / 2), 22), Color3(219, 219, 219), 20.f, true, true, wxorstr_(L"aidswa.re | %s | time left: %s days"), settings::auth::username.c_str(), settings::auth::days_left.c_str());

				if (!aidsware::ui::init)
				{
					aidsware::ui::OnSetupDevice();
				}
				
				if (aidsware::ui::get_bool(xorstr_("show peek assist checks")))
				{
					float r = aidsware::ui::get_float(xorstr_("rings")) * aidsware::ui::get_float(xorstr_("checks")) * 20.f;
					std::wstring w(std::to_wstring(r));
					//aidsware::ui::wrapper::label(std::to_string(r), Vector2(160.0f, 10.0f), aidsware::ui::Tabs::Combat);
					Renderer::rectangle_filled({ 10, 50 }, Vector2(110, 20), Color3(23, 25, 31));
					Renderer::rectangle_filled(Vector2(10, 50), Vector2(110, 3), Color3(1, 26, 51));

					Renderer::text({ 65, 62 }, Color3(222, 222, 222), 18.f, true, true, wxorstr_(L"%.f checks"), r);
				}
				

				aidsware::ui::vars::Container->Render();

				if (settings::cheat_init)
					entities::loop( );
				if (aidsware::ui::get_bool(xorstr_("draw targeting fov")))
					Renderer::circle(screen_center, aidsware::ui::get_color(xorstr_("targeting fov color")), aidsware::ui::get_float(xorstr_("target fov")), 1.f);
				//SleepEx(1, 0);
			}
			Renderer::end_frame();
		}
		else {
			static bool once = false;
			if (!once) {
				if(LocalPlayer::Entity())
					LocalPlayer::Entity()->playerFlags() |= ~PlayerFlags::IsAdmin;

				undo_hooks( );
				
				once = true;
			}
		}

		return present_original(swapChain, syncInterval, flags);
	}

	HRESULT resize_hook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
		Renderer::reset_canvas( );

		if (render_target_view)
			render_target_view->Release( );

		if (immediate_context)
			immediate_context->Release( );

		if (device)
			device->Release( );

		device = nullptr;

		return resize_original(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
	}

	bool init( ) {
		unity_player = (uintptr_t)GetModuleHandleA(xorstr_("UnityPlayer.dll"));
		game_assembly = (uintptr_t)GetModuleHandleA(xorstr_("GameAssembly.dll"));

		auto addr = mem::find_pattern(unity_player, (PBYTE)"\x48\x83\xEC\x28\xE8\x00\x00\x00\x00\x48\x8B\x80\xA0\x03\x00\x00", xorstr_("xxxxx????xxxxxxx"));
		
		if (!addr)
			return false;

		auto swapchain = reinterpret_cast<IDXGISwapChain * (__fastcall*)()>(addr)();

		if (swapchain) {
			auto table = *reinterpret_cast<PVOID**>(swapchain);
			present_original = reinterpret_cast<HRESULT(__fastcall*)(IDXGISwapChain*, UINT, UINT)>(table[ 8 ]);
			resize_original = reinterpret_cast<HRESULT(__fastcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>(table[ 13 ]);

			hookengine::hook(present_original, present_hook);
			hookengine::hook(resize_original, resize_hook);

			return true;
		}
		return false;
	}
}