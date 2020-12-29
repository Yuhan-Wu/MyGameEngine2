#include "../cEffect.h"

#include "Includes.h"
#include "../sContext.h"

namespace eae6320
{
	namespace Graphics
	{
		cResult cEffect::CleanUp() {
			return CleanUpShader();
		}

		cResult cEffect::InitializeShadingData(const char p_EffectPath[]) {
			return LoadShader(p_EffectPath);
		}

		void cEffect::Bind() {
			// Bind the shading data
			{
				auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
				{
					constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
					constexpr unsigned int interfaceCount = 0;
					// Vertex shader
					{
						EAE6320_ASSERT((m_VertexShader != nullptr) && (m_VertexShader->m_shaderObject.vertex != nullptr));
						direct3dImmediateContext->VSSetShader(m_VertexShader->m_shaderObject.vertex, noInterfaces, interfaceCount);
					}
					// Fragment shader
					{
						EAE6320_ASSERT((m_FragmentShader != nullptr) && (m_FragmentShader->m_shaderObject.vertex != nullptr));
						direct3dImmediateContext->PSSetShader(m_FragmentShader->m_shaderObject.fragment, noInterfaces, interfaceCount);
					}
				}
				// Render state
				{
					BindRenderState();
				}
			}
		}
	}
}