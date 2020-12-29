// Includes
//=========

#include "sRigidBodyState.h"

#include <Engine/Math/cMatrix_transformation.h>

// Interface
//==========

void eae6320::Physics::sRigidBodyState::Update( const float i_secondCountToIntegrate )
{
	// Update acceleration
	{
		acceleration = force / mass;
	}
	// Update position
	{
		Math::sVector new_speed = orientation * velocity;
		position += new_speed * i_secondCountToIntegrate;
	}
	// Update velocity
	{
		velocity += acceleration * i_secondCountToIntegrate;
		if (velocity.x > max_velocity) {
			velocity.x = max_velocity;
		}
		if (velocity.y > max_velocity) {
			velocity.y = max_velocity;
		}
		if (velocity.z > max_velocity) {
			velocity.z = max_velocity;
		}

		// Damping(acceleration.x, velocity.x, i_secondCountToIntegrate);
		// Damping(acceleration.y, velocity.y, i_secondCountToIntegrate);
		// Damping(acceleration.z, velocity.z, i_secondCountToIntegrate);
	}
	// Update orientation
	{
		const auto rotation = Math::cQuaternion( angularSpeed * i_secondCountToIntegrate, angularVelocity_axis_local );
		orientation = orientation * rotation;
		orientation.Normalize();
	}
}

void eae6320::Physics::sRigidBodyState::Damping(float p_AccelerationAlongAxis, float& p_VelocityAlongAxis, float p_DeltaTime) {
	if (p_AccelerationAlongAxis == 0 && p_VelocityAlongAxis != 0) {
		if (p_VelocityAlongAxis > 0) {
			p_VelocityAlongAxis -= damping_rate * p_DeltaTime;
			if (p_VelocityAlongAxis < 0) {
				p_VelocityAlongAxis = 0;
			}
		}
		else {
			p_VelocityAlongAxis += damping_rate * p_DeltaTime;
			if (p_VelocityAlongAxis > 0) {
				p_VelocityAlongAxis = 0;
			}
		}
		
	}
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::PredictFuturePosition( const float i_secondCountToExtrapolate ) const
{
	Math::sVector new_speed = orientation * velocity;
	return position + ( new_speed * i_secondCountToExtrapolate );
}

eae6320::Math::cQuaternion eae6320::Physics::sRigidBodyState::PredictFutureOrientation( const float i_secondCountToExtrapolate ) const
{
	const auto rotation = Math::cQuaternion( angularSpeed * i_secondCountToExtrapolate, angularVelocity_axis_local );
	return Math::cQuaternion( orientation * rotation ).GetNormalized();
}

eae6320::Math::cMatrix_transformation eae6320::Physics::sRigidBodyState::PredictFutureTransform( const float i_secondCountToExtrapolate ) const
{
	return Math::cMatrix_transformation( PredictFutureOrientation( i_secondCountToExtrapolate ), PredictFuturePosition( i_secondCountToExtrapolate ) );
}

void eae6320::Physics::sRigidBodyState::SetUseGravity(bool i_isStatic)
{
	useGravity = i_isStatic;
	if (useGravity)
		force += Math::sVector(0, -9.8f * mass, 0);
	else
		force -= Math::sVector(0, -9.8f * mass, 0);
}

bool eae6320::Physics::sRigidBodyState::GetUseGravity()
{
	return useGravity;
}

void eae6320::Physics::sRigidBodyState::SetPosition(Math::sVector i_position)
{
	position = i_position;
}

void eae6320::Physics::sRigidBodyState::SetRotation(Math::cQuaternion i_rotation)
{
	orientation = i_rotation;
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::GetPosition()
{
	return position;
}

eae6320::Math::cQuaternion eae6320::Physics::sRigidBodyState::GetRotation()
{
	return orientation;
}

void eae6320::Physics::sRigidBodyState::SetVelocity(Math::sVector i_velocity)
{
	velocity = i_velocity;
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::GetVelocity()
{
	return velocity;
}

void eae6320::Physics::sRigidBodyState::AddVelocity(Math::sVector i_velocity)
{
	velocity += i_velocity;
}

void eae6320::Physics::sRigidBodyState::SetAngularVelocity(Math::sVector i_angularVelocityAxis, float i_angularSpeed)
{
	angularVelocity_axis_local = i_angularVelocityAxis;
	angularSpeed = i_angularSpeed;
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::GetAngularVelocityAxis()
{
	return angularVelocity_axis_local;
}

float eae6320::Physics::sRigidBodyState::GetAngularVelocitySpeed()
{
	return angularSpeed;
}

void eae6320::Physics::sRigidBodyState::SetForce(Math::sVector i_force)
{
	force = i_force;

	if (useGravity)
	{
		force += Math::sVector(0, -9.8f * mass, 0);
	}
}

eae6320::Math::sVector eae6320::Physics::sRigidBodyState::GetForce()
{
	return force;
}

void eae6320::Physics::sRigidBodyState::AddForce(Math::sVector i_force)
{
	force += i_force;
}

void eae6320::Physics::sRigidBodyState::AddImpulse(Math::sVector i_impulse)
{
	velocity += i_impulse / mass;
}

void eae6320::Physics::sRigidBodyState::AddAngularImpulse(Math::sVector i_angularImpulse)
{
}

void eae6320::Physics::sRigidBodyState::SetMass(float i_mass)
{
	mass = i_mass;
}

float eae6320::Physics::sRigidBodyState::GetMass()
{
	return mass;
}
