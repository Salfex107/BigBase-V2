#include "common.hpp"
#include <natives.hpp>
#include <pointers.hpp>

#define playerID PLAYER::PLAYER_ID()
#define playerPED PLAYER::PLAYER_PED_ID()
#define playerCoord ENTITY::GET_ENTITY_COORDS(playerPED, 1)

namespace big
{
	class m_util
	{
	public:
		void requestModel(Hash model);
		void createVeh(Vehicle model, float x, float y, float z, bool inVeh);

	}; inline m_util g_util;
}