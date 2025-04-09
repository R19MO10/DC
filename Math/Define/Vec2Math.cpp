/**
* @ Vec2Math.cpp
*/
#include "Vec2Math.h"

#include "Vec2Int.h"
#include "Vec3Math.h"

// vec2Int‚©‚çvec2‚Ö‚Ì•ÏŠ·
vec2::vec2(const vec2Int& v)
	: x(static_cast<float>(v.x)),
	  y(static_cast<float>(v.y))
{}

// vec3‚©‚çvec2‚Ö‚Ì•ÏŠ·
vec2::vec2(const vec3& v)
	: x(v.x), y(v.y)
{}