/*
	This file should be #included by all shaders
*/

// Version Information
//====================

#if defined( EAE6320_PLATFORM_GL )

	// GLSL shaders require the version to be #defined before anything else in the shader
	#version 420

#endif

// Constant Buffers
//=================

#if defined( EAE6320_PLATFORM_D3D )

	#define DeclareConstantBuffer( i_name, i_id ) cbuffer i_name : register( b##i_id )
	#define mat4 float4x4
	#define vec2 float2
	#define vec4 float4
	#define vec3 float3
	#define MutiplyMatrix( matrix1, matrix2 ) mul( matrix1, matrix2 )
	#define VertexMain void main( in const float3 i_position : POSITION, in const float4 i_color : COLOR, out float4 o_position : SV_POSITION, out float4 o_color : COLOR )
	#define FragmentMain void main( in const float4 i_fragmentPosition : SV_POSITION, in const float4 i_color : COLOR, out float4 o_color : SV_TARGET )

#elif defined( EAE6320_PLATFORM_GL )

	#define DeclareConstantBuffer( i_name, i_id ) layout( std140, binding = i_id ) uniform i_name
	#define float4x4 mat4
	#define float2 vec2
	#define float4 vec4
	#define float3 vec3
	#define MutiplyMatrix( matrix1, matrix2 ) ( matrix1 * matrix2 )
	#define VertexMain layout( location = 0 ) in float3 i_position; layout( location = 1 ) in float4 i_color; layout( location = 0 ) out float4 o_color; void main()
	#define FragmentMain layout( location = 0 ) in float4 i_color; out float4 o_color; void main()

#endif

// Constant Buffers
//=================
DeclareConstantBuffer( g_constantBuffer_frame, 0 )
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For float4 alignment
	float2 g_padding;
};

DeclareConstantBuffer( g_constantBuffer_drawCall, 2 ){
	float4x4 g_transform_localToWorld;
};