/*
	This class builds shaders
*/

#ifndef EAE6320_CMESHBUILDER_H
#define EAE6320_CMESHBUILDER_H

// Includes
//=========

#include <Tools/AssetBuildLibrary/iBuilder.h>

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		class cAudioBuilder final : public iBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			cResult Build(const std::vector<std::string>& i_arguments) final;

		};
	}
}

#endif	// EAE6320_CMESHBUILDER_H