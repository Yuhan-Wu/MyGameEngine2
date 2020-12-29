/*
	This is the standard vertex shader

	A vertex shader is responsible for two things:
		* Telling the GPU where the vertex (one of the three in a triangle) should be drawn on screen in a given window
			* The GPU will use this to decide which fragments (i.e. pixels) need to be shaded for a given triangle
		* Providing any data that a corresponding fragment shader will need
			* This data will be interpolated across the triangle and thus vary for each fragment of a triangle that gets shaded
*/

#include <Shaders/shaders.inc>

VertexMain
{
	// Transform the local vertex into world space
	float4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		float4 vertexPosition_local = float4( i_position, 1.0 );
		vertexPosition_world = MutiplyMatrix( g_transform_localToWorld, vertexPosition_local );
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		float4 vertexPosition_camera = MutiplyMatrix( g_transform_worldToCamera, vertexPosition_world );
#if defined( EAE6320_PLATFORM_D3D )
		// Project the vertex from camera space into projected space
		o_position
#elif defined( EAE6320_PLATFORM_GL )
		// Project the vertex from camera space into projected space
		gl_Position
#endif
		= MutiplyMatrix( g_transform_cameraToProjected, vertexPosition_camera );
	}

	o_color = i_color;
}