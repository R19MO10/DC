/**
* @file GltfAnimation.cpp
*/
#include "GltfAnimation.h"

#include "GltfFile.h"
#include <algorithm>

namespace {

	/**
	* アニメーション計算用の中間データ型
	*/
	struct NodeMatrix
	{
		MyFName name;	// 名前
		mat4 m = mat4(1);	// 姿勢行列
		bool isCalculated = false;  // 計算済みフラグ
	};
	using NodeMatrices = std::vector<NodeMatrix>;
	
	/**
	* ノードのグローバル姿勢行列を計算する
	*/
	const mat4& CalcGlobalNodeMatrix(const std::vector<GltfNode>& nodes,
		const GltfNode& node, NodeMatrices& matrices)
	{
		const intptr_t currentNodeId = &node - &nodes[0];
		NodeMatrix& nodeMatrix = matrices[currentNodeId];

		// 「計算済み」の場合は自分の姿勢行列を返す
		if (nodeMatrix.isCalculated) {
			return nodeMatrix.m;
		}

		// 「計算済みでない」場合、親の姿勢行列を合成する
		if (node.parent) {
			// 親の行列を取得(再帰呼び出し)
			const mat4& matParent =
				CalcGlobalNodeMatrix(nodes, *node.parent, matrices);

			// 親の姿勢行列を合成
			nodeMatrix.m = matParent * nodeMatrix.m;
		}

		// 「計算済み」にする
		nodeMatrix.isCalculated = true;

		// 自分の姿勢行列を返す
		return nodeMatrix.m;
	}

	/**
	* チャネル上の指定した時刻の値を求める
	*
	* @param channel 対象のチャネル
	* @param time    値を求める時刻
	*
	* @return 時刻に対応する値
	*/
	template<typename T>
	T Interpolate(const GltfChannel<T>& channel, float time)
	{
		// time以上の時刻を持つ、最初のキーフレームを検索
		const auto& curOrOver = std::lower_bound(
			channel.keyframes.begin(), channel.keyframes.end(), time,
			[](const GltfKeyframe<T>& keyframe, float time) {
				return keyframe.time < time; });

		// timeが先頭キーフレームの時刻と等しい場合、先頭キーフレームの値を返す
		if (curOrOver == channel.keyframes.begin()) {
			return channel.keyframes.front().value;
		}

		// timeが末尾キーフレームの時刻より大きい場合、末尾キーフレームの値を返す
		if (curOrOver == channel.keyframes.end()) {
			return channel.keyframes.back().value;
		}

		// timeが先頭と末尾の間だった場合
		// キーフレーム間の時間におけるtimeの比率を計算し、比率によって補間した値を返す
		const auto& prev = curOrOver - 1; // ひとつ前の(time未満の時刻を持つ)キーフレーム
		const float frameTime = curOrOver->time - prev->time;
		const float t = std::clamp((time - prev->time) / frameTime, 0.0f, 1.0f);

		// 注意: 今は常に(球状)線形補間をしているが、本来は補間方法によって処理を分けるべき
		if constexpr (std::is_same_v<T, Quaternion>) {
			return slerp(prev->value, curOrOver->value, t);
		}
		else {
			return prev->value * (1 - t) + curOrOver->value * t;
		}
	}
}	// unnamed namespace


/**
* アニメーションを適用した姿勢行列を計算する
*
* @param file             meshNodeを所有するファイルオブジェクト
* @param meshNode         メッシュを持つノード
* @param animation        計算の元になるアニメーション
* @param time             アニメーションの再生位置
*
* @return アニメーションを適用した姿勢行列の配列
*/
GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
	const GltfNode* meshNode, const GltfAnimation* animation, float time)
{
	GltfAnimationMatrices matBones;
	if (!file || !meshNode) {
		return matBones;
	}

	// アニメーションが設定されていない場合...
	if (!animation) {
		// ノードのグローバル座標変換行列を使う
		size_t size = 1;
		if (meshNode->skin >= 0) {
			size = file->skins[meshNode->skin].joints.size();
		}
		matBones.resize(size, { meshNode->name, meshNode->matGlobal });

		return matBones;
	}

	// アニメーションが設定されている場合...
	NodeMatrices matrices;
	const auto& nodes = file->nodes;
	matrices.resize(nodes.size());


	// アニメーションしないノードのローカル姿勢行列を設定
	for (const auto e : animation->staticNodes) {
		matrices[e].m = nodes[e].matLocal;

		// 自身の親の名前をコピー
		matrices[e].name = nodes[e].name;
	}

	// アニメーションするノードのローカル姿勢行列を計算
	// (拡大縮小→回転→平行移動の順で適用)
	for (const auto& e : animation->translations) {
		const vec3 translation = Interpolate(e, time);
		matrices[e.targetNodeId].m = Mat::Translate(translation);

		// 名前をコピー
		matrices[e.targetNodeId].name = nodes[e.targetNodeId].name;
	}
	for (const auto& e : animation->rotations) {
		const Quaternion rotation = Interpolate(e, time);
		matrices[e.targetNodeId].m *= mat4(rotation);
	}
	for (const auto& e : animation->scales) {
		const vec3 scale = Interpolate(e, time);
		matrices[e.targetNodeId].m *= Mat::Scale(scale);
	}

	// アニメーションを適用したグローバル姿勢行列を計算（ローカル座標変換行列→バインドポーズ行列）
	if (meshNode->skin >= 0) {
		for (const auto& joint : file->skins[meshNode->skin].joints) {
			CalcGlobalNodeMatrix(nodes, nodes[joint.nodeId], matrices);
		}
	}
	else {
		// ジョイントがないのでメッシュノードだけ計算
		CalcGlobalNodeMatrix(nodes, *meshNode, matrices);
	}

	// 逆バインドポーズ行列を合成
	if (meshNode->skin >= 0) {
		// glTFのjointsキーにはノード番号が格納されている
		// しかし、頂点データのJOINTS_n属性には「joints配列のインデックス」が格納されている
		// 実際に姿勢行列を使うのは頂点データなので、姿勢行列をjoints配列の順番で格納する
		const auto& joints = file->skins[meshNode->skin].joints;
		matBones.resize(joints.size());
		for (size_t i = 0; i < joints.size(); ++i) {
			const auto& joint = joints[i];

			matBones[i].name = matrices[joint.nodeId].name;
			matBones[i].m = matrices[joint.nodeId].m * joint.matInverseBindPose;
		}
	}
	else {
		// ジョイントがないので逆バインドポーズ行列も存在しない
		const size_t nodeId = meshNode - &nodes[0];
		matBones.resize(1, { matrices[nodeId].name, matrices[nodeId].m });
	}

	return matBones;
}
