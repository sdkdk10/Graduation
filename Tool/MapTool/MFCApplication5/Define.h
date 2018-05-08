#pragma once
#include "Macro.h"
#include "Function.h"
#include "Functor.h"

enum HEAP_TYPE
{
	HEAP_DEFAULT,
	HEAP_INSTANCING,
	HEAP_END
};

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20