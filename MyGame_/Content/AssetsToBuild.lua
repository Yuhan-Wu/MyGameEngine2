--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
	shaders =
	{
		{ path = "Shaders/Vertex/vertexInputLayout_mesh.shader", arguments = { "vertex" } },
	},

	meshes = {
		{ path = "Meshes/square.mesh" },
		{ path = "Meshes/triangle1.mesh" },
		{ path = "Meshes/triangle2.mesh" },
		{ path = "Meshes/plane.mesh" },
		{ path = "Meshes/Floor.mesh" },
		{ path = "Meshes/Cube.mesh" },
		{ path = "Meshes/Sphere.mesh" },
		{ path = "Meshes/Pyramid.mesh" },
		{ path = "Meshes/Gear.mesh" },
		{ path = "Meshes/Cat.mesh" },
	},

	effects = {
		{ path = "Effects/standard.effect" },
		{ path = "Effects/animated.effect" },
	},

	audio = {
		{ path = "Audio/background.audio" },
	},
}
