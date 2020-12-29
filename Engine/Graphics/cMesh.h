#pragma once

#include "VertexFormats.h"
#ifdef EAE6320_PLATFORM_D3D
struct ID3D11Buffer;
#include "cVertexFormat.h"
#elif EAE6320_PLATFORM_GL
#include "OpenGL/Includes.h"
#endif

#include <Engine/Assets/ReferenceCountedAssets.h>

namespace eae6320
{
	namespace Graphics
	{
		enum MeshMode {
			BIT16, BIT32
		};

		class cMesh {

		public:

			static cResult Load(const char p_MeshPath[], cMesh*& o_Mesh);

			void Draw();

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cMesh);

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

		private:
			cMesh() = default;
			~cMesh();

			cResult InitializeGeometry16(uint16_t p_IndexCount, uint16_t p_VertexCount,
				eae6320::Graphics::VertexFormats::sVertex_mesh p_VertexData[], uint16_t p_IndexData[]);
			cResult InitializeGeometry32(uint32_t p_IndexCount, uint32_t p_VertexCount,
				eae6320::Graphics::VertexFormats::sVertex_mesh p_VertexData[], uint32_t p_IndexData[]);
			
			cResult CleanUp();

#if defined( EAE6320_PLATFORM_D3D )
			cVertexFormat* m_VertexFormat = nullptr;
			ID3D11Buffer* m_VertexBuffer = nullptr;
			ID3D11Buffer* m_IndexBuffer = nullptr;
#elif defined( EAE6320_PLATFORM_GL )
			GLuint m_VertexBufferId = 0;
			GLuint m_VertexArrayId = 0;
			GLuint m_IndexBufferId = 0;
#endif

			int m_ShapeCount = 2;
			// uint16_t m_VertexCountPerShape = 3;
			MeshMode m_MeshMode = MeshMode::BIT16;

			EAE6320_ASSETS_DECLAREREFERENCECOUNT();
		};
	}
}