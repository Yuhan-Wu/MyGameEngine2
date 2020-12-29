#pragma once

namespace eae6320
{
	namespace Audio
	{
		struct sEffect {
			bool m_Reverb = false;
			// [0, 100]
			float m_Density = 100;
			// [0.1, inf]
			float m_DecayTime = 1;
			// [1, 100] in feet
			float m_RoomSize = 100;
			// [0, 100]
			float m_WetDryMix = 100;
			// [0, 30]
			BYTE m_PositionLeft = 6;
			// [0, 30]
			BYTE m_PositionRight = 6;
			// [0, 300] in ms
			UINT32 m_ReflectionDel = 5;
		};
	}
}