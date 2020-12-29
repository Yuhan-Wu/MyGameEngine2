// Includes
//=========

#include "cAudioBuilder.h"

#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <Engine/Audio/sMusicConfig.h>
#include <Engine/Audio/sEffect.h>
#include <External/Lua/Includes.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <comdef.h>

namespace
{
	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, std::wstring& o_MusicPath, eae6320::Audio::sMusicConfig& o_MusicData, eae6320::Audio::sEffect& o_MusicEffect);
	
	eae6320::cResult LoadEffectValues(const char* const i_path, lua_State& io_luaState, eae6320::Audio::sEffect& o_MusicEffect);

	eae6320::cResult LoadAsset(const char* const i_path, std::wstring& o_MusicPath, eae6320::Audio::sMusicConfig& o_MusicData, eae6320::Audio::sEffect& o_MusicEffect);
}

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cAudioBuilder::Build(const std::vector<std::string>& i_arguments)
{
	// Load File
	eae6320::cResult result = eae6320::Results::Success;

	std::wstring music_path;
	eae6320::Audio::sMusicConfig music_data;
	eae6320::Audio::sEffect music_effect;
	if (!(result = LoadAsset(m_path_source, music_path, music_data, music_effect))) {
		return result;
	}
	const wchar_t* char_path = music_path.c_str();
	_bstr_t b(char_path);
	const char* c = b;
	std::string converted_str(c);

	std::string in_directory = std::string(m_path_source);
	std::size_t found = in_directory.find_last_of('/');
	std::string input_str = in_directory.substr(0, found) + "/" + converted_str;
	
	std::string out_directory = std::string(m_path_target);
	found = out_directory.find_last_of('/');
	std::string output_str = out_directory.substr(0, found) + "/" + converted_str;

	std::string err_message;
	// TODO Copy audio files to destination
	if (!(result = Platform::CopyFile(input_str.c_str(), output_str.c_str(), false, true, &err_message))) {
		eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, err_message.c_str());
		return result;
	}

	music_path = L"data/audio/" + music_path;
	std::ofstream outfile(m_path_target, std::ofstream::binary);
	size_t size = sizeof(eae6320::Audio::sMusicConfig);
	std::streamsize length = (long long)wcslen(music_path.c_str()) * sizeof(wchar_t);
	outfile.write((char*)music_path.c_str(), length);
	outfile.write((char*)&music_data, size);
	outfile.write((char*)&music_effect, sizeof(eae6320::Audio::sEffect));
	outfile.close();
	return result;
}

//---------------------Helper Functions------------------------------

namespace
{
	std::wstring stringToWstring(const std::string& t_str)
	{
		const char* char_str = t_str.c_str();
		_bstr_t b(char_str);
		const wchar_t* c = b;

		//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
		return std::wstring(c);
	}

	eae6320::cResult LoadTableValues(const char* const i_path, lua_State& io_luaState, std::wstring& o_MusicPath, eae6320::Audio::sMusicConfig& o_MusicData, eae6320::Audio::sEffect& o_MusicEffect) {
		auto result = eae6320::Results::Success;

		// Right now the asset table is at -1.
		// After the following table operation it will be at -2
		// and the "textures" table will be at -1

		{
			constexpr auto* const key = "audio";
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
				o_MusicPath = stringToWstring(std::string(value));
			}
		}

		{
			constexpr auto* const key = "loop";
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

			if (lua_type(&io_luaState, -1) != LUA_TBOOLEAN)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a boolean "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const bool value = lua_toboolean(&io_luaState, -1);
				o_MusicData.m_IsLoop = value;
			}
		}

		{
			constexpr auto* const key = "autoplay";
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

			if (lua_type(&io_luaState, -1) != LUA_TBOOLEAN)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a boolean "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const bool value = lua_toboolean(&io_luaState, -1);
				o_MusicData.m_Autoplay = value;
			}
		}

		{
			constexpr auto* const key = "volume";
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

			if (lua_type(&io_luaState, -1) != LUA_TNUMBER)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a float "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const auto value = lua_tonumber(&io_luaState, -1);
				o_MusicData.m_Volume = (float)value;
			}
		}

		{
			constexpr auto* const key = "pitch";
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

			if (lua_type(&io_luaState, -1) != LUA_TNUMBER)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a float "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				const auto value = lua_tonumber(&io_luaState, -1);
				o_MusicData.m_pitch = (float)value;
			}
		}

		{
			constexpr auto* const key = "effects";
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

			if (!lua_istable(&io_luaState, -1))
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(key) + "\" must be a table "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			{
				if (!(result = LoadEffectValues(i_path, io_luaState, o_MusicEffect)))
				{
					return result;
				}
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

	eae6320::cResult ReadFloat(const char* const i_path, lua_State& io_luaState, const char* p_Key, float& o_Result) {
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

				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TNUMBER)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(p_Key) + "\" must be a float "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			o_Result = (float)lua_tonumber(&io_luaState, -1);
		}
		return result;
	}

	eae6320::cResult ReadInt(const char* const i_path, lua_State& io_luaState, const char* p_Key, uint32_t& o_Result) {
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

				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TNUMBER)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(p_Key) + "\" must be a float "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			o_Result = (uint32_t)lua_tonumber(&io_luaState, -1);
		}
		return result;
	}

	eae6320::cResult ReadByte(const char* const i_path, lua_State& io_luaState, const char* p_Key, BYTE& o_Result) {
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

				return result;
			}

			if (lua_type(&io_luaState, -1) != LUA_TNUMBER)
			{
				result = eae6320::Results::InvalidFile;
				std::string err_message = "The value for \"" + std::string(p_Key) + "\" must be a float "
					"(instead of a " + luaL_typename(&io_luaState, -1) + ")";
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, err_message.c_str());
				return result;
			}

			o_Result = (BYTE)lua_tonumber(&io_luaState, -1);
		}
		return result;
	}

	eae6320::cResult LoadEffectValues(const char* const i_path, lua_State& io_luaState, eae6320::Audio::sEffect& o_MusicEffect) {
		auto result = eae6320::Results::Success;

		bool reverb = false;
		if (!(result = ReadBool(i_path, io_luaState, "reverb", reverb))) {
			return result;
		}

		if (reverb) {
			float density = 100;
			ReadFloat(i_path, io_luaState, "density", density);
			o_MusicEffect.m_Density = density;

			float decayTime = 1;
			ReadFloat(i_path, io_luaState, "decayTime", decayTime);
			o_MusicEffect.m_DecayTime = decayTime;

			float roomSize = 100;
			ReadFloat(i_path, io_luaState, "roomSize", roomSize);
			o_MusicEffect.m_RoomSize = roomSize;

			float wetDryMix = 100;
			ReadFloat(i_path, io_luaState, "wetDryMix", wetDryMix);
			o_MusicEffect.m_WetDryMix = wetDryMix;

			uint32_t reflectionDelay = 5;
			ReadInt(i_path, io_luaState, "reflectionDelay", reflectionDelay);
			o_MusicEffect.m_ReflectionDel = reflectionDelay;

			BYTE positionLeft = 6;
			ReadByte(i_path, io_luaState, "positionLeft", positionLeft);
			o_MusicEffect.m_PositionLeft = positionLeft;

			BYTE positionRight = 6;
			ReadByte(i_path, io_luaState, "positionRight", positionRight);
			o_MusicEffect.m_PositionRight = positionRight;
		}

		o_MusicEffect.m_Reverb = reverb;

		return result;
	}

	eae6320::cResult LoadAsset(const char* const i_path, std::wstring& o_MusicPath, eae6320::Audio::sMusicConfig& o_MusicData, eae6320::Audio::sEffect& o_MusicEffect) {
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


		result = LoadTableValues(i_path, *luaState, o_MusicPath, o_MusicData, o_MusicEffect);


		return result;
	}
}
