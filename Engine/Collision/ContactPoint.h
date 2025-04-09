/**
* @file ContactPoint.cpp
*/
#ifndef CONTACTPOINT_H_INCLUDED
#define CONTACTPOINT_H_INCLUDED
#include "../../Math/MyMath.h"

/**
* �Փˏ���ێ�����\����
*/
struct ContactPoint
{
	vec3 position;		// �Փˍ��W
	vec3 penetration;   // �ђʃx�N�g��
};

#endif // CONTACTPOINT_H_INCLUDED