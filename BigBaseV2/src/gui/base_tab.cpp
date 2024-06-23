#include "common.hpp"
#include "base_tab.h"
#include "imgui.h"
#include "script.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"
#include "gta_util.hpp"
#include "../../util.hpp"

namespace big
{
	void base_tab::render_base_tab()
	{
		bool o_bool{};
		const double min = 0., max = 10.;
		const char* const demo_combo[]
		{
			"One",
			"Two",
			"Three"
		};

		if (ImGui::Begin("Cummer"))
		{
			if (ImGui::Checkbox("Bool", g_settings.options["demo bool"].get<bool*>()))
				g_settings.save();
			if (ImGui::SliderInt("Int", (PINT)g_settings.options["demo int"].get<int64_t*>(), 0, 10))
				g_settings.save();
			if (ImGui::SliderScalar("Double", ImGuiDataType_Double, g_settings.options["demo double"].get<double*>(), &min, &max))
				g_settings.save();
			if (ImGui::Combo("Combo", (PINT)g_settings.options["demo combo"].get<int64_t*>(), demo_combo, sizeof(demo_combo) / sizeof(*demo_combo)))
				g_settings.save();

			if (ImGui::Button("Cum")) { g_fiber_pool->queue_job([&] { g_util.createVeh(0xB779A091, playerCoord.x, playerCoord.y, playerCoord.z, 1); }); }

			ImGui::SameLine();

			if (ImGui::Button("Unload"))
			{
				g_running = false;
			}
		}
		ImGui::End();
	}
}
