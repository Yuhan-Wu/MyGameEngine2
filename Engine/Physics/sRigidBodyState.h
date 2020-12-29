/*
	This struct can be used to represent the state of a
	rigid body in 3D space
*/

#ifndef EAE6320_PHYSICS_SRIGIDBODYSTATE_H
#define EAE6320_PHYSICS_SRIGIDBODYSTATE_H

// Includes
//=========

#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>

// Forward Declarations
//=====================

namespace eae6320
{
	namespace Math
	{
		class cMatrix_transformation;
	}
}

// Struct Declaration
//===================

namespace eae6320
{
	namespace Physics
	{
		struct sRigidBodyState
		{
			// Data
			//=====
			Math::sVector force;
			Math::sVector position;	// In arbitrary units determined by the applicaton's convention
			Math::sVector velocity;	// Distance per second
			Math::sVector acceleration;	// Distance per second^2
			Math::cQuaternion orientation;
			Math::sVector angularVelocity_axis_local = Math::sVector( 0.0f, 1.0f, 0.0f );	// In local space (not world space)
			float angularSpeed = 0.0f;	// Radians per second (positive values rotate right-handed, negative rotate left-handed)
			bool useGravity = false;
			float mass = 1.0f;

			float max_velocity = 1.0f;
			float damping_rate = 0.2f;
			// Interface
			//==========

			void Damping(float p_accelerationAlongAxis, float& p_VelocityAlongAxis, float p_DeltaTime);
			void Update( const float i_secondCountToIntegrate );
			Math::sVector PredictFuturePosition( const float i_secondCountToExtrapolate ) const;
			Math::cQuaternion PredictFutureOrientation( const float i_secondCountToExtrapolate ) const;
			Math::cMatrix_transformation PredictFutureTransform( const float i_secondCountToExtrapolate ) const;

			void SetUseGravity(bool i_useGravity);
			bool GetUseGravity();

			// Used by users
			void SetPosition(Math::sVector i_position);
			void SetRotation(Math::cQuaternion);
			Math::sVector GetPosition();
			Math::cQuaternion GetRotation();

			void SetVelocity(Math::sVector i_velocity);
			Math::sVector GetVelocity();
			void AddVelocity(Math::sVector i_velocity);

			void SetAngularVelocity(Math::sVector i_angularVelocityAxis, float i_angularSpeed);
			Math::sVector GetAngularVelocityAxis();
			float GetAngularVelocitySpeed();

			void SetForce(Math::sVector i_force);
			Math::sVector GetForce();
			void AddForce(Math::sVector i_force);

			void AddImpulse(Math::sVector i_impulse);
			void AddAngularImpulse(Math::sVector i_angularImpulse);

			void SetMass(float i_mass);
			float GetMass();
		};
	}
}

#endif	// EAE6320_PHYSICS_SRIGIDBODYSTATE_H
