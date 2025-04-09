/**
* @file MyMath.h
*/
#ifndef MYMATH_H_INCLUDED
#define MYMATH_H_INCLUDED

// 角度関係
#include "Define/AngleMath.h"

// Vector型
#include "Define/Vec2Math.h"
#include "Define/Vec3Math.h"
#include "Define/Vec4Math.h"

// Vector型計算補助
#include "Define/Vec2Helper.h"
#include "Define/Vec3Helper.h"
#include "Define/Vec4Helper.h"

// VectorInt型
#include "Define/Vec2Int.h"

// Matrix型
#include "Define/Mat3Math.h"
#include "Define/Mat4Math.h"

// Matrix型計算補助
#include "Define/Mat3Helper.h"
#include "Define/Mat4Helper.h"

// Quaternion型
#include "Define/Quaternion.h"

// Quaternion型計算補助
#include "Define/QuaternionHelper.h"


// ２乗
inline float dot(float f) {
	return f * f;
}

// 線形補間
inline float lerp(const float& start, const float& end, float alpha) {
	return (1.0f - alpha) * start + alpha * end;
}

namespace Mat {
	
	// 平行移動行列を作成
	inline mat4 Translate(const vec3& v)
	{
		mat4 m(1);
		m[3][0] = v.x;
		m[3][1] = v.y;
		m[3][2] = v.z;
		return m;
	}

	// 拡大縮小行列を作成
	inline mat4 Scale(const vec3& v)
	{
		mat4 m(1);
		m[0][0] = v.x;
		m[1][1] = v.y;
		m[2][2] = v.z;
		return m;
	}
}

// 回転角度から回転行列を作成
inline mat3 RotationMatrix(const vec3& rotation)
{
	// X軸回転
	const float sinX = sin(rotation.x);
	const float cosX = cos(rotation.x);
	mat3 mx = {
		{ 1,		0,			0	  },
		{ 0,		cosX,		sinX  },
		{ 0,		-sinX,		cosX  }
	};

	// Y軸回転
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);
	mat3 my = {
		{ cosY,		0,			-sinY },
		{ 0,		1,			0	  },
		{ sinY,		0,			cosY  }
	};

	// Z軸回転
	const float sinZ = sin(rotation.z);
	const float cosZ = cos(rotation.z);
	mat3 mz = {
		{ cosZ,		sinZ,		0	  },
		{ -sinZ,	cosZ,		0	  },
		{ 0,		0,			1	  }
	};

	return my * mx * mz;
}

// 平行移動量、回転角度、拡大率から座標変換行列を作成
inline mat4 TransformMatrix(
	const vec3& position, const vec3& rotation, const vec3& scale)
{
	const mat3 m = RotationMatrix(rotation);
	return {
		{ m.data[0] * scale.x,	0 },
		{ m.data[1] * scale.y,	0 },
		{ m.data[2] * scale.z,	0 },
		{ position,				1 }
	};
}
inline mat4 TransformMatrix(
	const vec3& position, const mat3& rotation, const vec3& scale)
{
	return {
		{ rotation.data[0] * scale.x,	0 },
		{ rotation.data[1] * scale.y,	0 },
		{ rotation.data[2] * scale.z,	0 },
		{ position,						1 }
	};
}

// 4x4行列から拡大率を抽出する
inline vec3 ExtractScale(const mat4& transformMatrix)
{
	const float sx = length(vec3(transformMatrix[0]));
	const float sy = length(vec3(transformMatrix[1]));
	const float sz = length(vec3(transformMatrix[2]));

	return { sx, sy, sz };
}

// 回転行列からオイラー角を抽出する
inline vec3 ExtractRotation(const mat3& rotationMatrix)
{
	const float yaw =
		atan2(rotationMatrix.data[2][0], rotationMatrix.data[2][2]);

	const float pitch =
		atan2(-rotationMatrix.data[2][1],
		sqrt(rotationMatrix.data[0][1] * rotationMatrix.data[0][1] + 
			rotationMatrix.data[1][1] * rotationMatrix.data[1][1]));

	const float s = std::sin(yaw);
	const float c = std::cos(yaw);
	
	const float roll = 
		atan2(s * rotationMatrix.data[1][2] - c * rotationMatrix.data[1][0],
			c * rotationMatrix.data[0][0] - s * rotationMatrix.data[0][2]);

	return { pitch, yaw, roll };
}

/**
* 座標変換行列を平行移動、拡大率、回転行列の各成分に分解する
*
* @param[in]  transform 分解元の座標変換行列
* @param[out] translate 平行移動の格納先となる変数
* @param[out] rotation  回転率の格納先となる変数
* @param[out] scale     拡大率の格納先となる変数
*/
inline void Decompose(const mat4& transform,
	vec3& translate, vec3& rotation, vec3& scale)
{
	// 平行移動を分解
	translate = vec3(transform[3]);

	// 拡大率を分解
	scale = ExtractScale(transform);

	// 回転行列を分解
	mat3 r = mat3(transform);
	r[0] /= scale.x;
	r[1] /= scale.y;
	r[2] /= scale.z;
	rotation = ExtractRotation(r);
}
inline void Decompose(const mat4& transform,
	vec3& translate, mat3& rotation, vec3& scale)
{
	// 平行移動を分解
	translate = vec3(transform[3]);

	// 拡大率を分解
	scale = ExtractScale(transform);

	// 回転行列を分解
	rotation = mat3(transform);
	rotation[0] /= scale.x;
	rotation[1] /= scale.y;
	rotation[2] /= scale.z;
}

/**
* 注視点を計算する
* 
* @param 視点の座標
* @param 視点の回転
* 
* @return 注視点座標
*/
inline vec3 CalcTargetPosition(const vec3& position, const vec3& rotation) {
	// カメラの方向ベクトルを生成する（例として簡略化）
	vec3 direction(
		cos(rotation.x) * sin(rotation.y),
		sin(rotation.x),
		cos(rotation.x) * cos(rotation.y)
	);

	// カメラの注視点は位置ベクトルに方向ベクトルを加算したものとして計算される
	vec3 target = position + direction;

	return target;
}

/**
* ビュー行列を作成する
*
* @param eye    視点の座標
* @param target 注視点の座標
* @param up     視点の上方向を指す仮のベクトル
*
* @return eye, target, upから作成したビュー行列
*/
inline mat4 LookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	// ワールド座標系における視点座標系のXYZ軸の向きを計算
	const vec3 axisZ = normalize(eye - target);
	const vec3 axisX = normalize(cross(up, axisZ));
	const vec3 axisY = normalize(cross(axisZ, axisX));

	// 座標を軸ベクトルに射影するように行列を設定
	mat4 m;
	m[0] = { axisX.x, axisY.x, axisZ.x, 0 };
	m[1] = { axisX.y, axisY.y, axisZ.y, 0 };
	m[2] = { axisX.z, axisY.z, axisZ.z, 0 };

	// eyeが原点となるように、eyeを各軸に射影して平行移動量を計算
	m[3] = { -dot(axisX, eye), -dot(axisY, eye), -dot(axisZ, eye), 1 };

	return m;
}


/**
* 平行投影行列
*
* @param left	描画範囲の左端までの距離
* @param right	描画範囲の右端までの距離
* @param bottom	描画範囲の下端までの距離
* @param top	描画範囲の上端までの距離
* @param zNear	描画範囲に含まれる最小Z座標
* @param zFar	描画範囲に含まれる最大Z座標
*/
inline mat4 Orthogonal(float left, float right, float bottom, float top, float zNear, float zFar)
{

	 // 描画範囲のサイズと中心座標を計算
	const vec3 size = { right - left, top - bottom, zFar - zNear };
	const vec3 center = { right + left, top + bottom, zFar + zNear };

	// 描画範囲内の座標が-1～+1に変換されるように、拡大率と平行移動を設定
	/* 座標に「2 / 描画範囲」を掛けてクリップ座標系へ変換（クリップ座標系はxYZが±1の座標系なため）*/
	mat4 m;
	m[0] = { 2 / size.x,    0,				0,				0 };
	m[1] = { 0,				2 / size.y,     0,				0 };
	m[2] = { 0,				0,				-2 / size.z,	0 };	/*Z軸を反転（OpenGLの視点座標系は右手系、クリップ座標系は左手系なため）*/
	m[3] = vec4(-center / size, 1);

	return m;
}

/**
* 透視投影行列
*
* @param fovy_rad		垂直視野角(ラジアン)
* @param aspectRatio	垂直視野角に対する水平視野角の比率
* @param zNear			描画範囲に含まれる最小Z座標
* @param zFar			描画範囲に含まれる最大Z座標
*/
inline mat4 Perspective(float fovy_rad, float aspectRatio, float zNear, float zFar)
{
	const float tanHalfFovy = tan(fovy_rad * 0.5f);

	mat4 Result(0);
	Result[0][0] = 1 / (aspectRatio * tanHalfFovy);
	Result[1][1] = 1 / (tanHalfFovy);
	Result[2][2] = -(zFar + zNear) / (zFar - zNear);
	Result[2][3] = -1;
	Result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);

	return Result;
}

#endif // MYMATH_H_INCLUDED