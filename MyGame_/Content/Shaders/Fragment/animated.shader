/*
	This is the standard fragment shader

	A fragment shader is responsible for telling the GPU what color a specific fragment should be
*/

#include <Shaders/shaders.inc>

FragmentMain
{
	float random_color1 = sin(g_elapsedSecondCount_simulationTime);
	if(random_color1 < 0){
		random_color1 += 1;
	}

	float random_color2 = cos(g_elapsedSecondCount_simulationTime);
	float random_color3 = 0.0f;
	if(random_color2 < 0){
		random_color3 = -random_color2;
		random_color2 += 1;
	}
	float4 calculatedColor = float4(
		// RGB (color)
		random_color1, random_color2, random_color3,
		// Alpha (opacity)
		1.0 );
	float4 combinedColor = calculatedColor * i_color;

	// Output solid white
	o_color = combinedColor;
}