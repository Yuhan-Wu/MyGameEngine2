// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Time/Time.h>
#include <Engine/Math/Functions.h>
#include <math.h>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space)) {
		// Pause the game
		Paused = true;
		SetSimulationRate(0);
		if (!MusicPaused) {
			music[0]->SetPitchRatio(1.2f);
			MusicPaused = true;
		}
		
	}
	else {
		Paused = false;
		SetSimulationRate(1.0f);
		if (MusicPaused) {
			music[0]->SetPitchRatio(1);
			MusicPaused = false;
		}
	}
}

void eae6320::cMyGame::UpdateSimulationBasedOnInput() {
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Shift)) {
		// remove the first one
		objects[0].SetIsActive(false);
	}
	else {
		objects[0].SetIsActive(true);
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Control)) {
		// bind a different effect with the second one
		objects[1].SetEffect(effects[0]);
	}
	else {
		objects[1].SetEffect(effects[1]);
	}

	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Enter)) {
		objects[1].SetMesh(meshes[2]);
	}
	else {
		objects[1].SetMesh(meshes[1]);
	}

	Math::sVector force(0, 0, 0);
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Left)) {
		force.x = -0.5f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Right)) {
		force.x = 0.5f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Up)) {
		force.y = 0.5f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Down)) {
		force.y = -0.5f;
	}
	// objects[1].SetSpeed(velocity);
	objects[1].AddForce(force);

	Math::sVector camera_velocity(0, 0, 0);
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::A)) {
		camera_velocity.x = -1.0f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::D)) {
		camera_velocity.x = 1.0f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::W)) {
		camera_velocity.z = -1.0f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::S)) {
		camera_velocity.z = 1.0f;
	}
	active_camera.SetSpeed(camera_velocity);

	// Only x axis for now
	float rotation = 0.0f;
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::E)) {
		rotation = -0.5f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Q)) {
		rotation = 0.5f;
	}
	
	active_camera.SetRotation(rotation);
}

void eae6320::cMyGame::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) {
	/*
	if (LastTime > 1) {
		StopDrawing = !StopDrawing;
		LastTime = 0.0f;
	}
	else {
		LastTime += i_elapsedSecondCount_sinceLastUpdate;
	}*/
	
	objects[1].Update(i_elapsedSecondCount_sinceLastUpdate);
	active_camera.Update(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cMyGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate) {
	// Submit background color
	float simulation_count = (float)GetElapsedSecondCount_simulation();
	float choice1 = std::abs(std::cos(simulation_count));
	float choice2 = std::abs(std::sin(simulation_count));
	eae6320::Graphics::sColor background(choice1, choice2, choice1);
	eae6320::Graphics::SubmitBackgroundColor(background);

	for (int i = sizeof(objects) / sizeof(objects[0]) - 1; i >= 0; i--) {
		objects[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}

	active_camera.SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
}

// Initialize / Clean Up
//----------------------

eae6320::cResult eae6320::cMyGame::Initialize()
{
	char str[100];
	strcpy(str, GetMainWindowName());
	strcat(str, " was successfully initialized.");
	Logging::OutputMessage(str);

	
	{
		const char* music_path = "data/Audio/background.baudio";
		if (!eae6320::Audio::cMusic::Load(music_path, music[0]))
		{
			EAE6320_ASSERTF(false, "Can't initialize music.");
		}
	}
	
	music[0]->Play();

	// Initialize the shading data
	
	{
		char effect_path[] = "data/Effects/animated.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[0]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}

	// Initialize the geometry
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/cat.bmesh", meshes[0]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[0] = nullptr;
		}
	}


	objects[0] = GameObject(meshes[0], effects[0]);

	// Initialize the shading data
	{
		char effect_path[] = "data/Effects/standard.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[1]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}

	// Initialize the geometry
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/Pyramid.bmesh", meshes[1]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[1] = nullptr;
		}
	}

	objects[1] = GameObject(meshes[1], effects[1]);

	// Initialize the geometry
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/triangle2.bmesh", meshes[2]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[2] = nullptr;
		}
	}

	// Initialize the geometry
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/Floor.bmesh", meshes[3]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[3] = nullptr;
		}
	}

	objects[2] = GameObject(meshes[3], effects[1]);

	cameras[0] = Camera();
	active_camera = cameras[0];
	
	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{

	// TODO delete audio

	for (int i = 0; i < 2; i++) {
		effects[i]->DecrementReferenceCount();
		effects[i] = nullptr;
		
	}
	for (int i = 0; i < 4; i++) {
		meshes[i]->DecrementReferenceCount();
		meshes[i] = nullptr;
	}

	delete music[0];

	char str[100];
	strcpy(str, GetMainWindowName());
	strcat(str, " was successfully cleaned up.");
	Logging::OutputMessage(str);
	return Results::Success;
}
