#define NOMINMAX
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define CRYPTOPP_UNCAUGHT_EXCEPTION_AVAILABLE
//#define THEM
#define authh
#define _WINSOCKAPI_

//#include <VMProtectSDK.h>

#include <Windows.h>
#include <stdint.h>
#include <Windows.h>
#include <psapi.h>
#include <d3d11.h>
#include <string>
#include <codecvt>
#include <locale>
#include <cstdint>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <emmintrin.h>
#include <comdef.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <lmcons.h>
#include <thread>
#include <map>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Winmm.lib")

//#include <ThemidaSDK.h>

/*
#define //VM_TIGER_WHITE_START
#define //VM_TIGER_WHITE_END

#define //VM_TIGER_RED_START
#define //VM_TIGER_RED_END

#define //VM_TIGER_BLACK_START
#define //VM_TIGER_BLACK_END

#define //VM_FISH_WHITE_START
#define //VM_FISH_WHITE_END

#define //VM_FISH_RED_START
#define //VM_FISH_RED_END

#define //VM_FISH_BLACK_START
#define //VM_FISH_BLACK_END

#define //VM_PUMA_WHITE_START
#define //VM_PUMA_WHITE_END

#define //VM_PUMA_RED_START
#define //VM_PUMA_RED_END

#define //VM_PUMA_BLACK_START
#define //VM_PUMA_BLACK_END

#define //VM_SHARK_WHITE_START
#define //VM_SHARK_WHITE_END

#define //VM_SHARK_RED_START
#define //VM_SHARK_RED_END 

#define //VM_SHARK_BLACK_START
#define //VM_SHARK_BLACK_END

#define //VM_DOLPHIN_WHITE_START
#define //VM_DOLPHIN_WHITE_END

#define //VM_DOLPHIN_RED_START
#define //VM_DOLPHIN_RED_END

#define //VM_DOLPHIN_BLACK_START
#define //VM_DOLPHIN_BLACK_END

#define //VM_EAGLE_WHITE_START
#define //VM_EAGLE_WHITE_END

#define //VM_EAGLE_RED_START
#define //VM_EAGLE_RED_END

#define //VM_EAGLE_BLACK_START
#define //VM_EAGLE_BLACK_END

#define //VM_MUTATE_ONLY_START
#define //VM_MUTATE_ONLY_END
*/
#pragma warning ( disable : 4172 )

#include "core/sdk/utils/string.hpp"
#include "core/sdk/utils/xorstr.hpp"
#include "core/sdk/utils/xorf.hpp"
#include "utils/WinReg.hpp"
#include "utils/Fingerprint.hpp"
#include "utils/Cryptography.hpp"

#include "settings.hpp"
#include "core/sdk/vector.hpp"
#include "core/stdafx.hpp"
#include "core/drawing/renderer.hpp"

#define FGUI_IMPLEMENTATION
#define FGUI_USE_D2D
#include "FGUI/FGUI.hpp"
#include "core/drawing/fgui/FInput.hpp"
#include "core/drawing/fgui/FRenderer.hpp"
#include "core/drawing/ui.hpp"

#include "core/sdk/utils/hookengine.hpp"
#include "core/sdk/mem.hpp"
#include "core/sdk/utils/crc32.hpp"
#include "core/sdk/il2cpp/wrapper.hpp"
#include "core/sdk/il2cpp/dissector.hpp"
//#include "core/sdk/il2cpp/il2cpp_lib.hpp" XD
#include "core/sdk/structs.hpp"
#include "core/sdk/game.hpp"
#include "core/main/cache.hpp"
#include "core/main/other.hpp"
#include "core/sdk/utils/math.hpp"
#include "core/main/entities.hpp"
#include "core/drawing/d3d.hpp"
#include "core/main/aimutils.hpp"
#include "core/main/hooks.hpp"

#include "cfgmanager.hpp"

#include "Request.hpp"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

//#include "cpr/cpr.h"

#define _(args) xorstr_(args)

std::string get_username(std::string file)
{
	std::string r;
	configmanager->GetValue(xorstr_("logindata"), xorstr_("username"), &r, file.c_str());
	return r;
}

std::string get_pwd(std::string file)
{
	std::string r;
	configmanager->GetValue(xorstr_("logindata"), xorstr_("password"), &r, file.c_str());
	return r;
}
#define MAX_LINE 255
void entry_thread() {
	//VM_DOLPHIN_BLACK_START
	//VMProtectBeginUltra(xorstr_("entry"));
	PWSTR szPath = NULL;
	
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &szPath)))
	{
		std::filesystem::create_directories(StringConverter::ToASCII(std::wstring(szPath) + wxorstr_(L"\\aidswa.re")));
		std::filesystem::create_directories(StringConverter::ToASCII(std::wstring(szPath) + wxorstr_(L"\\aidswa.re\\images")));
		std::filesystem::create_directories(StringConverter::ToASCII(std::wstring(szPath) + wxorstr_(L"\\aidswa.re\\sounds")));

		settings::data_dir = StringConverter::ToASCII(std::wstring(szPath) + wxorstr_(L"\\aidswa.re"));
		
		std::string username = xorstr_("");
		std::string password = xorstr_("");
		std::string hwid, hwidtoken, respondedhwid;
		bool success;
		std::string fn = settings::data_dir + xorstr_("\\auth.ini");
		username = get_username(fn);
		password = get_pwd(fn);
		using namespace http;
		using namespace rapidjson;
		try // try to make a request
		{
			Request request(_("http://rustche.at/index.php/api/auth"));

			// making the post request
			const Response postResponse = request.send(_("POST"), _("login=") + username + _("&password=") + password, {
			_("Content-Type: application/x-www-form-urlencoded"), _("XF-Api-Key: UixWAD_Vg_Rv_O2yV5eOFvgvqkzS1Zwd")
				});
			
			Document document;
			document.Parse(string(postResponse.body.begin(), postResponse.body.end()).c_str());

			assert(document[_("success")].IsBool()); // checking if the resonded function "success" is a bool
			success = document[_("success")].GetBool(); // defining our local variable "success" to be the responded bool
			if (!success)
				exit(-1);
            if (Value* user = GetValueByPointer(document, "/user"))
            {
                if ((*user)[_("is_staff")].GetBool() || (*user)[_("is_admin")].GetBool() || (*user)[_("is_moderator")].GetBool() || (*user)[_("is_super_admin")].GetBool() == true) { // we check if either one of those staff variables are true
					settings::auth::username = std::wstring(username.begin(), username.end());
                }
                else
                {
                    if (Value* v = GetValueByPointer(document, "/user")) {
                        const Value& secondary_group = (*v)[_("secondary_group_ids")];
                        assert(secondary_group.IsArray());
                        if (secondary_group == NULL
                            || secondary_group.ObjectEmpty()
                            || secondary_group.Empty())
                        {
                            exit(-1);
                        }
						if (secondary_group[0].GetInt() == 5 || secondary_group[0].GetInt() == 8)
						{
							try
							{
								hwid = SystemFingerprint::CreateUniqueFingerprint()->ToString();
								hwidtoken = _("69420");

								Request requesthwid(_("http://rustche.at/auth/hwid.php?username=") + username + _("&token=") + hwidtoken + _("&hwid=") + hwid); // we will use now the hwid.php for the new request

								const http::Response getResponseHWID = requesthwid.send(_("GET"));
								respondedhwid = string(getResponseHWID.body.begin(), getResponseHWID.body.end());

								//get days left
								Request requestdaysleft(_("http://rustche.at/auth/days_left.php?username=") + username + _("&token=69420"));
								const http::Response _days = requestdaysleft.send(_("GET"));
								string days_str = string(_days.body.begin(), _days.body.end());
								settings::auth::days_left = std::wstring(days_str.begin(), days_str.end());

								if (respondedhwid == _("new")) // if there is no hwid set
								{
									settings::auth::username = std::wstring(username.begin(), username.end());
								}
								else if (respondedhwid == _("accepted")) // if hwid check was successful
								{
									settings::auth::username = std::wstring(username.begin(), username.end());
								}
								else if (respondedhwid == _("declined"))
								{
									exit(-1);
								}
								else
								{
									exit(-1);
									// some other error within the system
								}
							}
							catch (std::exception& e)
							{
								exit(-1);
							}
						}
						else // no subscription - wrong usergroup
						{
							exit(-1);
						}
                    }
                }
            }
		}
		catch (std::exception e)
		{
			exit(-1);
		}
	}


	HRESULT dl;

	typedef HRESULT(WINAPI* URLDownloadToFileA_t)(LPUNKNOWN pCaller, LPCSTR szURL, LPCSTR szFileName, DWORD dwReserved, void* lpfnCB);
	URLDownloadToFileA_t xURLDownloadToFileA;
	xURLDownloadToFileA = (URLDownloadToFileA_t)GetProcAddress(LoadLibraryA(xorstr_("urlmon")), xorstr_("URLDownloadToFileA"));

	std::string url = std::string(xorstr_("http://185.132.38.210/assets/awlogo.png"));
	std::string url1 = std::string(xorstr_("http://185.132.38.210/assets/menu69.png"));
	std::string url2 = std::string(xorstr_("http://185.132.38.210/assets/weapon.png"));
	std::string url3 = std::string(xorstr_("http://185.132.38.210/assets/visuals.png"));
	std::string url4 = std::string(xorstr_("http://185.132.38.210/assets/misc.png"));
	std::string url5 = std::string(xorstr_("http://185.132.38.210/assets/color.png"));
	std::string destination = std::string(settings::data_dir + xorstr_("\\images\\awlogo.png"));
	std::string destination1 = std::string(settings::data_dir + xorstr_("\\images\\menu.png"));
	std::string destination2 = std::string(settings::data_dir + xorstr_("\\images\\weapon.png"));
	std::string destination3 = std::string(settings::data_dir + xorstr_("\\images\\visuals.png"));
	std::string destination4 = std::string(settings::data_dir + xorstr_("\\images\\misc.png"));
	std::string destination5 = std::string(settings::data_dir + xorstr_("\\images\\color.png"));

	dl = xURLDownloadToFileA(NULL, url.c_str(), destination.c_str(), 0, NULL);
	dl = xURLDownloadToFileA(NULL, url1.c_str(), destination1.c_str(), 0, NULL);
	dl = xURLDownloadToFileA(NULL, url2.c_str(), destination2.c_str(), 0, NULL);
	dl = xURLDownloadToFileA(NULL, url3.c_str(), destination3.c_str(), 0, NULL);
	dl = xURLDownloadToFileA(NULL, url4.c_str(), destination4.c_str(), 0, NULL);
	dl = xURLDownloadToFileA(NULL, url5.c_str(), destination5.c_str(), 0, NULL);

	d3d::init();

	AllocConsole( );
	SetConsoleTitleA(xorstr_("dbg"));
	settings::console_window = GetConsoleWindow();
	freopen_s(reinterpret_cast<FILE**>(stdin), xorstr_("CONIN$"), xorstr_("r"), stdin);
	freopen_s(reinterpret_cast<FILE**>(stdout), xorstr_("CONOUT$"), xorstr_("w"), stdout);
	ShowWindow(settings::console_window, SW_HIDE);


	//VMProtectEnd();
	initialize_cheat();
	//VM_DOLPHIN_BLACK_END
	do_hooks();
}

//extern "C" __declspec(dllexport) int Gamer()
//{
//	MessageBoxA(0, "Success", "gamer", 0);
//	return 1337;
//}

bool DllMain(HMODULE hMod, uint32_t call_reason, LPVOID reserved) {
	if (call_reason == DLL_PROCESS_ATTACH)
	{
		const auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(entry_thread), 0, 0, nullptr);
		if (handle != NULL)
			CloseHandle(handle);
	}
	return true;
}