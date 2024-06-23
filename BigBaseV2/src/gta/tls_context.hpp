#pragma once
#include <intrin.h>
#include "fwddec.hpp"

namespace rage
{
#pragma pack(push, 1)
	class tlsContext
	{
	public:
		char gap0[0xB4];                // 0x00 - 0xB4 (180 bytes)
		uint8_t m_unk_byte;             // 0xB4 - 0xB5 (1 byte)
		char gapB5[3];                  // 0xB5 - 0xB8 (3 bytes)
		sysMemAllocator* m_allocator;   // 0xB8 - 0xC0 (8 bytes)
		char gapC0[0x798];              // 0xC0 - 0x858 (1944 bytes)
		scrThread* m_script_thread;     // 0x858 - 0x860 (8 bytes)
		bool m_is_script_thread_active; // 0x860 - 0x861 (1 byte)

		static tlsContext* get()
		{
			return *reinterpret_cast<tlsContext**>(__readgsqword(0x58));
		}
	};
#pragma pack(pop)

	static_assert(sizeof(tlsContext) == 0x861, "tlsContext size is incorrect");
}