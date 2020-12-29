#include "sCollider.h"

#include <External/Lua/Includes.h>
#include <Engine/Platform/Platform.h>

eae6320::cResult eae6320::Physics::sCollider::CreateMeshInstanceFromPath(const char* i_path, sCollider*& o_ptr)
{
	o_ptr = new sCollider();

	auto result = eae6320::Results::Success;

	Platform::sDataFromFile biData;
	if (!(result = Platform::LoadBinaryFile(i_path, biData)))
		return result;

	auto currentOffset = reinterpret_cast<uintptr_t>(biData.data);

	uint16_t shape = *reinterpret_cast<uint16_t*>(currentOffset);
	currentOffset += sizeof(uint16_t);

	if (shape == 0)
	{
		o_ptr->shape = eColliderType::rectangular;
		o_ptr->isStatic = *reinterpret_cast<bool*>(currentOffset);
		currentOffset += sizeof(bool);
		o_ptr->isTrigger = *reinterpret_cast<bool*>(currentOffset);
		currentOffset += sizeof(bool);
		o_ptr->bounciness = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f1 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f2 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f3 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f4 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f5 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f6 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		o_ptr->colliderData0 = eae6320::Math::sVector(f1, f2, f3);
		o_ptr->colliderData1 = eae6320::Math::sVector(f4, f5, f6);
	}
	else
	{
		o_ptr->shape = eColliderType::circular;
		o_ptr->isStatic = *reinterpret_cast<bool*>(currentOffset);
		currentOffset += sizeof(bool);
		o_ptr->isTrigger = *reinterpret_cast<bool*>(currentOffset);
		currentOffset += sizeof(bool);
		o_ptr->bounciness = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f1 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f2 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f3 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		float f4 = *reinterpret_cast<float*>(currentOffset);
		currentOffset += sizeof(float);
		o_ptr->colliderData0 = eae6320::Math::sVector(f2, f3, f4);
		o_ptr->colliderData2 = f1;
	}
	/*uint16_t vertexCount = *reinterpret_cast<uint16_t*>(currentOffset);

	currentOffset += sizeof(uint16_t);
	uint16_t indexCount = *reinterpret_cast<uint16_t*>(currentOffset);
	uint16_t triangleCount = indexCount / 3;

	currentOffset += sizeof(uint16_t);
	auto* const positions = reinterpret_cast<VertexFormats::sVertex_mesh*>(currentOffset);

	currentOffset += sizeof(VertexFormats::sVertex_mesh) * vertexCount;
	uint16_t* indices = reinterpret_cast<uint16_t*>(currentOffset);

	VertexFormats::sVertex_mesh* positionsCopy = new VertexFormats::sVertex_mesh[vertexCount];
	memcpy(positionsCopy, reinterpret_cast<void*>(positions), sizeof(VertexFormats::sVertex_mesh) * vertexCount);
	uint16_t* indicesCopy = new uint16_t[indexCount];
	memcpy(indicesCopy, reinterpret_cast<void*>(indices), sizeof(uint16_t) * indexCount);*/

	return result;
}

void eae6320::Physics::sCollider::AttachToGameObject(GameObject* i_ptr)
{
	owner = i_ptr;
	body = i_ptr->GetRigidbody();
}
