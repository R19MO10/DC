/**
* @ file GltfAnination.h
*/
#ifndef GLTFANIMATION_H_INCLUDED
#define GLTFANIMATION_H_INCLUDED
#include "../../Utility/MyFName.h"
#include "../../Math/MyMath.h"
#include <vector>
#include <string>
#include <memory>

// 先行宣言
struct GltfFile;
using  GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* スキン
*/
struct GltfSkin
{
	MyFName name; // スキン名

	// ジョイント(ボーン)データ
	struct Joint {
		int nodeId;
		mat4 matInverseBindPose;
	};
	std::vector<Joint> joints; // ジョイント配列
};

/**
* ノード
*/
struct GltfNode
{
	MyFName name;                // ノード名
	int mesh = -1;                   // メッシュ番号
	int skin = -1;                   // スキン番号
	GltfNode* parent = nullptr;      // 親ノード
	std::vector<GltfNode*> children; // 子ノード配列
	mat4 matLocal = mat4(1);  // ローカル行列
	mat4 matGlobal = mat4(1); // グローバル行列
};

/**
* シーン
*/
struct GltfScene
{
	std::vector<const GltfNode*> nodes;     // ノード配列
	std::vector<const GltfNode*> meshNodes; // メッシュを持つノードのみの配列
};

/**
* アニメーションの補間方法
*/
enum class GltfInterpolation
{
	step,        // 補間なし
	linear,      // 線形補間
	cubicSpline, // 3次スプライン補間
};

/**
* アニメーションのキーフレーム
*/
template<typename T> struct GltfKeyframe
{
	float time; // 時刻
	T value;    // 適用する値
};

/**
* アニメーションのチャネル
*/
template<typename T> struct GltfChannel
{
	int targetNodeId;                       // 値を適用するノードID
	GltfInterpolation interpolation;        // 補間方法
	std::vector<GltfKeyframe<T>> keyframes; // キーフレーム配列
};

/**
* アニメーション
*/
struct GltfAnimation
{
	MyFName name; // アニメーション名
	std::vector<GltfChannel<vec3>> translations; // 平行移動チャネルの配列
	std::vector<GltfChannel<Quaternion>> rotations;    // 回転チャネルの配列
	std::vector<GltfChannel<vec3>> scales;       // 拡大縮小チャネルの配列
	std::vector<int> staticNodes; // アニメーションしないノードIDの配列
	float totalTime_s = 0;
};
using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;

// アニメーション用座標変換行列の配列
struct GltfAnimationMat
{
	MyFName name;	// ボーン名前
	mat4 m;
};
using GltfAnimationMatrices = std::vector<GltfAnimationMat>;

GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
	const GltfNode* meshNode, const GltfAnimation* animation, float time);

#endif // GLTFANIMATION_H_INCLUDED