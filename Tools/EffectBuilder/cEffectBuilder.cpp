// Includes
//=========

#include "cEffectBuilder.h"

#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <External/Lua/Includes.h>

#include <fstream>
#include <string>
#include <algorithm>

namespace
{

	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, uint8_t& o_RenderStateBits, std::string& o_VertexPath, std::string& o_FragmentPath);

	eae6320::cResult LoadTableValues_renderState_values(const char* const i_path, lua_State& io_luaState, uint8_t& o_RenderStateBits);

	eae6320::cResult LoadAsset(const char* const i_path, uint8_t& o_RenderStateBits, std::string& o_VertexPath, std::string& o_FragmentPath);
}

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cEffectBuilder::Build(const std::vector<std::string>& i_arguments)
{
	// Load File
	eae6320::cResult result = eae6320::Results::Success;

	uint8_t render_state_bits = 0;
	std::string vertex_path;
	std::string fragment_path;

	if (!(result = LoadAsset(m_path_source, render_state_bits, vertex_path, fragment_path)))
	{
		return result;
	}

	std::string out_vertex_path;
	std::string out_fragment_path;
	std::string err_message;
	if (!(result = ConvertSourceRelativePathToBuiltRelativePath(vertex_path.c_str(), "shaders", out_vertex_path, &err_message))) {
		eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, err_message.c_str());
		return result;
	}

	if (!(result = ConvertSourceRelativePathToBuiltRelativePath(fragment_path.c_str(), "shaders", out_fragment_path, &err_message))) {
		eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, err_message.c_str());
		return result;
	}

	// std::string error_message;
	// result = eae6320::Platform::CopyFile(m_path_source, m_path_target, false, true, &error_message);
	std::ofstream outfile(m_path_target, std::ofstream::binary);

	outfile.write((char*)(&render_state_bits), sizeof(uint8_t));

	out_vertex_path = "data/" + out_vertex_path;
	outfile.write(out_vertex_path.c_str(), out_vertex_path.size());

	outfile.write("\0", 1);

	out_fragment_path = "data/" + out_fragment_path;
	outfile.write(out_fragment_path.c_str(), out_fragment_path.size());

	outfile.write("\0", 1);

	outfile.close();

	return result;
}

//---------------------Helper Functions------------------------------

namespace
{
	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, uint8_t& o_RenderStateBits, std::string& o_VertexPath, std::string& o_FragmentPath)
	{
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1:
		
		{
			constexpr auto* const key = "render_state";
			lua_pushstring(&io_luaState, key);
			lua_gettable(&io_luaState, -2);

			eae6320::cScopeGuard scopeGuard_popVertices([&io_luaState]
				{
					lua_pop(&io_luaState, 1);
				});

			if (lua_istable(&io_luaState, -1))
			{
				if (!(result = LoadTableValues_renderState_values(i_path, io_luaState, o_RenderStateBits)))
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
		}
		
		{
			constexpr auto* const key = "vertex_shader";
			lua_pushstring(&io_luaState, key);
			{
				constexpr int currentIndexOfTable = -2;
				lua_gettable(&io_luaState, currentIndexOfTable);
			}

			eae6320::cScopeGuard scopeGuard_popName( [&io_luaState]{lua_pop(&io_luaState, 1);} );
			if (lua_isnil(&io_luaState, -1))
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "No value for \"" + std::string(key) + "\" was found in the asset table";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				
				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TSTRING)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a string "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const auto* const value = lua_tostring(&io_luaState, -1);
				o_VertexPath = std::string(value);
			}
		}

		{
			constexpr auto* const key = "fragment_shader";
			lua_pushstring(&io_luaState, key);
			{
				constexpr int currentIndexOfTable = -2;
				lua_gettable(&io_luaState, currentIndexOfTable);
			}

			eae6320::cScopeGuard scopeGuard_popName([&io_luaState] {lua_pop(&io_luaState, 1); });
			if (lua_isnil(&io_luaState, -1))
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "No value for \"" + std::string(key) + "\" was found in the asset table";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());

				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TSTRING)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a string "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const auto* const value = lua_tostring(&io_luaState, -1);
				o_FragmentPath = std::string(value);
			}
		}

		return result;
	}

	eae6320::cResult ReadBool(const char* const i_path, lua_State& io_luaState, const char* p_Key, bool& o_Result) {
		auto result = eae6320::Results::Success;
		{
			lua_pushstring(&io_luaState, p_Key);
			{
				constexpr int currentIndexOfTable = -2;
				lua_gettable(&io_luaState, currentIndexOfTable);
			}

			eae6320::cScopeGuard scopeGuard_popName([&io_luaState] {lua_pop(&io_luaState, 1); });
			
			if (lua_isnil(&io_luaState, -1))
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "No value for \"" + std::string(p_Key) + "\" was found in the asset table";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());

				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TBOOLEAN)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(p_Key) + "\" must be a boolean "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			o_Result = lua_toboolean(&io_luaState, -1);
		}
		return result;
	}

	eae6320::cResult LoadTableValues_renderState_values(const char* const i_path, lua_State& io_luaState, uint8_t& o_RenderStateBits)
	{
		auto result = eae6320::Results::Success;

		bool AlphaTransparency;
		bool DepthTesting;
		bool DepthWriting; 
		bool DrawBothTriangleSides;
		
		if (!(result  = ReadBool(i_path, io_luaState, "alpha_transparency", AlphaTransparency))) {
			return result;
		}

		if (!(result = ReadBool(i_path, io_luaState, "depth_testing", DepthTesting))) {
			return result;
		}
		
		if (!(result = ReadBool(i_path, io_luaState, "depth_writing", DepthWriting))) {
			return result;
		}

		if (!(result = ReadBool(i_path, io_luaState, "draw_both_triangle_sides", DrawBothTriangleSides))) {
			return result;
		}

		o_RenderStateBits = 0;

		uint8_t mask = 1;
		
		if (AlphaTransparency) {
			o_RenderStateBits |= mask << 0;
		}

		if (DepthTesting) {
			o_RenderStateBits |= mask << 1;
		}

		if (DepthWriting) {
			o_RenderStateBits |= mask << 2;
		}

		if (DrawBothTriangleSides) {
			o_RenderStateBits |= mask << 3;
		}

		return result;
	}

	eae6320::cResult LoadAsset(const char* const i_path, uint8_t& o_RenderStateBits, std::string& o_VertexPath, std::string& o_FragmentPath)
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
					std::string err_message = "Asset files must return a single table (instead of " +
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


		result = LoadTableValues(i_path, *luaState, o_RenderStateBits, o_VertexPath, o_FragmentPath);


		return result;
	}
}
