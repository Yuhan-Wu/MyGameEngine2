// Includes
//=========

#include "cFakeRaiden.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>
#include <Engine/UserInput/UserInput.h>
#include <Engine/Time/Time.h>
#include <Engine/Math/Functions.h>
#include <Engine/Physics/sCollider.h>
#include <Engine/Physics/Physics.h>
#include <math.h>
#include <stdlib.h>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cFakeRaiden::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space)) {
		// Shoot
		if (!isShot) {
			for (int i = 0; i < MAX_ROCKET_BULLET; i++) {
				if (!rocket_bullet[i].GetIsActive()) {
					Math::sVector rocket_pos = rocket[0].GetRigidbody()->GetPosition();
					rocket_bullet[i].GetRigidbody()->SetPosition(rocket_pos + Math::sVector(0, 0.61f, 0));
					rocket_bullet[i].ToggleIsActive();
					rocket_bullet[i].GetRigidbody()->SetVelocity(Math::sVector(0, 1, 0));
					Physics::RegisterCollider(rocket_bullet[i].GetCollider());
					music[1]->Play(true);
					isShot = true;
					break;
				}
			}
		}
	}
}

void eae6320::cFakeRaiden::UpdateSimulationBasedOnInput() {

	Math::sVector player_speed(0, 0, 0);
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Left)) {
		player_speed.x = -1.f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Right)) {
		player_speed.x = 1.f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Up)) {
		player_speed.y = 1.f;
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Down)) {
		player_speed.y = -1.f;
	}
	// objects[1].SetSpeed(velocity);
	rocket[0].SetSpeed(player_speed);

}

void eae6320::cFakeRaiden::UpdateSimulationBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate) {

	if (GameEnd) {
		flag.Update(i_elapsedSecondCount_sinceLastUpdate);
		return;
	}
	
	// Plane
	if (plane[0].GetIsActive()) {
		Math::sVector plane_velocity = plane[0].GetRigidbody()->GetVelocity();
		if (plane[0].GetRigidbody()->GetPosition().x <= -2) {
			plane_velocity = Math::sVector(0.5f, 0, 0);
		}
		if (plane[0].GetRigidbody()->GetPosition().x >= 2) {
			plane_velocity = Math::sVector(-0.5f, 0, 0);
		}
		plane[0].GetRigidbody()->SetVelocity(plane_velocity);
		plane[0].Update(i_elapsedSecondCount_sinceLastUpdate);
	}
	for (int i = 0; i < BOSS_BULLET; i++) {
		plane_bullet[i].Update(i_elapsedSecondCount_sinceLastUpdate);
	}

	// UFO
	for (int i = 0; i < 2; i++) {
		if (ufo[i].GetIsActive()) {
			Math::sVector ufo_velocity = ufo[i].GetRigidbody()->GetVelocity();
			if ((ufo[i].GetRigidbody()->GetPosition().x < ufo_pos[i].x - 1 || ufo[i].GetRigidbody()->GetPosition().x > ufo_pos[i].x + 1) && ufo_velocity.x != 0) {

				ufo_velocity.y = ufo_velocity.x;
				ufo_velocity.x = 0;
			}
			else if ((ufo[i].GetRigidbody()->GetPosition().y < ufo_pos[i].y - 1 || ufo[i].GetRigidbody()->GetPosition().y > ufo_pos[i].y + 1) && ufo_velocity.y != 0) {

				ufo_velocity.x = -ufo_velocity.y;
				ufo_velocity.y = 0;
			}
			ufo[i].GetRigidbody()->SetVelocity(ufo_velocity);
			ufo[i].Update(i_elapsedSecondCount_sinceLastUpdate);
		}
	}
	

	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		ufo_bullet[i].Update(i_elapsedSecondCount_sinceLastUpdate);
	}

	// Rocket bullet
	for (int i = 0; i < MAX_ROCKET_BULLET; i++) {
		rocket_bullet[i].Update(i_elapsedSecondCount_sinceLastUpdate);
	}

	// Car bullet
	for (int i = 0; i < 2; i++) {
		car[i].Update(i_elapsedSecondCount_sinceLastUpdate);
	}
	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		car_bullet[i].Update(i_elapsedSecondCount_sinceLastUpdate);
	}

	active_camera.Update(i_elapsedSecondCount_sinceLastUpdate);

	rocket[0].Update(i_elapsedSecondCount_sinceLastUpdate);
}

void eae6320::cFakeRaiden::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate) {
	if (active_car == 0 && active_ufo == 0 && active_plane == 0) {
		InitializeSubRound();
	}
	// Submit background color
	if (isBlinking) {
		float choice1 = abs(sin(i_elapsedSecondCount_sinceLastSimulationUpdate));
		float choice2 = abs(cos(i_elapsedSecondCount_sinceLastSimulationUpdate));
		eae6320::Graphics::SubmitBackgroundColor(Graphics::sColor(choice1, choice2, choice1));
		BlinkingTimer++;
		if (BlinkingTimer > BLINKING) {
			BlinkingTimer = 0;
			isBlinking = false;
		}
	}
	else {
		eae6320::Graphics::SubmitBackgroundColor(original_background);
	}

	if (GameEnd) {
		flag.SetIsActive(true);
		flag.SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
		return;
	}

	Physics::CheckCollision();
	
	// Submit plane
	plane[0].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	if (plane[0].GetIsActive()) {
		plane_timer++;
		if (plane_timer >= BOSS_COOL_DOWN) {
			for (int i = 0; i < BOSS_BULLET; i++) {
				if (!plane_bullet[i].GetIsActive()) {
					Math::sVector rocket_pos = plane[0].GetRigidbody()->GetPosition();
					plane_bullet[i].GetRigidbody()->SetPosition(rocket_pos - Math::sVector(0, 0.61f, 0));
					plane_bullet[i].SetIsActive(true);
					plane_bullet[i].GetRigidbody()->SetVelocity(Math::sVector(0, -1, 0));
					Physics::RegisterCollider(plane_bullet[i].GetCollider());
					plane_timer = 0;
					break;
				}
			}
		}
	}
	else {
		active_plane = 0;
	}
	for (int i = 0; i < BOSS_BULLET; i++) {
		plane_bullet[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}

	// Submit ufo
	int ufo_num = 0;
	if(active_ufo > 0)
		ufo_timer++;
	for (int i = 0; i < 2; i++) {
		if (ufo[i].GetIsActive()) {
			ufo_num++;
			if (ufo_timer >= ENEMY_COOL_DOWN) {
				for (int j = 0; j < ENEMY_BULLET * 2; j++) {
					if (!ufo_bullet[j].GetIsActive()) {
						Math::sVector rocket_pos = ufo[i].GetRigidbody()->GetPosition();
						ufo_bullet[j].GetRigidbody()->SetPosition(rocket_pos - Math::sVector(0, 0.61f, 0));
						ufo_bullet[j].SetIsActive(true);
						ufo_bullet[j].GetRigidbody()->SetVelocity(Math::sVector(0, -1, 0));
						Physics::RegisterCollider(ufo_bullet[j].GetCollider());
						break;
					}
				}
			}
		}
		ufo[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}
	active_ufo = ufo_num;
	if(ufo_timer > ENEMY_COOL_DOWN) ufo_timer = 0;
	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		ufo_bullet[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}

	// Submit Car
	int car_num = 0;
	if (active_car > 0)
		car_timer++;
	for (int i = 0; i < 2; i++) {
		car[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
		Math::sVector car_pos = car[i].GetRigidbody()->GetPosition();
		if (car[i].GetIsActive()) {
			car_num++;
			if (car_pos.y < -5) {
				car[i].SetIsActive(false);
				active_car--;
				Physics::DeregisterCollider(car[i].GetCollider());
			}
			if (car_timer >= ENEMY_COOL_DOWN) {
				for (int j = 0; j < ENEMY_BULLET * 2; j++) {
					if (!car_bullet[j].GetIsActive()) {
						car_bullet[j].GetRigidbody()->SetPosition(car_pos + Math::sVector(0.61f, 0.61f, 0));
						car_bullet[j].SetIsActive(true);
						car_bullet[j].GetRigidbody()->SetVelocity(Math::sVector(1, 1, 0));
						Physics::RegisterCollider(car_bullet[j].GetCollider());
						break;
					}
				}
			}
		}
	}
	if(car_timer >= ENEMY_COOL_DOWN) car_timer = 0;
	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		car_bullet[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}

	// Submit rocket
	if (isShot) {
		rocket_timer++;
		if (rocket_timer >= ROCKET_COOL_DOWN) {
			rocket_timer = 0;
			isShot = false;
		}
	}
	rocket[0].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	for (int i = 0; i < MAX_ROCKET_BULLET; i++) {
		rocket_bullet[i].SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
	}
	

	active_camera.SubmitData(i_elapsedSecondCount_sinceLastSimulationUpdate);
}

void eae6320::cFakeRaiden::InitializeSubRound() {
	sub_round++;
	if (sub_round > big_round) {
		// TODO boss fight
		big_round++;
		sub_round = 0;
		active_plane = 1;
		plane[0].SetIsActive(true);
		plane[0].GetRigidbody()->SetPosition(plane_pos);
		plane[0].SetSpeed(Math::sVector(-0.5f, 0, 0));
		Physics::RegisterCollider(plane[0].GetCollider());
	}
	else {
		if (big_round > 2) {
			active_car = 2;
			car[0].SetIsActive(true);
			car[1].SetIsActive(true);
			int rIndex = rand() % 4;
			car[0].GetRigidbody()->SetPosition(car_pos[rIndex]);
			car[1].GetRigidbody()->SetPosition(car_pos[(rIndex + 2) % 4]);
			car[0].GetRigidbody()->SetVelocity(Math::sVector(0, -0.3f, 0));
			car[1].GetRigidbody()->SetVelocity(Math::sVector(0, -0.3f, 0));
			Physics::RegisterCollider(car[0].GetCollider());
			Physics::RegisterCollider(car[1].GetCollider());
		}
		else {
			active_car = 1;
			car[0].SetIsActive(true);
			int rIndex = rand() % 4;
			car[0].GetRigidbody()->SetPosition(car_pos[rIndex]);
			car[0].GetRigidbody()->SetVelocity(Math::sVector(0, -0.3f, 0));
			Physics::RegisterCollider(car[0].GetCollider());
		}
		if (big_round > 4) {
			active_ufo = 2;
			ufo[0].SetIsActive(true);
			ufo[1].SetIsActive(true);
			ufo[0].GetRigidbody()->SetPosition(ufo_pos[0]);
			ufo[1].GetRigidbody()->SetPosition(ufo_pos[0]);
			ufo[0].GetRigidbody()->SetVelocity(Math::sVector(-1, 0, 0));
			ufo[1].GetRigidbody()->SetVelocity(Math::sVector(1, 0, 0));
			Physics::RegisterCollider(ufo[0].GetCollider());
			Physics::RegisterCollider(ufo[1].GetCollider());
		}
		else {
			active_ufo = 1;
			ufo[0].SetIsActive(true);
			ufo[0].GetRigidbody()->SetPosition(ufo_pos[0]);
			ufo[0].GetRigidbody()->SetVelocity(Math::sVector(-1, 0, 0));
			Physics::RegisterCollider(ufo[0].GetCollider());
		}
	}
}

// Initialize / Clean Up
//----------------------

eae6320::cResult eae6320::cFakeRaiden::Initialize()
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

	{
		const char* music_path = "data/Audio/trigger.baudio";
		if (!eae6320::Audio::cMusic::Load(music_path, music[1]))
		{
			EAE6320_ASSERTF(false, "Can't initialize music.");
		}
	}

	
	// Initialize the geometry
	// Initialize walls
	{
		Physics::sCollider* collider_w0 = nullptr;
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/top.bcollider", collider_w0))
		{
			EAE6320_ASSERTF(false, "Can't initialize wall0 without the collider data");
		}
		wall[0] = GameObject();
		collider_w0->m_Type = Physics::ObjectType::WorldStatic;
		wall[0].AttachCollider(collider_w0);
		Physics::RegisterCollider(collider_w0);
	}

	{
		Physics::sCollider* collider_w1 = nullptr;
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/bottom.bcollider", collider_w1))
		{
			EAE6320_ASSERTF(false, "Can't initialize wall1 without the collider data");
		}
		wall[1] = GameObject();
		collider_w1->m_Type = Physics::ObjectType::WorldStatic;
		wall[1].AttachCollider(collider_w1);
		Physics::RegisterCollider(collider_w1);
	}

	{
		Physics::sCollider* collider_w2 = nullptr;
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/left.bcollider", collider_w2))
		{
			EAE6320_ASSERTF(false, "Can't initialize wall2 without the collider data");
		}
		wall[2] = GameObject();
		collider_w2->m_Type = Physics::ObjectType::WorldStatic;
		wall[2].AttachCollider(collider_w2);
		Physics::RegisterCollider(collider_w2);
	}

	{
		Physics::sCollider* collider_w3 = nullptr;
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/right.bcollider", collider_w3))
		{
			EAE6320_ASSERTF(false, "Can't initialize wall3 without the collider data");
		}
		wall[3] = GameObject();
		collider_w3->m_Type = Physics::ObjectType::WorldStatic;
		wall[3].AttachCollider(collider_w3);
		Physics::RegisterCollider(collider_w3);
	}
	
	// Initialize the shading data
	{
		char effect_path[] = "data/Effects/animated.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[0]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}

	{
		char effect_path[] = "data/Effects/standard.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[1]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}

	// Rocket & rocket bullets
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/rocket.bmesh", meshes[1]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[1] = nullptr;
		}
	}
	rocket[0] = Player(meshes[1], effects[1]);
	rocket[0].SetContext(this);
	rocket[0].SetDamageEachTime(20);
	Physics::sCollider* collider1 = nullptr;
	// Initialize the geometry
	{
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/rocket.bcollider", collider1))
		{
			EAE6320_ASSERTF(false, "Can't initialize rocket without the collider data");
		}
	}
	collider1->m_Type = Physics::ObjectType::Player;
	rocket[0].AttachCollider(collider1);
	rocket[0].GetRigidbody()->SetPosition(Math::sVector(0, -2, 0));
	Physics::RegisterCollider(collider1);

	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/bullet.bmesh", meshes[4]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[4] = nullptr;
		}
	}
	for (int i = 0; i < MAX_ROCKET_BULLET; i++) {
		rocket_bullet[i] = Bullet(meshes[4], effects[1]);
		Physics::sCollider* collider_rocket_bullet = nullptr;
		
		// Initialize the geometry
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/rocketbullet.bcollider", collider_rocket_bullet))
			{
				EAE6320_ASSERTF(false, "Can't initialize rocket bullet without the collider data");
			}
		}
		collider_rocket_bullet->m_Type = Physics::ObjectType::PlayerBullet;
		rocket_bullet[i].AttachCollider(collider_rocket_bullet);
		rocket_bullet[i].GetRigidbody()->SetPosition(Math::sVector(0, -1.39f, 0));
		rocket_bullet[i].SetIsActive(false);
	}

	{
		char effect_path[] = "data/Effects/ufo.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[2]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}
	// UFO
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/ufo.bmesh", meshes[2]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[2] = nullptr;
		}
	}
	for (int i = 0; i < 2; i++) {
		ufo[i] = Enemy(meshes[2], effects[2]);
		Physics::sCollider* collider_u = nullptr;
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/enemy.bcollider", collider_u))
			{
				EAE6320_ASSERTF(false, "Can't initialize ufo without the collider data");
			}
		}
		collider_u->m_Type = Physics::ObjectType::Enemy;
		ufo[i].AttachCollider(collider_u);
		ufo[i].SetIsActive(false);
		// ufo[0].GetRigidbody()->SetVelocity(Math::sVector(-1, 0, 0));
	}
	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		ufo_bullet[i] = Bullet(meshes[4], effects[2]);
		Physics::sCollider* collider_ufo_bullet = nullptr;
		// Initialize the geometry
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/rocketbullet.bcollider", collider_ufo_bullet))
			{
				EAE6320_ASSERTF(false, "Can't initialize rocket bullet without the collider data");
			}
		}
		collider_ufo_bullet->m_Type = Physics::ObjectType::EnemyBullet;
		ufo_bullet[i].AttachCollider(collider_ufo_bullet);
		ufo_bullet[i].GetRigidbody()->SetPosition(Math::sVector(0, 0, 0));
		ufo_bullet[i].SetIsActive(false);
	}


	{
		char effect_path[] = "data/Effects/plane.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[3]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}
	// Plane & plane bullets
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/plane.bmesh", meshes[3]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[3] = nullptr;
		}
	}
	plane[0] = Enemy(meshes[3], effects[3]);
	Physics::sCollider* collider_p = nullptr;
	{
		if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/enemy.bcollider", collider_p))
		{
			EAE6320_ASSERTF(false, "Can't initialize plane without the collider data");
		}
	}
	collider_p->m_Type = Physics::ObjectType::Enemy;
	plane[0].AttachCollider(collider_p);
	plane[0].SetIsActive(false);
	plane[0].SetDamageEachTime(25);
	// Physics::RegisterCollider(collider_p);
	for (int i = 0; i < BOSS_BULLET; i++) {
		plane_bullet[i] = Bullet(meshes[4], effects[3]);
		Physics::sCollider* collider_plane_bullet = nullptr;
		// Initialize the geometry
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/rocketbullet.bcollider", collider_plane_bullet))
			{
				EAE6320_ASSERTF(false, "Can't initialize rocket bullet without the collider data");
			}
		}
		collider_plane_bullet->m_Type = Physics::ObjectType::EnemyBullet;
		plane_bullet[i].AttachCollider(collider_plane_bullet);
		plane_bullet[i].GetRigidbody()->SetPosition(Math::sVector(0, 0, 0));
		plane_bullet[i].SetIsActive(false);
	}

	{
		char effect_path[] = "data/Effects/car.beffect";
		if (!eae6320::Graphics::cEffect::Load(effect_path, effects[4]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shading data");
		}
	}
	//Car
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/car.bmesh", meshes[0]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[0] = nullptr;
		}
	}

	for (int i = 0; i < 2; i++) {
		car[i] = Enemy(meshes[0], effects[4]);
		Physics::sCollider* collider_c = nullptr;
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/enemy.bcollider", collider_c))
			{
				EAE6320_ASSERTF(false, "Can't initialize car without the collider data");
			}
		}
		collider_c->m_Type = Physics::ObjectType::Enemy;
		car[i].AttachCollider(collider_c);
		car[i].GetRigidbody()->SetRotation(Math::cQuaternion(0.785f, Math::sVector(0, 0, 1)));
		car[i].SetIsActive(false);
		// car[i].GetRigidbody()->SetPosition(Math::sVector(-3, -2, 0));
	}
	
	// Physics::RegisterCollider(collider_c);
	for (int i = 0; i < ENEMY_BULLET * 2; i++) {
		car_bullet[i] = Bullet(meshes[4], effects[4]);
		Physics::sCollider* collider_car_bullet = nullptr;
		// Initialize the geometry
		{
			if (!eae6320::Physics::sCollider::CreateMeshInstanceFromPath("data/Colliders/rocketbullet.bcollider", collider_car_bullet))
			{
				EAE6320_ASSERTF(false, "Can't initialize rocket bullet without the collider data");
			}
		}
		collider_car_bullet->m_Type = Physics::ObjectType::EnemyBullet;
		car_bullet[i].AttachCollider(collider_car_bullet);
		car_bullet[i].GetRigidbody()->SetPosition(Math::sVector(0, 0, 0));
		car_bullet[i].SetIsActive(false);
	}
	
	// Flag
	{
		if (!eae6320::Graphics::cMesh::Load("data/Meshes/flag.bmesh", meshes[5]))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the geometry data");
			meshes[5] = nullptr;
		}
	}
	flag = GameObject(meshes[5], effects[1]);
	flag.SetIsActive(false);

	// Camera
	cameras[0] = Camera();
	active_camera = cameras[0];
	
	return Results::Success;
}

eae6320::cResult eae6320::cFakeRaiden::CleanUp()
{
	for (int i = 0; i < 5; i++) {
		effects[i]->DecrementReferenceCount();
		effects[i] = nullptr;
		
	}
	for (int i = 0; i < 6; i++) {
		meshes[i]->DecrementReferenceCount();
		meshes[i] = nullptr;
	}

	for (int i = 0; i < 2; i++) {
		delete music[i];
	}

	Physics::CleanUp();

	char str[100];
	strcpy(str, GetMainWindowName());
	strcat(str, " was successfully cleaned up.");
	Logging::OutputMessage(str);
	return Results::Success;
}
