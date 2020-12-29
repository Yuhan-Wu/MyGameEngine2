#include "cEffect.h"

#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>

namespace eae6320
{
	namespace Graphics
	{

		cResult cEffect::Load(const char p_EffectPath[], cEffect*& o_Effect) {
			auto result = Results::Success;

			EAE6320_ASSERT(o_Effect == nullptr);

			cEffect* new_Effect = nullptr;
			cScopeGuard scopeGuard([&o_Effect, &result, &new_Effect]
				{
					if (result)
					{
						EAE6320_ASSERT(new_Effect != nullptr);
						o_Effect = new_Effect;
					}
					else
					{
						if (new_Effect)
						{
							new_Effect->DecrementReferenceCount();
							new_Effect = nullptr;
						}
						o_Effect = nullptr;
					}
				});

			// Allocate a new vertex format
			{
				new_Effect = new (std::nothrow) cEffect();
				if (!new_Effect)
				{
					result = Results::OutOfMemory;
					EAE6320_ASSERTF(false, "Couldn't allocate memory for an effect");
					Logging::OutputError("Failed to allocate memory for an effect");
					return result;
				}
			}

			// Initialize the platform-specific vertex format
			if (!(result = new_Effect->InitializeShadingData(p_EffectPath)))
			{
				EAE6320_ASSERTF(false, "Initialization of new effect failed");
				return result;
			}

			return result;
		}

		cEffect::~cEffect() {
			EAE6320_ASSERT(m_referenceCount == 0);
			const auto result = CleanUp();
			EAE6320_ASSERT(result);
		}

		cResult cEffect::CleanUpShader() {
			if (m_VertexShader)
			{
				m_VertexShader->DecrementReferenceCount();
				m_VertexShader = nullptr;
			}
			if (m_FragmentShader)
			{
				m_FragmentShader->DecrementReferenceCount();
				m_FragmentShader = nullptr;
			}
			return Results::Success;
		}

		void cEffect::BindRenderState() {
			m_RenderState.Bind();
		}

		cResult cEffect::LoadShader(const char p_EffectPath[]) {
			auto result = eae6320::Results::Success;

			// Load binary file
			Platform::sDataFromFile data_from_file;
			std::string err_message;

			result = Platform::LoadBinaryFile(p_EffectPath, data_from_file, &err_message);


			if (!result) {
				Logging::OutputError(err_message.c_str());
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(data_from_file.data);
			const auto finalOffset = currentOffset + data_from_file.size;

			uint8_t render_state = *reinterpret_cast<uint8_t*>(currentOffset);
			currentOffset += sizeof(render_state);

			{
				if (!(result = m_RenderState.Initialize(render_state)))
				{
					EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
					return result;
				}
			}
			
			const char* vertex_path = reinterpret_cast<char*>(currentOffset);
			currentOffset += strlen(vertex_path) + 1;
			
			if (vertex_path && !(result = eae6320::Graphics::cShader::Load(vertex_path,
				m_VertexShader, eae6320::Graphics::eShaderType::Vertex)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
				return result;
			}

			const char* fragment_path = reinterpret_cast<char*>(currentOffset);
			currentOffset += strlen(fragment_path);
			
			if (fragment_path && !(result = eae6320::Graphics::cShader::Load(fragment_path,
				m_FragmentShader, eae6320::Graphics::eShaderType::Fragment)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
				return result;
			}
			

			return result;
		}
	}
}