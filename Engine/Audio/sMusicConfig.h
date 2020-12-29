#pragma once

#include <string>

namespace eae6320
{
	namespace Audio 
	{
		struct sMusicConfig {
			float m_Volume = 1;
			float m_pitch = 1;
			bool m_IsLoop = false;
			bool m_Autoplay = false;
			// TODO Effects
		};
	}
}