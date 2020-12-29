// Includes
//=========

#include "cMeshBuilder.h"

#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Graphics/VertexFormats.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Time/Time.h>
#include <External/Lua/Includes.h>

#include <fstream>
#include <string>
#include <algorithm>

namespace
{
	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData, void*& o_IndexData);

	eae6320::cResult LoadTableValues_vertices(const char* const i_path, lua_State& io_luaState, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData);
	eae6320::cResult LoadTableValues_vertices_values(const char* const i_path, lua_State& io_luaState, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData);

	eae6320::cResult LoadTableValues_vertex(const char* const i_path, lua_State& io_luaState,
		eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex);
	eae6320::cResult LoadTableValues_position_values(lua_State& io_luaState, 
		eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex);
	eae6320::cResult LoadTableValues_color_values(lua_State& io_luaState, 
		eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex);

	eae6320::cResult LoadTableValues_indices(const char* const i_path, lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, void*& o_IndexData);
	eae6320::cResult LoadTableValues_indices_values(lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, void*& o_IndexData);

	eae6320::cResult LoadAsset(const char* const i_path, uint8_t& o_Mode, uint32_t& o_IndexCount, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData, void*& o_IndexData);
}

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cMeshBuilder::Build(const std::vector<std::string>& i_arguments)
{
	// Load File
	eae6320::cResult result = eae6320::Results::Success;

	// 0 => 16
	// 1 => 32
	uint8_t mode = 0;
	uint32_t index_count = 0;
	uint32_t vertex_count = 0;
	eae6320::Graphics::VertexFormats::sVertex_mesh* vertex_data = nullptr;
	void* index_data = nullptr;

	if (!(result = LoadAsset(m_path_source, mode, index_count, vertex_count, vertex_data, index_data)))
	{
		if (vertex_data) delete[] vertex_data;
		if (index_data) delete[] index_data;
		return result;
	}

	// std::string error_message;
	// result = eae6320::Platform::CopyFile(m_path_source, m_path_target, false, true, &error_message);

	std::ofstream outfile(m_path_target, std::ofstream::binary);
	
	outfile.write((char*)(&mode), sizeof(uint8_t));
	
	switch (mode) {
	case 1:
		// Write index count
		outfile.write((char*)(&index_count), sizeof(uint32_t));
		// Write vertex count
		outfile.write((char*)(&vertex_count), sizeof(uint32_t));
		// Write index array
		outfile.write((char*)(index_data), ((long long)sizeof(uint32_t) * (long long)index_count));
		break;
	case 0:
	default:
		uint16_t parsed_index_count = (uint16_t)index_count;
		uint16_t parsed_vertex_count = (uint16_t)vertex_count;
		// Write index count
		outfile.write((char*)(&parsed_index_count), sizeof(uint16_t));
		// Write vertex count
		outfile.write((char*)(&parsed_vertex_count), sizeof(uint16_t));
		// Write index array
		outfile.write((char*)(index_data), ((long long)sizeof(uint16_t) * (long long)parsed_index_count));
	}

	// Write vertex array
	size_t size = (sizeof(eae6320::Graphics::VertexFormats::sVertex_mesh) * vertex_count);
	outfile.write((char*)vertex_data, size);

	outfile.close();

	if (vertex_data) delete[] vertex_data;
	if (index_data) delete[] index_data;

	return result;
}

//---------------------Helper Functions------------------------------

namespace
{
	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData, void*& o_IndexData)
	{
		auto result = eae6320::Results::Success;

		if (!(result = LoadTableValues_vertices(i_path, io_luaState, o_VertexCount, o_VertexData)))
		{
			return result;
		}
		if (!(result = LoadTableValues_indices(i_path, io_luaState, o_Mode, o_IndexCount, o_IndexData)))
		{
			return result;
		}

		return result;
	}

	// Vertices
	eae6320::cResult LoadTableValues_vertices(const char* const i_path, lua_State& io_luaState, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData)
	{
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		constexpr auto* const key = "vertices";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);

		eae6320::cScopeGuard scopeGuard_popVertices([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});

		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValues_vertices_values(i_path, io_luaState, o_VertexCount, o_VertexData)))
			{
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			std::string err_message = "The value at \"" + std::string(key) + "\" must be a table "
				"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
			eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
			return result;
		}

		return result;
	}

	eae6320::cResult LoadTableValues_vertices_values(const char* const i_path, lua_State& io_luaState, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData)
	{
		auto result = eae6320::Results::Success;

		const auto vertexCount = (uint32_t)luaL_len(&io_luaState, -1);
		(o_VertexCount) = vertexCount;
		o_VertexData = new eae6320::Graphics::VertexFormats::sVertex_mesh[vertexCount];
		for (uint32_t i = 1; i <= vertexCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			// Now each vertex table is at -1
			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_vertex(i_path, io_luaState, o_VertexData + i - 1)))
				{
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "Wrong vertex format.");
				return result;
			}
		}

		return result;
	}

	// A single vertex
	eae6320::cResult LoadTableValues_vertex(const char* const i_path, lua_State& io_luaState, eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex) {
		auto result = eae6320::Results::Success;

		{
			constexpr auto* const key1 = "position";
			lua_pushstring(&io_luaState, key1);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popParameters([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_position_values(io_luaState, o_Vertex)))
				{
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value at \"" + std::string(key1) + "\" must be a table "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}
		}

		{
			constexpr auto* const key2 = "color";
			lua_pushstring(&io_luaState, key2);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popParameters2([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_color_values(io_luaState, o_Vertex)))
				{
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value at \"" + std::string(key2) + "\" must be a table "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}
		}
		return result;
	}

	eae6320::cResult LoadTableValues_position_values(lua_State& io_luaState, eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex) {
		auto result = eae6320::Results::Success;

		const auto valueCount = luaL_len(&io_luaState, -1);
		float vertex[3] = { 0.0f, 0.0f, 0.0f };
		for (int i = 1; i <= valueCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			const char* curValue = lua_tostring(&io_luaState, -1);
			vertex[i - 1] = (float)atof(curValue);
		}
		o_Vertex->x = vertex[0];
		o_Vertex->y = vertex[1];
		o_Vertex->z = vertex[2];

		return result;
	}

	eae6320::cResult LoadTableValues_color_values(lua_State& io_luaState, eae6320::Graphics::VertexFormats::sVertex_mesh* o_Vertex) {
		auto result = eae6320::Results::Success;

		const auto valueCount = luaL_len(&io_luaState, -1);
		float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		for (int i = 1; i <= valueCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			const char* curValue = lua_tostring(&io_luaState, -1);
			color[i - 1] = (float)atof(curValue);
		}
		o_Vertex->r = (uint8_t)roundf(color[0] * 255);
		o_Vertex->g = (uint8_t)roundf(color[1] * 255);
		o_Vertex->b = (uint8_t)roundf(color[2] * 255);
		o_Vertex->a = (uint8_t)roundf(color[3] * 255);

		return result;
	}

	//Indices
	eae6320::cResult LoadTableValues_indices(const char* const i_path, lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, void*& o_IndexData)
	{
		auto result = eae6320::Results::Success;

		constexpr auto* const key = "indices";
		lua_pushstring(&io_luaState, key);
		lua_gettable(&io_luaState, -2);
		eae6320::cScopeGuard scopeGuard_popParameters([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
		if (lua_istable(&io_luaState, -1))
		{
			if (!(result = LoadTableValues_indices_values(io_luaState, o_Mode, o_IndexCount, o_IndexData)))
			{
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			std::string err_message = "The value at \"" + std::string(key) + "\" must be a table "
				"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
			eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
			return result;
		}

		return result;
	}

	eae6320::cResult LoadTableValues_indices_values(lua_State& io_luaState, uint8_t& o_Mode, uint32_t& o_IndexCount, void*& o_IndexData)
	{
		// Right now the indices table is at -1.
		// Every time the while() statement is executed it will be at -2
		// and the next key will be at -1.
		// Inside the block the table will be at -3,
		// the current key will be at -2,
		// and the value will be at -1.

		auto result = eae6320::Results::Success;

		const auto indexCount = (int)luaL_len(&io_luaState, -1);
		(o_IndexCount) = (uint32_t)(indexCount);
		uint16_t max16 = 0xff;
		uint32_t* index_data = new uint32_t[indexCount];
		for (int i = 1; i <= indexCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});
			const char* curValue = lua_tostring(&io_luaState, -1);
			index_data[i - 1] = (uint32_t)atoi(curValue);
		}
		uint32_t max_index = *(std::max_element(index_data, index_data + indexCount));
#if defined( EAE6320_PLATFORM_D3D )
		for (int i = 0; i < indexCount / 3; i++) {
			uint32_t temp = index_data[i * 3];
			index_data[i * 3] = index_data[i * 3 + 2];
			index_data[i * 3 + 2] = temp;
		}
#endif
		if (max_index > max16) {
			o_Mode = 1;
			o_IndexData = index_data;
		}
		else {
			o_Mode = 0;
			uint16_t* shortened = new uint16_t[indexCount];
			for (int i = 0; i < indexCount; i++) {
				shortened[i] = (uint16_t)index_data[i];
			}
			delete[] index_data;
			o_IndexData = shortened;
		}

		return result;
	}

	eae6320::cResult LoadAsset(const char* const i_path, uint8_t& o_Mode, uint32_t& o_IndexCount, uint32_t& o_VertexCount,
		eae6320::Graphics::VertexFormats::sVertex_mesh*& o_VertexData, void*& o_IndexData)
	{
		auto result = eae6320::Results::Success;

		// Create a new Lua state
		lua_State* luaState = nullptr;
		eae6320::cScopeGuard scopeGuard_onExit([&luaState]
			{
				if (luaState)
				{
					lua_close(luaState);
					luaState = nullptr;
				}
			});
		{
			luaState = luaL_newstate();
			if (!luaState)
			{
				result = eae6320::Results::OutOfMemory;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "Failed to create a new Lua state");
				return result;
			}
		}

		// Load the asset file as a "chunk",
		// meaning there will be a callable function at the top of the stack
		const auto stackTopBeforeLoad = lua_gettop(luaState);
		{

			const auto luaResult = luaL_loadfile(luaState, i_path);

			if (luaResult != LUA_OK)
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, lua_tostring(luaState, -1));
				
				// Pop the error message
				lua_pop(luaState, 1);
				return result;
			}
		}
		// Execute the "chunk", which should load the asset
		// into a table at the top of the stack
		{
			constexpr int argumentCount = 0;
			constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
			constexpr int noMessageHandler = 0;
			const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
			if (luaResult == LUA_OK)
			{
				// A well-behaved asset file will only return a single value
				const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
				if (returnedValueCount == 1)
				{
					// A correct asset file _must_ return a table
					if (!lua_istable(luaState, -1))
					{
						result = eae6320::Results::InvalidFile;
						std::string err_message = "Asset files must return a table (instead of a " +
							std::string(luaL_typename(luaState, -1)) + ")";
						eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
						// Pop the returned non-table value
						lua_pop(luaState, 1);
						return result;
					}
				}
				else
				{
					result = eae6320::Results::InvalidFile;
					std::string err_message = "Asset files must return a single table (instead of "+
						returnedValueCount;
					err_message += " values)";
					eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
					// Pop every value that was returned
					lua_pop(luaState, returnedValueCount);
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, lua_tostring(luaState, -1));
				// Pop the error message
				lua_pop(luaState, 1);
				return result;
			}
		}

		// If this code is reached the asset file was loaded successfully,
		// and its table is now at index -1
		eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
			{
				lua_pop(luaState, 1);
			});


		result = LoadTableValues(i_path, *luaState, o_Mode, o_IndexCount, o_VertexCount, o_VertexData, o_IndexData);


		return result;
	}
}
