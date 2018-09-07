#pragma once
#include "GameObject.h"

class MapObject : public GameObject
{
public:
	explicit MapObject();
	virtual ~MapObject();

public:
	virtual void Initialize();

public:
	BoundingBox Bounds;
};