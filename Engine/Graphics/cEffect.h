#pragma once

#include "cRenderState.h"
#include "cShader.h"

#include <Engine/Assets/ReferenceCountedAssets.h>

namespace eae6320
{
	namespace Graphics
	{
		class cEffect {

		public:
			static cResult Load(const char p_EffectPath[], cEffect*& o_Effect);

			void Bind();

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

		private:
			cEffect() = default;
			~cEffect();

			cResult InitializeShadingData(const char p_EffectPath[]);
			cResult CleanUp();

			cResult CleanUpShader();
			void BindRenderState();
			cResult LoadShader(const char p_EffectPath[]);

			cShader* m_VertexShader = nullptr;
			cShader* m_FragmentShader = nullptr;
			cRenderState m_RenderState;

#if defined( EAE6320_PLATFORM_GL )
			GLuint m_ProgramId = 0;
#endif

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
		};
	}
}