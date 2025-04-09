/**
* @file Vec2Int.cpp
*/
#include "Vec2Int.h"

#include "Vec2Math.h"


// vec2‚©‚çvecInt2‚Ö‚Ì•ÏŠ·
vec2Int::vec2Int(const vec2& v)
	: x(static_cast<int>(v.x)),
	  y(static_cast<int>(v.y))
{}