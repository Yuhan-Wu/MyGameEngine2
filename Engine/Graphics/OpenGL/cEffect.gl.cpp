#include "../cEffect.h"

#include "Includes.h"

#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Logging/Logging.h>

namespace eae6320
{
	namespace Graphics
	{
		cResult cEffect::CleanUp() {
			auto result = Results::Success;
			if (m_ProgramId != 0)
			{
				glDeleteProgram(m_ProgramId);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = eae6320::Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				m_ProgramId = 0;
			}
			cResult clean_shader_result = CleanUpShader();
			if (!clean_shader_result) {
				if (result) {
					result = clean_shader_result;
				}
			}
			return result;
		}

		cResult cEffect::InitializeShadingData(const char p_EffectPath[]) {
			auto result = LoadShader(p_EffectPath);
			if (!result) {
				return result;
			}
			GLuint* ProgramId = &m_ProgramId;
			// Create a program
			eae6320::cScopeGuard scopeGuard_program([&result, ProgramId]
				{
					if (!result)
					{
						if ((*ProgramId) != 0)
						{
							glDeleteProgram(*ProgramId);
							const auto errorCode = glGetError();
							if (errorCode != GL_NO_ERROR)
							{
								EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
								eae6320::Logging::OutputError("OpenGL failed to delete the program: %s",
									reinterpret_cast<const char*>(gluErrorString(errorCode)));
							}
							(*ProgramId) = 0;
						}
					}
				});
			{
				EAE6320_ASSERT(m_ProgramId == 0);
				m_ProgramId = glCreateProgram();
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to create a program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
				else if (m_ProgramId == 0)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERT(false);
					eae6320::Logging::OutputError("OpenGL failed to create a program");
					return result;
				}
			}
			// Attach the shaders to the program
			{
				// Vertex
				{
					EAE6320_ASSERT((m_VertexShader != nullptr) && (m_VertexShader->m_shaderId != 0));
					glAttachShader(m_ProgramId, m_VertexShader->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the vertex shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				// Fragment
				{
					EAE6320_ASSERT((m_FragmentShader != nullptr) && (m_FragmentShader->m_shaderId != 0));
					glAttachShader(m_ProgramId, m_FragmentShader->m_shaderId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to attach the fragment shader to the program: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
			}
			// Link the program
			{
				glLinkProgram(m_ProgramId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					// Get link info
					// (this won't be used unless linking fails
					// but it can be useful to look at when debugging)
					std::string linkInfo;
					{
						GLint infoSize;
						glGetProgramiv(m_ProgramId, GL_INFO_LOG_LENGTH, &infoSize);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							if (infoSize > 0)
							{
								auto* const info = new (std::nothrow) GLchar[infoSize];
								if (info)
								{
									eae6320::cScopeGuard scopeGuard_info([info]
										{
											delete[] info;
										});
									constexpr GLsizei* const dontReturnLength = nullptr;
									glGetProgramInfoLog(m_ProgramId, static_cast<GLsizei>(infoSize), dontReturnLength, info);
									const auto errorCode = glGetError();
									if (errorCode == GL_NO_ERROR)
									{
										linkInfo = info;
									}
									else
									{
										result = eae6320::Results::Failure;
										EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
										eae6320::Logging::OutputError("OpenGL failed to get link info of the program: %s",
											reinterpret_cast<const char*>(gluErrorString(errorCode)));
										return result;
									}
								}
								else
								{
									result = eae6320::Results::OutOfMemory;
									EAE6320_ASSERTF(false, "Couldn't allocate memory for the program link info");
									eae6320::Logging::OutputError("Failed to allocate memory for the program link info");
									return result;
								}
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to get the length of the program link info: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					// Check to see if there were link errors
					GLint didLinkingSucceed;
					{
						glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &didLinkingSucceed);
						const auto errorCode = glGetError();
						if (errorCode == GL_NO_ERROR)
						{
							if (didLinkingSucceed == GL_FALSE)
							{
								result = eae6320::Results::Failure;
								EAE6320_ASSERTF(false, linkInfo.c_str());
								eae6320::Logging::OutputError("The program failed to link: %s",
									linkInfo.c_str());
								return result;
							}
						}
						else
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to find out if linking of the program succeeded: %s",
								reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to link the program: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}

			return result;
		}

		void cEffect::Bind() {
			// Bind the shading data

			{
				EAE6320_ASSERT(m_ProgramId != 0);
				glUseProgram(m_ProgramId);
				EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
			}
			// Render state
			{
				BindRenderState();
			}

		}
	}
}