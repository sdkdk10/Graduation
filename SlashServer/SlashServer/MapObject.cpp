#include "MapObject.h"
#include "Defines.h"

MapObject::MapObject()
{
}

MapObject::~MapObject()
{
}

void MapObject::Initialize()
{
	GameObject::Initialize();
	objectType_ = TYPE_MAPOBJECT;
}
