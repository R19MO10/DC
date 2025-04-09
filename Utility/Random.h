/**
* @file Random.h
*/
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED
#include "../Math/MyMath.h"

/**
* ���������@�\���i�[���閼�O���
*/
namespace Random 
{
	/**
	* ����������������
	*
	* @param seed �����̎�
	*/
	void Initialize(int seed);


	/**
	* 0.0�`1.0�͈̔͂̈�l�������擾����
	*
	* @return ������������
	*/
	template <typename T>
	inline T Value() {
		return Range<T>(T(0), T(1));
	}

	/**
	* �w�肳�ꂽ�͈̔͂̈�l�������擾����
	*
	* @param min �����͈͂̍ŏ��l
	* @param max �����͈͂̍ő�l
	*
	* @return ������������
	*/
	template <typename T>
	T Range(T min, T max);

	template<>
	vec2Int Range<vec2Int>(vec2Int min, vec2Int max);
	template <>
	vec2 Range<vec2>(vec2 min, vec2 max);
	template <>
	vec3 Range<vec3>(vec3 min, vec3 max);
	template <>
	vec4 Range<vec4>(vec4 min, vec4 max);

} // namespace Random

#endif // RANDOM_H_INCLUDED