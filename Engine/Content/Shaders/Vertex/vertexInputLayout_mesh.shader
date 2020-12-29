/*
	This vertex shader is used to create a Direct3D vertex input layout object
*/

#include <Shaders/shaders.inc>

VertexMain
{
#if defined( EAE6320_PLATFORM_D3D )
	// The shader program is only used to generate a vertex input layout object;
	// the actual shading code is never used
	o_position = float4( i_position, 1.0 );

#elif defined( EAE6320_PLATFORM_GL )
	// The shader program is only used by Direct3D
	gl_Position = float4( i_position, 1.0 );

#endif
	o_color = i_color;
}