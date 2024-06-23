#include "util.hpp"

namespace big
{
	void m_util::requestModel(Hash model)
	{
		while (!STREAMING::HAS_MODEL_LOADED(model))
		{
			STREAMING::REQUEST_MODEL(model);
			script::get_current()->yield(5ms);
		}
	}

	void m_util::createVeh(Vehicle model, float x, float y, float z, bool inVeh)
	{
		if (STREAMING::IS_MODEL_VALID(model) && STREAMING::IS_MODEL_A_VEHICLE(model))
		{
			requestModel(model);

			*(unsigned short*)g_pointers->m_model_spawn_bypass = 0x9090;
			auto veh = VEHICLE::CREATE_VEHICLE(model, x, y, z, ENTITY::GET_ENTITY_HEADING(playerPED), 1, 1, 1);
			*(unsigned short*)g_pointers->m_model_spawn_bypass = 0x0574;

			if (g_pointers->m_is_session_started)
			{
				DECORATOR::DECOR_SET_INT(veh, "MPBITSET", 0);
				if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(veh))
					NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(NETWORK::NET_TO_VEH(veh), 1);

				STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(veh);
			}

			if (inVeh)
				PED::SET_PED_INTO_VEHICLE(playerPED, veh, -1);
		}
		else
		{
			LOG(WARNING) << "Vehicle is not Valid!";
		}
	}
}