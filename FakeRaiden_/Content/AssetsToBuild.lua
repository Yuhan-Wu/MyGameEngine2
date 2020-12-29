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
		{ path = "Meshes/Flag.mesh" },
		{ path = "Meshes/triangle1.mesh" },
		{ path = "Meshes/triangle2.mesh" },
		{ path = "Meshes/Plane.mesh" },
		{ path = "Meshes/Floor.mesh" },
		{ path = "Meshes/Cube.mesh" },
		{ path = "Meshes/Bullet.mesh" },
		{ path = "Meshes/UFO.mesh" },
		{ path = "Meshes/Rocket.mesh" },
		{ path = "Meshes/Car.mesh" },
	},

	effects = {
		{ path = "Effects/standard.effect" },
		{ path = "Effects/animated.effect" },
		{ path = "Effects/plane.effect" },
		{ path = "Effects/ufo.effect" },
		{ path = "Effects/car.effect" },
	},

	audio = {
		{ path = "Audio/background.audio" },
		{ path = "Audio/trigger.audio" },
	},

	collider = {
		{ path = "Colliders/bottom.mcollider" },
		{ path = "Colliders/top.mcollider" },
		{ path = "Colliders/left.mcollider" },
		{ path = "Colliders/right.mcollider" },
		{ path = "Colliders/enemy.mcollider" },
		{ path = "Colliders/rocket.mcollider" },
		{ path = "Colliders/rocketbullet.mcollider" },
	},
}
