#include "cMesh.h"

#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Time/Time.h>

namespace eae6320
{
	namespace Graphics
	{

		cResult cMesh::Load(const char p_MeshPath[], cMesh*& o_Mesh) {
			auto result = eae6320::Results::Success;

			EAE6320_ASSERT(o_Mesh == nullptr);

			cMesh* new_mesh = nullptr;
			cScopeGuard scopeGuard([&o_Mesh, &result, &new_mesh]
				{
					if (result)
					{
						EAE6320_ASSERT(new_mesh != nullptr);
						o_Mesh = new_mesh;
					}
					else
					{
						if (new_mesh)
						{
							new_mesh->DecrementReferenceCount();
							new_mesh = nullptr;
						}
						o_Mesh = nullptr;
					}
				});

			// Allocate a new vertex format
			{
				new_mesh = new (std::nothrow) cMesh();
				if (!new_mesh)
				{
					result = Results::OutOfMemory;
					EAE6320_ASSERTF(false, "Couldn't allocate memory for a mesh");
					Logging::OutputError("Failed to allocate memory for a mesh");
					return result;
				}
			}

			// Load binary file
			Platform::sDataFromFile data_from_file;
			std::string err_message;

			result = Platform::LoadBinaryFile(p_MeshPath, data_from_file, &err_message);


			if (!result) {
				Logging::OutputError(err_message.c_str());
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(data_from_file.data);
			const auto finalOffset = currentOffset + data_from_file.size;

			// Load mode
			uint8_t mode = *reinterpret_cast<uint8_t*>(currentOffset);
			currentOffset += sizeof(mode);

			switch (mode) {
			case 1:
			{
				// Load index count
				const auto index_count = *reinterpret_cast<uint32_t*>(currentOffset);
				currentOffset += sizeof(index_count);

				// Load vertex count
				const auto vertex_count = *reinterpret_cast<uint32_t*>(currentOffset);
				currentOffset += sizeof(vertex_count);

				// Load indices
				const auto index_data = reinterpret_cast<uint32_t*>(currentOffset);
				currentOffset += sizeof(uint32_t) * index_count;

				// Load vertices
				const auto vertex_data = reinterpret_cast<eae6320::Graphics::VertexFormats::sVertex_mesh*>(currentOffset);
				currentOffset += sizeof(eae6320::Graphics::VertexFormats::sVertex_mesh) * vertex_count;
				eae6320::Graphics::VertexFormats::sVertex_mesh cur_vertex;
				for (uint32_t i = 0; i < vertex_count; i++) {
					cur_vertex = vertex_data[i];
				}

				EAE6320_ASSERT(currentOffset == finalOffset);

				// Initialize the platform-specific vertex format
				if (!(result = new_mesh->InitializeGeometry32(index_count, vertex_count, vertex_data, index_data)))
				{
					EAE6320_ASSERTF(false, "Initialization of new mesh failed.");
					return result;
				}
			}
				break;
			case 0:
			default:
			{
				// Load index count
				const auto index_count = *reinterpret_cast<uint16_t*>(currentOffset);
				currentOffset += sizeof(index_count);

				// Load vertex count
				const auto vertex_count = *reinterpret_cast<uint16_t*>(currentOffset);
				currentOffset += sizeof(vertex_count);

				// Load indices
				const auto index_data = reinterpret_cast<uint16_t*>(currentOffset);
				currentOffset += sizeof(uint16_t) * index_count;
				// Load vertices
				const auto vertex_data = reinterpret_cast<eae6320::Graphics::VertexFormats::sVertex_mesh*>(currentOffset);
				currentOffset += sizeof(eae6320::Graphics::VertexFormats::sVertex_mesh) * vertex_count;

				EAE6320_ASSERT(currentOffset == finalOffset);

				// Initialize the platform-specific vertex format
				if (!(result = new_mesh->InitializeGeometry16(index_count, vertex_count, vertex_data, index_data)))
				{
					EAE6320_ASSERTF(false, "Initialization of new mesh failed.");
					return result;
				}
			}
			}


			return result;
		}

		cMesh::~cMesh() {
			EAE6320_ASSERT(m_referenceCount == 0);
			const auto result = CleanUp();
			EAE6320_ASSERT(result);
		}
	}
}