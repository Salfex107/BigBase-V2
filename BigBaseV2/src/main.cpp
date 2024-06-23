#include "common.hpp"
#include "features.hpp"
#include "fiber_pool.hpp"
#include "gui.hpp"
#include "logger.hpp"
#include "hooking.hpp"
#include "pointers.hpp"
#include "renderer.hpp"
#include "script_mgr.hpp"

BOOL APIENTRY DllMain(HMODULE hmod, DWORD reason, PVOID)
{
	using namespace big;
	if (reason == DLL_PROCESS_ATTACH)
	{

		DisableThreadLibraryCalls(hmod);

		g_hmodule = hmod;
		g_main_thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD
		{
			while (!FindWindow(L"grcWindow", L"Grand Theft Auto V"))
				std::this_thread::sleep_for(20ms);

			auto logger_instance = std::make_unique<logger>();
			try
			{
				LOG(RAW) << u8R"kek(
 ______  _       ______                        ______  
(____  \(_)     (____  \                      (_____ \ 
 ____)  )_  ____ ____)  ) ____  ___  ____ _   _ ____) )
|  __  (| |/ _  |  __  ( / _  |/___)/ _  ) | | /_____/ 
| |__)  ) ( ( | | |__)  | ( | |___ ( (/ / \ V /_______ 
|______/|_|\_|| |______/ \_||_(___/ \____) \_/(_______)
          (_____|)kek";
				auto pointers_instance = std::make_unique<pointers>();
				auto renderer_instance = std::make_unique<renderer>();
				auto fiber_pool_instance = std::make_unique<fiber_pool>(10);
				auto hooking_instance = std::make_unique<hooking>();
				LOG(INFO) << "Components initialized.";

				g_settings.load();
				LOG(INFO) << "Settings Loaded.";

				g_script_mgr.add_script(std::make_unique<script>(&features::script_func));
				g_script_mgr.add_script(std::make_unique<script>(&gui::script_func));
				LOG(INFO) << "Scripts registered.";

				g_hooking->enable();
				LOG(INFO) << "Hooking enabled.";

				while (g_running)
				{
					std::this_thread::sleep_for(200ms);
				}

				g_hooking->disable();
				LOG(INFO) << "Hooking disabled.";

				std::this_thread::sleep_for(200ms);

				g_script_mgr.remove_all_scripts();
				LOG(INFO) << "Scripts unregistered.";

				hooking_instance.reset();
				fiber_pool_instance.reset();
				renderer_instance.reset();
				pointers_instance.reset();
				LOG(INFO) << "Components uninitialized.";
			}
			catch (std::exception const &ex)
			{
				LOG(FATAL) << ex.what();
				MessageBoxA(nullptr, ex.what(), nullptr, MB_OK | MB_ICONEXCLAMATION);
			}

			LOG(INFO) << "Farewell!";
			logger_instance.reset();

			CloseHandle(g_main_thread);
			FreeLibraryAndExitThread(g_hmodule, 0);
		}, nullptr, 0, &g_main_thread_id);
	}

	return true;
}
