/**
* @file ContactPoint.cpp
*/
#ifndef CONTACTPOINT_H_INCLUDED
#define CONTACTPOINT_H_INCLUDED
#include "../../Math/MyMath.h"

/**
* 衝突情報を保持する構造体
*/
struct ContactPoint
{
	vec3 position;		// 衝突座標
	vec3 penetration;   // 貫通ベクトル
};

#endif // CONTACTPOINT_H_INCLUDED