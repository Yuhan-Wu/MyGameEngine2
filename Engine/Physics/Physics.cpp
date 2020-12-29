#include "Physics.h"
#include "sCollider.h"
#include "sRigidBodyState.h"
#include "eColliderType.h"

#include <Engine/Math/cQuaternion.h>
#include <Engine/Math/sVector.h>
#include <set>
#include <vector>
#include <algorithm>

struct Collision
{
    eae6320::Physics::sCollider* collider1;
    eae6320::Physics::sCollider* collider2;
    eae6320::Math::sVector normal;
    float penetration;
};

bool CheckPair(Collision* io_collision);
bool CheckCircleCircle(Collision* io_collision);
bool CheckRectangleRectangle(Collision* io_collision);
bool CheckRectangleCircle(Collision* io_collision);
void ResolveCollision(Collision* i_collision);

namespace
{
    std::set<eae6320::Physics::sCollider*> s_movingColliders;
    std::set<eae6320::Physics::sCollider*> s_staticColliders;
    std::vector<eae6320::Physics::sCollider*> s_pendingErase;
    float correctionFactor = 0.2f;
}

void eae6320::Physics::CheckCollision()
{
    for (auto it = s_movingColliders.begin(); it != s_movingColliders.end(); it++)
    {
        for (auto it2 = std::next(it, 1); it2 != s_movingColliders.end(); it2++)
        {
            Collision preCollision;
            preCollision.collider1 = *it;
            preCollision.collider2 = *it2;

            if (CheckPair(&preCollision))
            {
                ResolveCollision(&preCollision);
            }
        }

        for (auto it2 = s_staticColliders.begin(); it2 != s_staticColliders.end(); it2++)
        {
            Collision preCollision;
            preCollision.collider1 = *it;
            preCollision.collider2 = *it2;

            if (CheckPair(&preCollision))
            {
                ResolveCollision(&preCollision);
            }
        }
    }

    for (auto elem : s_pendingErase)
    {
        if (elem->isStatic)
            s_staticColliders.erase(elem);
        else
            s_movingColliders.erase(elem);
    }

    s_pendingErase.clear();
}

void eae6320::Physics::RegisterCollider(sCollider* i_collider)
{
    if (i_collider->isStatic)
        s_staticColliders.insert(i_collider);
    else
        s_movingColliders.insert(i_collider);
}

void eae6320::Physics::DeregisterCollider(sCollider* i_collider)
{
    s_pendingErase.push_back(i_collider);
}

void eae6320::Physics::ChangeColliderIsStatic(sCollider* i_collider)
{
    if (i_collider->isStatic)
    {
        s_staticColliders.insert(i_collider);
        s_movingColliders.erase(i_collider);
    }
    else
    {
        s_movingColliders.insert(i_collider);
        s_staticColliders.erase(i_collider);
    }
}

eae6320::cResult eae6320::Physics::CleanUp()
{
    return cResult();
}

bool CheckPair(Collision* io_collision)
{
    eae6320::Physics::eColliderType collider1Type = io_collision->collider1->shape;
    eae6320::Physics::eColliderType collider2Type = io_collision->collider2->shape;

    if (collider1Type == eae6320::Physics::eColliderType::circular
        && collider2Type == eae6320::Physics::eColliderType::circular)
        return CheckCircleCircle(io_collision);
    else if (collider1Type == eae6320::Physics::eColliderType::rectangular
        && collider2Type == eae6320::Physics::eColliderType::rectangular)
        return CheckRectangleRectangle(io_collision);
    else if (collider1Type == eae6320::Physics::eColliderType::rectangular
        && collider2Type == eae6320::Physics::eColliderType::circular)
        return CheckRectangleCircle(io_collision);
    else if (collider1Type == eae6320::Physics::eColliderType::circular
        && collider2Type == eae6320::Physics::eColliderType::rectangular)
    {
        eae6320::Physics::sCollider* temp = io_collision->collider1;
        io_collision->collider1 = io_collision->collider2;
        io_collision->collider2 = temp;
        return CheckRectangleCircle(io_collision);
    }
    else
        return false;
}

bool CheckCircleCircle(Collision* io_collision)
{
    eae6320::Physics::sCollider* collider1 = io_collision->collider1;
    eae6320::Physics::sCollider* collider2 = io_collision->collider2;

    eae6320::Math::sVector from1to2 = (collider2->colliderData0 + collider2->body->GetPosition())
        - (collider1->colliderData0 + collider1->body->GetPosition());

    float radiusSum = collider1->colliderData2 + collider2->colliderData2;
    float distance = from1to2.GetLength();

    if (distance > radiusSum)
        return false;

    float radiusSumSqaure = radiusSum * radiusSum;

    if (distance != 0)
    {
        io_collision->penetration = radiusSum - distance;
        io_collision->normal = from1to2 / distance;
    }
    else
    {
        io_collision->penetration = collider1->colliderData2;
        io_collision->normal = eae6320::Math::sVector(1, 0, 0);
    }

    return true;
}

bool CheckRectangleRectangle(Collision* io_collision)
{
    eae6320::Physics::sCollider* collider1 = io_collision->collider1;
    eae6320::Physics::sCollider* collider2 = io_collision->collider2;

    eae6320::Math::sVector from1to2 = (collider2->colliderData0 + collider2->body->GetPosition())
        - (collider1->colliderData0 + collider1->body->GetPosition());

    float xExtent1 = (collider1->colliderData1.x - collider1->colliderData0.x) / 2;
    float xExtent2 = (collider2->colliderData1.x - collider2->colliderData0.x) / 2;

    float xOverlap = xExtent1 + xExtent2 - abs(from1to2.x);

    if (xOverlap > 0)
    {
        float yExtent1 = (collider1->colliderData1.y - collider1->colliderData0.y) / 2;
        float yExtent2 = (collider2->colliderData1.y - collider2->colliderData0.y) / 2;

        float yOverlap = yExtent1 + yExtent2 - abs(from1to2.y);

        if (yOverlap > 0)
        {
            if (xOverlap > yOverlap)
            {
                io_collision->penetration = xOverlap;
                if (from1to2.y < 0)
                    io_collision->normal = eae6320::Math::sVector(0, -1, 0);
                else
                    io_collision->normal = eae6320::Math::sVector(0, 1, 0);
            }
            else
            {
                io_collision->penetration = yOverlap;
                if (from1to2.x < 0)
                    io_collision->normal = eae6320::Math::sVector(-1, 0, 0);
                else
                    io_collision->normal = eae6320::Math::sVector(1, 0, 0);
            }
        }
    }

    return true;
}

bool CheckRectangleCircle(Collision* io_collision)
{
    eae6320::Physics::sCollider* collider1 = io_collision->collider1;
    eae6320::Physics::sCollider* collider2 = io_collision->collider2;

    eae6320::Math::sVector from1to2 = (collider2->colliderData0 + collider2->body->GetPosition())
        - ((collider1->colliderData0 + collider1->colliderData1) / 2.f + collider1->body->GetPosition());

    float xExtent1 = (collider1->colliderData1.x - collider1->colliderData0.x) / 2;
    float yExtent1 = (collider1->colliderData1.y - collider1->colliderData0.y) / 2;

    eae6320::Math::sVector closestOnAToB;

    float tempExtent0 = xExtent1;
    if (xExtent1 < 0) tempExtent0 = -tempExtent0;
    closestOnAToB.x = std::clamp(from1to2.x, -tempExtent0, tempExtent0);
    float tempExtent1 = yExtent1;
    if (yExtent1 < 0) tempExtent1 = -tempExtent1;
    closestOnAToB.y = std::clamp(from1to2.y, -tempExtent1, tempExtent1);

    bool inside = false;

    eae6320::Math::sVector normal = from1to2 - closestOnAToB;
    float distance = normal.GetLength();
    float radius = collider2->colliderData2;

    if (distance > radius)
        return false;

    if (distance != 0)
        io_collision->normal = normal.GetNormalized();
    else
        io_collision->normal = eae6320::Math::sVector(0, 1, 0);
    io_collision->penetration = radius - distance;

    return true;
}

void ResolveCollision(Collision* i_collision)
{
    eae6320::Physics::sCollider* collider1 = i_collision->collider1;
    eae6320::Physics::sCollider* collider2 = i_collision->collider2;

    if (collider1->isTrigger && collider2->isTrigger)
    {
        collider1->owner->OnTrigger(collider2);
        collider2->owner->OnTrigger(collider1);
        return;
    }
    else if (collider1->isTrigger)
    {
        collider1->owner->OnTrigger(collider2);
        collider2->owner->OnCollision(collider1);
        return;
    }
    else if (collider2->isTrigger)
    {
        collider1->owner->OnCollision(collider2);
        collider2->owner->OnTrigger(collider1);
        return;
    }
    else
    {
        collider1->owner->OnCollision(collider2);
        collider2->owner->OnCollision(collider1);
    }

    eae6320::Physics::sRigidBodyState* rigidbody1 = collider1->body;
    eae6320::Physics::sRigidBodyState* rigidbody2 = collider2->body;
    eae6320::Math::sVector normal = i_collision->normal;

    eae6320::Math::sVector relativeVelocity;
    float reverseMassFactor;

    if (collider1->isStatic == true)
    {
        relativeVelocity = rigidbody2->GetVelocity();
        reverseMassFactor = 1 / rigidbody2->GetMass();
    }
    else if (collider2->isStatic == true)
    {
        relativeVelocity = -rigidbody1->GetVelocity();
        reverseMassFactor = 1 / rigidbody1->GetMass();
    }
    else
    {
        relativeVelocity = rigidbody2->GetVelocity() - rigidbody1->GetVelocity();
        reverseMassFactor = 1 / rigidbody1->GetMass() + 1 / rigidbody2->GetMass();
    }

    float speedAlongNormal = normal.x * relativeVelocity.x + normal.y * relativeVelocity.y;

    if (speedAlongNormal > 0)
        return;

    float bounciness = collider1->bounciness > collider2->bounciness ? collider2->bounciness : collider1->bounciness;

    float impulseScale = speedAlongNormal * (bounciness + 1) / reverseMassFactor;

    eae6320::Math::sVector impulse = normal * impulseScale;

    eae6320::Math::sVector correction = normal * correctionFactor * i_collision->penetration / (reverseMassFactor);

    if (collider1->isStatic == true)
    {
        rigidbody2->AddImpulse(-impulse);
        rigidbody2->SetPosition(rigidbody2->GetPosition() + correction / rigidbody2->GetMass());
    }
    else if (collider2->isStatic == true)
    {
        rigidbody1->AddImpulse(impulse);
        rigidbody1->SetPosition(rigidbody1->GetPosition() - correction / rigidbody1->GetMass());
    }
    else
    {
        rigidbody1->AddImpulse(impulse);
        rigidbody2->AddImpulse(-impulse);

        rigidbody1->SetPosition(rigidbody1->GetPosition() - correction / rigidbody1->GetMass());
        rigidbody2->SetPosition(rigidbody2->GetPosition() + correction / rigidbody2->GetMass());
    }
}