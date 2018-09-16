#pragma once
#pragma once
#include "Functor.h" 
#include "MapObject.h"

class CollisionUtil
{
public:

	explicit CollisionUtil() {};
	virtual ~CollisionUtil() {};

public:
	static XMFLOAT3 GetSlideVector(GameObject* obj, GameObject* collObject)
	{
		XMFLOAT3 Center = XMFLOAT3{ collObject->world_._41, collObject->world_._42, collObject->world_._43 };

		XMFLOAT3 Player = XMFLOAT3{ obj->world_._41, obj->world_._42, obj->world_._43 };
		XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터
		XMFLOAT3 MovingRefletVector = XMFLOAT3{ 0, 0, 0 };

		float tanceta = dirVector.z / dirVector.x;
		float ceta = atan(tanceta) * 57.3248f;

		float extenttanceta = collObject->xmOOBB_.Extents.z / collObject->xmOOBB_.Extents.x;
		float extentceta = atan(extenttanceta) * 57.3248f;

		dirVector = Vector3::Normalize(dirVector);

		if (Player.x > Center.x && 0 < ceta && ceta < extentceta)
			MovingRefletVector = XMFLOAT3(1, 0, 0); // Right
		if (Player.z > Center.z && extentceta < ceta && ceta < 90)
			MovingRefletVector = XMFLOAT3(0, 0, 1); // Top
		if (Player.z > Center.z && -90 < ceta && ceta < -extentceta)
			MovingRefletVector = XMFLOAT3(0, 0, 1); // Top
		if (Player.x < Center.x && -extentceta < ceta && ceta < 0)
			MovingRefletVector = XMFLOAT3(-1, 0, 0); // Left
		if (Player.x < Center.x && 0 < ceta && ceta < extentceta)
			MovingRefletVector = XMFLOAT3(-1, 0, 0); // Left
		if (Player.z < Center.z && extentceta < ceta && ceta < 90)
			MovingRefletVector = XMFLOAT3(0, 0, -1); // Bottom
		if (Player.z < Center.z && -90 < ceta && ceta < -extentceta)
			MovingRefletVector = XMFLOAT3(0, 0, -1); // Bottom
		if (Player.x > Center.x && -extentceta < ceta && ceta < 0)
			MovingRefletVector = XMFLOAT3(1, 0, 0); // Right

		return MovingRefletVector;
	}

	static XMFLOAT3 GetMapSlideVector(GameObject* obj, GameObject* collMapObject)
	{
		XMMATRIX world = XMLoadFloat4x4(&collMapObject->world_);

		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

		// View space to the object's local space.

		// Transform the camera frustum from view space to the object's local space.

		BoundingOrientedBox mLocalPlayerBounds;
		// Transform the camera frustum from view space to the object's local space.
		obj->xmOOBB_.Transform(mLocalPlayerBounds, invWorld);

		BoundingBox WorldBounds = dynamic_cast<MapObject*>(collMapObject)->Bounds;
		// Transform the camera frustum from view space to the object's local space.
		WorldBounds.Transform(WorldBounds, XMLoadFloat4x4(&(collMapObject->world_)));


		//XMFLOAT3 pCollObjPos2= pCollobj->m_pTransCom->m_f3Position;
		//XMFLOAT3 Center = pCollobj->GetTransform()->GetPosition();
		//XMFLOAT3 Player = pobj->GetPosition();

		XMFLOAT3 Center = XMFLOAT3(collMapObject->world_._41, collMapObject->world_._42, collMapObject->world_._43);
		XMFLOAT3 Player = XMFLOAT3(obj->world_._41, obj->world_._42, obj->world_._43);
		XMFLOAT3 look = XMFLOAT3(collMapObject->world_._21, collMapObject->world_._22, collMapObject->world_._23);
		XMFLOAT3 dirVector = Vector3::Subtract(Player, Center);   // 충돌 객체에서 플레이어로 가는 벡터
		XMFLOAT3 MovingRefletVector = XMFLOAT3{ 0, 0, 0 };

		float tanceta = dirVector.z / dirVector.x;
		float ceta = atan(tanceta) * 57.3248f;

		float extenttanceta = WorldBounds.Extents.z / WorldBounds.Extents.x;

		float extentceta = atan(extenttanceta) * 57.3248f;

		look = Vector3::Normalize(look);
		dirVector = Vector3::Normalize(dirVector);

		if (Player.x > Center.x && 0 < ceta && ceta < extentceta)
			MovingRefletVector = XMFLOAT3(1, 0, 0);
		if (Player.z > Center.z && extentceta < ceta && ceta < 90)
			MovingRefletVector = XMFLOAT3(0, 0, 1);
		if (Player.z > Center.z && -90 < ceta && ceta < -extentceta)
			MovingRefletVector = XMFLOAT3(0, 0, 1);
		if (Player.x < Center.x && -extentceta < ceta && ceta < 0)
			MovingRefletVector = XMFLOAT3(-1, 0, 0);
		if (Player.x < Center.x && 0 < ceta && ceta < extentceta)
			MovingRefletVector = XMFLOAT3(-1, 0, 0);
		if (Player.z < Center.z && extentceta < ceta && ceta < 90)
			MovingRefletVector = XMFLOAT3(0, 0, -1);
		if (Player.z < Center.z && -90 < ceta && ceta < -extentceta)
			MovingRefletVector = XMFLOAT3(0, 0, -1);
		if (Player.x > Center.x && -extentceta < ceta && ceta < 0)
			MovingRefletVector = XMFLOAT3(1, 0, 0);

		return MovingRefletVector;

	}

	static bool ProcessMapObjectColl(GameObject* mapObject, GameObject* player, XMFLOAT3& xmf3Shift)
	{
		XMMATRIX world = XMLoadFloat4x4(&mapObject->world_);
		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
		BoundingOrientedBox mLocalPlayerBounds;
		player->xmOOBB_.Transform(mLocalPlayerBounds, invWorld);

		if (mLocalPlayerBounds.Contains(dynamic_cast<MapObject*>(mapObject)->Bounds) != DirectX::DISJOINT)
		{

			XMFLOAT3 SlideVector = CollisionUtil::GetMapSlideVector(player, mapObject);
			float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
			if (cosCeta < 0)
			{
				auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
				XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

				player->world_._41 += sub_xmf3Shift.x, player->world_._42 += sub_xmf3Shift.y, player->world_._43 += sub_xmf3Shift.z;
				return true;
			}
		}
		return false;
	}
	static bool ProcessObjectColl(GameObject* object, GameObject* player, XMFLOAT3& xmf3Shift)
	{
		if (player->xmOOBB_.Contains(object->xmOOBB_))
		{
			XMFLOAT3 SlideVector = CollisionUtil::GetSlideVector(player, object);
			float cosCeta = Vector3::DotProduct(SlideVector, xmf3Shift);
			if (cosCeta < 0)
			{
				auto result = Vector3::MultiplyScalr(SlideVector, Vector3::DotProduct(xmf3Shift, SlideVector));
				XMFLOAT3 sub_xmf3Shift = Vector3::Subtract(xmf3Shift, result);

				if(ObjectType::TYPE_MONSTER == object->objectType_)
					sub_xmf3Shift.x *= 1.5, sub_xmf3Shift.y *= 1,5, sub_xmf3Shift.z *= 1.5;

				player->world_._41 += sub_xmf3Shift.x, player->world_._42 += sub_xmf3Shift.y, player->world_._43 += sub_xmf3Shift.z;
				return true;
			}
		}
		return false;
	}
};