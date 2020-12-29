#include "../cMesh.h"

#include "includes.h"
#include "../VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <algorithm>

namespace eae6320 {
	namespace Graphics {
		cResult cMesh::InitializeGeometry16(uint16_t p_IndexCount, uint16_t p_VertexCount,
			eae6320::Graphics::VertexFormats::sVertex_mesh p_VertexData[], uint16_t p_IndexData[]) {
			auto result = eae6320::Results::Success;
			m_MeshMode = MeshMode::BIT16;
			m_ShapeCount = p_IndexCount / 3;
			// m_VertexCountPerShape = p_VertexCountPerShape;

			// Create a vertex array object and make it active
			{
				constexpr GLsizei arrayCount = 1;
				glGenVertexArrays(arrayCount, &m_VertexArrayId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindVertexArray(m_VertexArrayId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new vertex array: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Create a vertex buffer object and make it active
			{
				constexpr GLsizei bufferCount = 1;
				glGenBuffers(bufferCount, &m_VertexBufferId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Create an index buffer object and make it active
			{
				constexpr GLsizei bufferCount = 1;
				glGenBuffers(bufferCount, &m_IndexBufferId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new index buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused index buffer ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Assign the data to the buffer
			{
				auto vertexBufferCount = p_VertexCount;
				eae6320::Graphics::VertexFormats::sVertex_mesh* vertexData = p_VertexData;

				auto bufferSize = sizeof(vertexData[0]) * vertexBufferCount;
				EAE6320_ASSERT((long)bufferSize <= std::numeric_limits<GLsizeiptr>::max());
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(vertexData),
					// In our class we won't ever read from the buffer
					GL_STATIC_DRAW);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			{
				const auto vertexCount = p_IndexCount;
				uint16_t* indexData = p_IndexData;
				// transform to right-hand
				/*
				for (unsigned int i = 0; i < p_ShapeCount; i++) {
					auto cur_group = i * 3;
					std::reverse(indexData + cur_group, indexData + cur_group + 3);
				}*/
				auto bufferSize = sizeof(indexData[0]) * vertexCount;
				EAE6320_ASSERT((long)bufferSize <= std::numeric_limits<GLsizeiptr>::max());
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(indexData),
					// In our class we won't ever read from the buffer
					GL_STATIC_DRAW);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to allocate the index buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Initialize vertex format
			{
				// The "stride" defines how large a single vertex is in the stream of data
				// (or, said another way, how far apart each position element is)
				constexpr auto stride = static_cast<GLsizei>(sizeof(eae6320::Graphics::VertexFormats::sVertex_mesh));

				// Position (0)
				// 3 floats == 12 bytes
				// Offset = 0
				{
					constexpr GLuint vertexElementLocation = 0;
					constexpr GLint elementCount = 3;
					constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
					glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
						reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sVertex_mesh, x)));
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glEnableVertexAttribArray(vertexElementLocation);
						const GLenum errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
								vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}

				// COLOR (0)
				// 4 uint8_t == 4 bytes
				// Offset = 12
				{
					constexpr GLuint vertexElementLocation = 1;
					constexpr GLint elementCount = 4;
					constexpr GLboolean Normalized = GL_TRUE;	// The given floats should be used as-is
					glVertexAttribPointer(vertexElementLocation, elementCount, GL_UNSIGNED_BYTE, Normalized, stride,
						reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sVertex_mesh, r)));
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glEnableVertexAttribArray(vertexElementLocation);
						const GLenum errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
								vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
			}

			return result;
		}

		cResult cMesh::InitializeGeometry32(uint32_t p_IndexCount, uint32_t p_VertexCount,
			eae6320::Graphics::VertexFormats::sVertex_mesh p_VertexData[], uint32_t p_IndexData[]) {
			auto result = eae6320::Results::Success;
			m_MeshMode = MeshMode::BIT32;
			m_ShapeCount = p_IndexCount / 3;
			// m_VertexCountPerShape = p_VertexCountPerShape;

			// Create a vertex array object and make it active
			{
				constexpr GLsizei arrayCount = 1;
				glGenVertexArrays(arrayCount, &m_VertexArrayId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindVertexArray(m_VertexArrayId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new vertex array: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused vertex array ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Create a vertex buffer object and make it active
			{
				constexpr GLsizei bufferCount = 1;
				glGenBuffers(bufferCount, &m_VertexBufferId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new vertex buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused vertex buffer ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Create an index buffer object and make it active
			{
				constexpr GLsizei bufferCount = 1;
				glGenBuffers(bufferCount, &m_IndexBufferId);
				const auto errorCode = glGetError();
				if (errorCode == GL_NO_ERROR)
				{
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferId);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to bind a new index buffer: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
				else
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to get an unused index buffer ID: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Assign the data to the buffer
			{
				auto vertexBufferCount = p_VertexCount;
				eae6320::Graphics::VertexFormats::sVertex_mesh* vertexData = p_VertexData;

				auto bufferSize = sizeof(vertexData[0]) * vertexBufferCount;
				EAE6320_ASSERT((long)bufferSize <= std::numeric_limits<GLsizeiptr>::max());
				glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(vertexData),
					// In our class we won't ever read from the buffer
					GL_STATIC_DRAW);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to allocate the vertex buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			{
				const auto vertexCount = p_IndexCount;
				uint32_t* indexData = p_IndexData;
				// transform to right-hand
				/*
				for (unsigned int i = 0; i < p_ShapeCount; i++) {
					auto cur_group = i * 3;
					std::reverse(indexData + cur_group, indexData + cur_group + 3);
				}*/
				auto bufferSize = sizeof(indexData[0]) * vertexCount;
				EAE6320_ASSERT((long)bufferSize <= std::numeric_limits<GLsizeiptr>::max());
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(bufferSize), reinterpret_cast<GLvoid*>(indexData),
					// In our class we won't ever read from the buffer
					GL_STATIC_DRAW);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					eae6320::Logging::OutputError("OpenGL failed to allocate the index buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
					return result;
				}
			}
			// Initialize vertex format
			{
				// The "stride" defines how large a single vertex is in the stream of data
				// (or, said another way, how far apart each position element is)
				constexpr auto stride = static_cast<GLsizei>(sizeof(eae6320::Graphics::VertexFormats::sVertex_mesh));

				// Position (0)
				// 3 floats == 12 bytes
				// Offset = 0
				{
					constexpr GLuint vertexElementLocation = 0;
					constexpr GLint elementCount = 3;
					constexpr GLboolean notNormalized = GL_FALSE;	// The given floats should be used as-is
					glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, notNormalized, stride,
						reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sVertex_mesh, x)));
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glEnableVertexAttribArray(vertexElementLocation);
						const GLenum errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
								vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}

				// COLOR (0)
				// 4 uint8_t == 4 bytes
				// Offset = 12
				{
					constexpr GLuint vertexElementLocation = 1;
					constexpr GLint elementCount = 4;
					constexpr GLboolean Normalized = GL_TRUE;	// The given floats should be used as-is
					glVertexAttribPointer(vertexElementLocation, elementCount, GL_UNSIGNED_BYTE, Normalized, stride,
						reinterpret_cast<GLvoid*>(offsetof(eae6320::Graphics::VertexFormats::sVertex_mesh, r)));
					const auto errorCode = glGetError();
					if (errorCode == GL_NO_ERROR)
					{
						glEnableVertexAttribArray(vertexElementLocation);
						const GLenum errorCode = glGetError();
						if (errorCode != GL_NO_ERROR)
						{
							result = eae6320::Results::Failure;
							EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							eae6320::Logging::OutputError("OpenGL failed to enable the POSITION vertex attribute at location %u: %s",
								vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
							return result;
						}
					}
					else
					{
						result = eae6320::Results::Failure;
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						eae6320::Logging::OutputError("OpenGL failed to set the POSITION vertex attribute at location %u: %s",
							vertexElementLocation, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						return result;
					}
				}
			}

			return result;
		}

		cResult cMesh::CleanUp() {
			auto result = Results::Success;
			if (m_VertexArrayId != 0)
			{
				// Make sure that the vertex array isn't bound
				{
					// Unbind the vertex array
					glBindVertexArray(0);
					const auto errorCode = glGetError();
					if (errorCode != GL_NO_ERROR)
					{
						if (result)
						{
							result = Results::Failure;
						}
						EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
						Logging::OutputError("OpenGL failed to unbind all vertex arrays before cleaning up geometry: %s",
							reinterpret_cast<const char*>(gluErrorString(errorCode)));
					}
				}
				constexpr GLsizei arrayCount = 1;
				glDeleteVertexArrays(arrayCount, &m_VertexArrayId);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					Logging::OutputError("OpenGL failed to delete the vertex array: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				m_VertexArrayId = 0;
			}
			if (m_VertexBufferId != 0)
			{
				constexpr GLsizei bufferCount = 1;
				glDeleteBuffers(bufferCount, &m_VertexBufferId);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					Logging::OutputError("OpenGL failed to delete the vertex buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				m_VertexBufferId = 0;
			}
			if (m_IndexBufferId != 0)
			{
				constexpr GLsizei bufferCount = 1;
				glDeleteBuffers(bufferCount, &m_IndexBufferId);
				const auto errorCode = glGetError();
				if (errorCode != GL_NO_ERROR)
				{
					if (result)
					{
						result = Results::Failure;
					}
					EAE6320_ASSERTF(false, reinterpret_cast<const char*>(gluErrorString(errorCode)));
					Logging::OutputError("OpenGL failed to delete the index buffer: %s",
						reinterpret_cast<const char*>(gluErrorString(errorCode)));
				}
				m_IndexBufferId = 0;
			}

			return result;
		}

		void cMesh::Draw() {
			// Draw the geometry
				// Bind a specific vertex buffer to the device as a data source
			{
				EAE6320_ASSERT(m_VertexArrayId != 0);
				glBindVertexArray(m_VertexArrayId);
				EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
			}
			// Render triangles from the currently-bound vertex buffer
			{
				// The mode defines how to interpret multiple vertices as a single "primitive";
				// a triangle list is defined
				// (meaning that every primitive is a triangle and will be defined by three vertices)
				constexpr GLenum mode = GL_TRIANGLES;
				// It's possible to start rendering primitives in the middle of the stream
				const GLvoid* const offset = 0;
				switch (m_MeshMode) {
				case MeshMode::BIT32:
					glDrawElements(mode, static_cast<GLsizei>(m_ShapeCount * 3), GL_UNSIGNED_INT, offset);
					break;
				case MeshMode::BIT16:
				default:
					glDrawElements(mode, static_cast<GLsizei>(m_ShapeCount * 3), GL_UNSIGNED_SHORT, offset);
				}
				
				EAE6320_ASSERT(glGetError() == GL_NO_ERROR);

			}
		}
	}
}