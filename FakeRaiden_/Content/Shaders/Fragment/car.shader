/*
	This is the standard fragment shader

	A fragment shader is responsible for telling the GPU what color a specific fragment should be
*/

#include <Shaders/shaders.inc>

FragmentMain
{
	
	float4 calculatedColor = float4(
		// RGB (color)
		0.06, 0.43, 0.04,
		// Alpha (opacity)
		1.0 );
	float4 combinedColor = calculatedColor * i_color;

	// Output solid white
	o_color = combinedColor;
}