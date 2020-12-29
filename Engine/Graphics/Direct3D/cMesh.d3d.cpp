#include "../cMesh.h"

#include "Includes.h"
#include "../sContext.h"
#include "../ConstantBufferFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>

namespace eae6320 {
	namespace Graphics {
		cResult cMesh::InitializeGeometry16(uint16_t p_IndexCount, uint16_t p_VertexCount,
			eae6320::Graphics::VertexFormats::sVertex_mesh p_VertexData[], uint16_t p_IndexData[]) {
			auto result = eae6320::Results::Success;
			m_MeshMode = MeshMode::BIT16;
			m_ShapeCount = p_IndexCount / 3;
			// m_VertexCountPerShape = p_VertexCountPerShape;

			auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
			EAE6320_ASSERT(direct3dDevice);

			// Vertex Format
			{
				if (!(result = eae6320::Graphics::cVertexFormat::Load(eae6320::Graphics::eVertexType::Mesh, m_VertexFormat,
					"data/Shaders/Vertex/vertexInputLayout_mesh.bshader")))
				{
					EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
					return result;
				}
			}
			// Vertex Buffer
			{
				auto vertexBufferCount = p_VertexCount;

				eae6320::Graphics::VertexFormats::sVertex_mesh* vertexData = p_VertexData;
				auto bufferSize = sizeof(vertexData[0]) * vertexBufferCount;
				EAE6320_ASSERT(bufferSize <= std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max());
				if (bufferSize > std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max()) {
					return Results::Failure;
				}
				auto bufferDescription = D3D11_BUFFER_DESC();
				bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
				bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription.MiscFlags = 0;
				bufferDescription.StructureByteStride = 0;	// Not used
				
				uint16_t* indexData = p_IndexData;
				
				auto bufferSize_index = sizeof(indexData[0]) * p_IndexCount;
				EAE6320_ASSERT(bufferSize_index <= std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max());
				if (bufferSize_index > std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max()) {
					return Results::Failure;
				}
				auto bufferDescription_index = D3D11_BUFFER_DESC();
				bufferDescription_index.ByteWidth = static_cast<unsigned int>(bufferSize_index);
				bufferDescription_index.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDescription_index.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription_index.MiscFlags = 0;
				bufferDescription_index.StructureByteStride = 0;	// Not used

				const auto initialData = [vertexData]
				{
					D3D11_SUBRESOURCE_DATA initialData{};

					initialData.pSysMem = vertexData;
					// (The other data members are ignored for non-texture buffers)

					return initialData;
				}();

				const auto initialData_index = [indexData]
				{
					D3D11_SUBRESOURCE_DATA initialData{};

					initialData.pSysMem = indexData;
					// (The other data members are ignored for non-texture buffers)

					return initialData;
				}();

				const auto result_create = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_VertexBuffer);
				if (FAILED(result_create))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", result_create);
					eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", result_create);
					return result;
				}

				const auto result_create_index = direct3dDevice->CreateBuffer(&bufferDescription_index, &initialData_index, &m_IndexBuffer);
				if (FAILED(result_create_index))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "3D object index buffer creation failed (HRESULT %#010x)", result_create_index);
					eae6320::Logging::OutputError("Direct3D failed to create a 3D object index buffer (HRESULT %#010x)", result_create_index);
					return result;
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

			auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
			EAE6320_ASSERT(direct3dDevice);

			// Vertex Format
			{
				if (!(result = eae6320::Graphics::cVertexFormat::Load(eae6320::Graphics::eVertexType::Mesh, m_VertexFormat,
					"data/Shaders/Vertex/vertexInputLayout_mesh.bshader")))
				{
					EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
					return result;
				}
			}
			// Vertex Buffer
			{
				auto vertexBufferCount = p_VertexCount;

				eae6320::Graphics::VertexFormats::sVertex_mesh* vertexData = p_VertexData;
				auto bufferSize = sizeof(vertexData[0]) * vertexBufferCount;
				EAE6320_ASSERT(bufferSize <= std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max());
				if (bufferSize > std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max()) {
					return Results::Failure;
				}
				auto bufferDescription = D3D11_BUFFER_DESC();
				bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
				bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription.MiscFlags = 0;
				bufferDescription.StructureByteStride = 0;	// Not used

				uint32_t* indexData = p_IndexData;
				// transform to left-hand
				for (int i = 0; i < m_ShapeCount; i++) {
					auto cur_group = i * 3;
					std::reverse(indexData + cur_group, indexData + cur_group + 3);
				}
				auto bufferSize_index = sizeof(indexData[0]) * p_IndexCount;
				EAE6320_ASSERT(bufferSize_index <= std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max());
				if (bufferSize_index > std::numeric_limits<decltype(D3D11_BUFFER_DESC::ByteWidth)>::max()) {
					return Results::Failure;
				}
				auto bufferDescription_index = D3D11_BUFFER_DESC();
				bufferDescription_index.ByteWidth = static_cast<unsigned int>(bufferSize_index);
				bufferDescription_index.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
				bufferDescription_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDescription_index.CPUAccessFlags = 0;	// No CPU access is necessary
				bufferDescription_index.MiscFlags = 0;
				bufferDescription_index.StructureByteStride = 0;	// Not used

				const auto initialData = [vertexData]
				{
					D3D11_SUBRESOURCE_DATA initialData{};

					initialData.pSysMem = vertexData;
					// (The other data members are ignored for non-texture buffers)

					return initialData;
				}();

				const auto initialData_index = [indexData]
				{
					D3D11_SUBRESOURCE_DATA initialData{};

					initialData.pSysMem = indexData;
					// (The other data members are ignored for non-texture buffers)

					return initialData;
				}();

				const auto result_create = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_VertexBuffer);
				if (FAILED(result_create))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", result_create);
					eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", result_create);
					return result;
				}

				const auto result_create_index = direct3dDevice->CreateBuffer(&bufferDescription_index, &initialData_index, &m_IndexBuffer);
				if (FAILED(result_create_index))
				{
					result = eae6320::Results::Failure;
					EAE6320_ASSERTF(false, "3D object index buffer creation failed (HRESULT %#010x)", result_create_index);
					eae6320::Logging::OutputError("Direct3D failed to create a 3D object index buffer (HRESULT %#010x)", result_create_index);
					return result;
				}
			}

			return result;
		}

		cResult cMesh::CleanUp() {
			if (m_VertexFormat)
			{
				m_VertexFormat->DecrementReferenceCount();
				m_VertexFormat = nullptr;
			}
			if (m_VertexBuffer)
			{
				m_VertexBuffer->Release();
				m_VertexBuffer = nullptr;
			}
			if (m_IndexBuffer) {
				m_IndexBuffer->Release();
				m_IndexBuffer = nullptr;
			}

			return Results::Success;
		}
		
		void cMesh::Draw() {
			// Draw the geometry

			auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
			// Bind a specific vertex buffer to the device as a data source
			{
				ID3D11Buffer* vertexBuffer = m_VertexBuffer;
				EAE6320_ASSERT(vertexBuffer != nullptr);
				constexpr unsigned int startingSlot = 0;
				constexpr unsigned int vertexBufferCount = 1;
				// The "stride" defines how large a single vertex is in the stream of data
				constexpr unsigned int bufferStride = sizeof(VertexFormats::sVertex_mesh);
				// It's possible to start streaming data in the middle of a vertex buffer
				constexpr unsigned int bufferOffset = 0;

				direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &(vertexBuffer), &bufferStride, &bufferOffset);
			}
			{
				ID3D11Buffer* indexBuffer = m_IndexBuffer;
				EAE6320_ASSERT(indexBuffer);
				DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
				switch (m_MeshMode)
				{
				case MeshMode::BIT32:
					indexFormat = DXGI_FORMAT_R32_UINT;
					break;
				case MeshMode::BIT16:
				default:
					break;
				}
				// The indices start at the beginning of the buffer
				constexpr unsigned int offset = 0;
				direct3dImmediateContext->IASetIndexBuffer(indexBuffer, indexFormat, offset);
			}
			// Specify what kind of data the vertex buffer holds
			{
				// Bind the vertex format (which defines how to interpret a single vertex)
				{
					EAE6320_ASSERT(m_VertexFormat != nullptr);
					m_VertexFormat->Bind();
				}
				// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
				// the vertex buffer was defined as a triangle list
				// (meaning that every primitive is a triangle and will be defined by three vertices)
				direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}

			// Render triangles from the currently-bound vertex buffer
			{
				// It's possible to start rendering primitives in the middle of the stream
				constexpr unsigned int indexOfFirstIndexToUse = 0;
				constexpr unsigned int offsetToAddToEachIndex = 0;
				direct3dImmediateContext->DrawIndexed(static_cast<unsigned int>(m_ShapeCount * 3), indexOfFirstIndexToUse, offsetToAddToEachIndex);
			}
			
		}
	}
}
