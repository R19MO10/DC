/**
* @file Collision.cpp
*/
#include "Collision.h"

#include <algorithm>
#include <cmath>
#include <vector>

/**
* 衝突判定用の構造体や関数を格納する名前空間
*/
namespace Calc {

	// 無名名前空間
	namespace {

		/**
		* OBBの頂点座標を取得する
		*
		* @param box 頂点座標を求めるOBB
		*
		* @return boxの頂点座標配列
		*
		*   5----4
		*  /|   /|
		* 1----0 |
		* | 7--|-6
		* |/   |/
		* 3----2
		*/
		std::vector<vec3> GetVertices(const Box& box)
		{
			const vec3 ax = box.axis[0] * box.scale.x;
			const vec3 ay = box.axis[1] * box.scale.y;
			const vec3 az = box.axis[2] * box.scale.z;
			return {
				box.position + ax + ay + az, // 0
				box.position - ax + ay + az, // 1
				box.position + ax - ay + az, // 2
				box.position - ax - ay + az, // 3
				box.position + ax + ay - az, // 4
				box.position - ax + ay - az, // 5
				box.position + ax - ay - az, // 6
				box.position - ax - ay - az  // 7
			};
		}

		/**
		* OBBの辺を取得する
		*
		* @param vertices 頂点座標を求めるOBB
		*
		* @return boxの辺の配列
		*
		*   5----4
		*  /|   /|
		* 1----0 |
		* | 7--|-6
		* |/   |/
		* 3----2
		*/
		std::vector<LineSegment> GetEdges(const vec3* vertices)
		{
			return {
				// X軸方向
				{ vertices[0], vertices[1] },
				{ vertices[2], vertices[3] },
				{ vertices[4], vertices[5] },
				{ vertices[6], vertices[7] },

				// Y軸方向
				{ vertices[0], vertices[2] },
				{ vertices[1], vertices[3] },
				{ vertices[4], vertices[6] },
				{ vertices[5], vertices[7] },

				// Z軸方向
				{ vertices[0], vertices[4] },
				{ vertices[1], vertices[5] },
				{ vertices[2], vertices[6] },
				{ vertices[3], vertices[7] },
			};
		}

		/**
		* OBBの面を取得する
		*
		* @param box 面を求めるOBB
		*
		* @return boxの面の配列
		*/
		std::vector<Plane> GetPlanes(const Box& box)
		{
			const vec3 ax = box.axis[0] * box.scale.x;
			const vec3 ay = box.axis[1] * box.scale.y;
			const vec3 az = box.axis[2] * box.scale.z;

			return {	// 平面上の座標を面法線に射影
				{ box.axis[0], dot(box.axis[0], box.position + ax) }, // +x
				{-box.axis[0],-dot(box.axis[0], box.position - ax) }, // -x
				{ box.axis[1], dot(box.axis[1], box.position + ay) }, // +y
				{-box.axis[1],-dot(box.axis[1], box.position - ay) }, // -y
				{ box.axis[2], dot(box.axis[2], box.position + az) }, // +z
				{-box.axis[2],-dot(box.axis[2], box.position - az) }, // -z
			};
		}

		/**
		* OBBの頂点を軸に射影し、最大値と最小値を求める
		*
		* @param vertices OBBの頂点配列
		* @param axis     射影先の軸ベクトル
		* @param min      最小値を格納する変数
		* @param max      最大値を格納する変数
		*/
		void ProjectBoxToAxis(const vec3* vertices, const vec3& axis,
			float& min, float& max)
		{
			min = max = dot(axis, vertices[0]);

			for (int i = 0; i < 8; ++i) {
				float f = dot(axis, vertices[i]);	// 座標をベクトルに射影する
				min = std::min(min, f);
				max = std::max(max, f);
			}
		}

		/**
		* 2つのOBBを軸ベクトルに射影し、交差している距離を求める
		*
		* @param verticesA  OBBその1の頂点配列
		* @param verticesB  OBBその2の頂点配列
		* @param axis       射影先の軸ベクトル
		* @param shouldFlip 衝突法線の反転の有無
		*
		* @return 交差している距離(計算の都合でマイナス値になっていることに注意)
		*/
		float GetIntersectionLength(const vec3* verticesA, const vec3* verticesB,
			const vec3& axis, bool& shouldFlip)
		{
			// 2つのOBBを軸に射影
			float minA, maxA, minB, maxB;
			ProjectBoxToAxis(verticesA, axis, minA, maxA);
			ProjectBoxToAxis(verticesB, axis, minB, maxB);

			// 射影した範囲が交差していないなら0を返す
			if (minB > maxA && minA > maxB) {
				return 0;
			}

			// 衝突法線は「OBBその1」が左、「OBBその2」が右にあると想定しているので、
			// 順序が逆になっている場合、衝突法施の向きを反転する必要がある、
			// ということを、呼び出し元に伝える
			shouldFlip = minB < minA;

			// 交差している距離を計算
			float lenA = maxA - minA;
			float lenB = maxB - minB;
			float min = std::min(minA, minB);
			float max = std::max(maxA, maxB);
			float length = max - min;
			return length - lenA - lenB;
		}

		/**
		* 点がOBB内にあるかどうかを調べる
		*
		* @param point 調べる点の座標
		* @param box   調べるOBB
		*
		* @retval true  pointはboxの内側にある
		* @retval false pointはboxの外側にある
		*/
		bool IsPointInsideBox(const vec3& point, const Box& box)
		{
			const vec3 v = point - box.position;	// 点に向かうベクトルを計算
			for (int i = 0; i < 3; ++i) {
				const float d = dot(v, box.axis[i]);	// 軸に射影
				if (d < -box.scale[i] || d > box.scale[i]) {
					return false; // 点はOBBの外側に有る
				}
			}
			return true; // 点はOBBの内側にある
		}

		/**
		* 線分と面の交点を求める
		*
		* @param seg   線分
		* @param plane 面
		* @param p     交点を格納する変数
		*
		* @retval true  交点は線分上にある
		* @retval false 交点は線分の外にある
		*/
		bool ClipLineSegment(const LineSegment& seg, const Plane& plane, vec3& p)
		{
			// 辺ベクトルを面法線に射影
			const vec3 ab = seg.end - seg.start;
			float n = dot(plane.normal, ab);
			if (abs(n) < 0.0001f) {
				return false; // 平行なので交差なし
			}

			// 交点までの距離を求める
			float t = plane.d - dot(plane.normal, seg.start);
			if (t < 0 || t > n) {
				return false; // 交点が線分の外にあるので交差なし
			}

			// 交点座標を計算
			t /= n; // ワールド座標系の長さからベクトルabに対する比率に変換
			p = seg.start + ab * t;	// 始点から交点までのベクトル
			return true;
		}

		/**
		* OBBの全ての辺について、もう一つのOBBの面との交点を求める
		*
		* @param vertices OBBその1の頂点配列
		* @param box      OBBその2
		*
		* @return edgesとboxの交点
		*/
		std::vector<vec3> ClipEdges(const std::vector<vec3>& vertices, const Box& box)
		{
			// 辺と面を取得
			const auto edges = GetEdges(vertices.data());
			const auto planes = GetPlanes(box);

			// 交点用のメモリを予約
			// 2つのOBBが相当深く交差していない限り、交点の数は辺の数以下になるはず
			std::vector<vec3> result;
			result.reserve(edges.size());

			// 辺と面の交点を求める
			/*最適化：例えば、衝突法線と逆向きの面と辺は無視できるでしょう。*/
			for (const auto& plane : planes) {
				for (const auto& edge : edges) {
					vec3 p;
					if (ClipLineSegment(edge, plane, p)) {
						// 交点がOBBその2の内側にあるなら、有効な交点として記録
						if (IsPointInsideBox(p, box)) {
							result.push_back(p);
						}
					}
				} // for edges
			} // for planes
			return result;
		}
	} // unnamed namespace

	/**
	* AABBから点への最近接点
	*
	* @param aabb  判定対象のAABB
	* @param point 判定対象の点
	*
	* @return AABBの中で最も点に近い座標
	*/
	vec3 ClosestPoint(const AABB& aabb, const vec3& point)
	{
		vec3 result;
		for (int i = 0; i < 3; ++i) {
			const float min = aabb.min[i] + aabb.position[i];
			const float max = aabb.max[i] + aabb.position[i];
			result[i] = std::clamp(point[i], min, max);
		}
		return result;
	}

	/**
	* OBBから点への最近接点
	*
	* @param box   判定対象のOBB
	* @param point 判定対象の点
	*
	* @return 長方形の中で最も点に近い座標
	*/
	vec3 ClosestPoint(const Box& box, const vec3& point)
	{
		// OBBから点に向かうベクトル
		const vec3 v = point - box.position;

		vec3 result = box.position;
		for (int i = 0; i < 3; ++i) {
			// ベクトルをOBBの軸に射影
			float d = dot(v, box.axis[i]);

			// 射影で得られた値をOBBの範囲内に制限
			if (box.scale[i] > 0) {
				d = std::clamp(d, -box.scale[i], box.scale[i]);
			}
			else {
				d = std::clamp(d, box.scale[i], -box.scale[i]);
			}

			// 最近接点を更新
			result += d * box.axis[i];
		}
		return result;
	}


	/**
	* 線分上の最近接点を求める
	*
	* @param seg   最近接点を求める線分
	* @param point 最近接点を求める座標
	*
	* @return pointからsegへの最近接点
	*/
	vec3 ClosestPoint(const LineSegment& seg, const vec3& point)
	{
		// 点を線分に射影
		const vec3 d = seg.end - seg.start;
		const float t = dot(point - seg.start, d);

		// 線分の範囲に制限
		if (t <= 0) {
			return seg.start;
		}
		const float d2 = dot(d, d); // 線分の長さの2乗
		if (t >= d2) {
			return seg.end;
		}

		// 最近接点を計算して返す
		return seg.start + d * (t / d2);
	}

	/**
	* 2つの線分の最近接点を求める
	*
	* @param segA  最近接点を求める線分その1
	* @param segB  最近接点を求める線分その2
	* @param denom segA上の最近接点の比率
	*
	* @return segA上の最近接点とsegB上の最近接点
	*/
	LineSegment ClosestPoint(const LineSegment& segA, const LineSegment& segB)
	{
		// 線分Aの方向ベクトルと長さを計算
		const vec3 directionA = segA.end - segA.start;
		const float lengthSquaredA = dot(directionA, directionA);	// 線分Aの長さの2乗

		// 線分Bの方向ベクトルと長さを計算
		const vec3 directionB = segB.end - segB.start;
		const float lengthSquaredB = dot(directionB, directionB); // 線分Bの長さの2乗

		// 線分AとBの始点間のベクトルを計算
		const vec3 relativePosition = segA.start - segB.start;

		// 線分AとBの方向ベクトルの内積を計算
		const float dotAB = dot(directionA, directionB);
		const float dotARelative = dot(directionA, relativePosition);
		const float dotBRelative = dot(directionB, relativePosition);


		// 線分AとBの両方の長さが0の特殊ケースを処理（長さが0の線分を「点」とみなして計算）
		if (lengthSquaredA < 0.0001f && lengthSquaredB < 0.0001f) {
			/* 線分AとBがともに点の場合、その点同士を返す */
			return { segA.start, segB.start };
		}


		// 線分Aの長さが0
		if (lengthSquaredA < 0.0001f) {
			if (dotBRelative <= 0) {
				/* 線分Aの始点から線分Bへの最短距離が負の場合、
				   線分Aの始点を返す */
				return { segA.start, segB.start };
			}
			else if (dotBRelative >= lengthSquaredB) {
				/* 線分Aの始点から線分Bへの最短距離が線分Bの長さを超える場合、
				　 線分Aの始点から線分Bの終点を返す */
				return { segA.start, segB.end };
			}

			/* 線分Aの始点から線分Bへの最短距離が線分Bの長さ内に収まる場合、
			   線分Aの始点から線分B上の最近接点を返す */
			return { segA.start, segB.start + directionB * (dotBRelative / lengthSquaredB) };
		}


		// 線分Bの長さが0
		if (lengthSquaredB < 0.0001f) {
			if (-dotARelative <= 0) {
				/* 線分Bの始点から線分Aへの最短距離が負の場合、
				   線分Aの始点を返す */
				return { segA.start, segB.start };
			}
			else if (-dotARelative >= lengthSquaredA) {
				/* 線分Bの始点から線分Aへの最短距離が線分Aの長さを超える場合、
				   線分Aの終点から線分Bの始点を返す */
				return { segA.end, segB.start };
			}

			/* 線分Bの始点から線分Aへの最短距離が線分Aの長さ内に収まる場合、
			   線分A上の最近接点から線分Bの始点を返す */
			return { segA.start + directionA * (-dotARelative / lengthSquaredA), segB.start };
		}


		// 線分の長さが0より大きいと分かったら、線分が平行かどうかをチェックする
		// 線分が平行な場合、sを求める式の分母が0になり計算できないため
		const float denom = lengthSquaredA * lengthSquaredB - dotAB * dotAB;


		// 線分が平行でない場合
		if (denom) {
			// 線分Aの最近接点を求め、線分Aの範囲に制限
			float s = std::clamp((dotBRelative * dotAB - dotARelative * lengthSquaredB) / denom, 0.0f, 1.0f);

			// 線分Bの最近接点を求める
			float t = dotAB * s + dotBRelative;

			// tが線分Bの範囲外の場合の処理
			LineSegment result;
			if (t < 0) {
				// tが負の場合、線分Bの始点が最近接点になる
				result.end = segB.start;
				s = std::clamp(-dotARelative / lengthSquaredA, 0.0f, 1.0f);	// 線分Aの範囲内に制限
			}
			else if (t > lengthSquaredB) {
				// tが線分Bの長さを超える場合、線分Bの終点が最近接点になる
				result.end = segB.end;
				s = std::clamp((dotAB - dotARelative) / lengthSquaredA, 0.0f, 1.0f);	// 線分Aの範囲内に制限
			}
			else {
				// tを線分Bの範囲内に制限し、その位置における線分B上の点を計算し、最近接点として設定
				result.end = segB.start + directionB * (t / lengthSquaredB);
			}

			// 線分A上の最近接点を計算し、求めた最近接点を返す
			result.start = segA.start + directionA * s;
			return result;

		}


		// 線分が平行な場合、線分の重複する範囲の中点を選択

		// 線分Bを線分Aに射影
		float u = dot(directionA, segB.start - segA.start);
		float v = dot(directionA, segB.end - segA.start);

		// 射影した座標を線分Aの範囲に制限
		u = std::clamp(u, 0.0f, lengthSquaredA);
		v = std::clamp(v, 0.0f, lengthSquaredA);

		// uとvの中点が線分Aの最近接点になる
		const float s = (u + v) / lengthSquaredA * 0.5f;
		LineSegment result;
		result.start = segA.start + directionA * s;

		// 線分Aの最近接点を線分Bに射影
		const float t = dot(directionB, result.start - segB.start);

		// 射影した座標を線分Bの範囲に制限
		if (t <= 0) {
			/* tが負の場合、線分Aの最近接点が線分Bの始点の逆方向に位置しているため、
			   最近接点を線分Bの始点に設定 */
			result.end = segB.start;
		}
		else if (t >= lengthSquaredB) {
			/* tが線分Bの長さ以上の場合、線分Aの最近接点が線分Bの終点の逆方向に位置しているため、
			   最近接点を線分Bの終点に設定 */
			result.end = segB.end;
		}
		else {
			// tを線分Bの範囲内に制限し、その位置における線分B上の点を計算し、最近接点として設定
			result.end = segB.start + directionB * (t / lengthSquaredB);
		}

		return result;
	}

	/**
	* 3D長方形と線分の最近接点を求める
	*
	* @param rect 最近接点を求める3D長方形
	* @param seg  最近接点を求める線分
	*
	* @return seg上の最近接点とrect上の最近接点
	*/
	LineSegment ClosestPoint(const Rectangle& rect, const LineSegment& seg)
	{
		// 線分の始点を面に射影
		const vec3 ca = seg.start - rect.center;
		const float a0 = dot(ca, rect.axis[0]);
		const float a1 = dot(ca, rect.axis[1]);

		// 長方形と線分の平行性を調べる
		const vec3 ab = seg.end - seg.start;
		const float n = dot(rect.normal, ab);
		if (abs(n) <= 0.0001f) {
			// 平行な場合、線分が長方形の上に乗っているかどうかを調べる
			float tmin = -FLT_MAX;
			float tmax = FLT_MAX;
			const float list[] = { a0, a1 };
			for (int i = 0; i < 2; ++i) {
				// 線分とaxis[i]との平行性を調べる
				const float d = dot(ab, rect.axis[i]);
				if (abs(d) <= 0.0001f) {
					continue; // 平行なので交点がない
				}
				// 長方形中心からの距離を線分の比率として表す
				float t0 = (-rect.scale[i] - list[i]) / d;
				float t1 = (rect.scale[i] - list[i]) / d;
				if (t0 > t1) {
					std::swap(t0, t1);
				}
				// 長方形と交差している範囲を計算
				tmin = std::max(t0, tmin);
				tmax = std::min(t1, tmax);
			}

			// 線分の始点と終点の範囲に制限する
			tmin = std::max(0.0f, tmin);
			tmax = std::min(1.0f, tmax);

			// 線分が長方形に重なっている場合
			// 重複範囲の中心を最近接点とする
			if (tmin <= tmax) {
				const vec3 p = seg.start + ab * ((tmin + tmax) * 0.5f);
				const float d = dot(rect.normal, p - rect.center);
				return { p, p - rect.normal * d };
			}
		} // if (abs(n) <= 0.0001f)

		// 線分が面と交差しているかチェック
		// 交差している場合、最近接点は交点になる
		{
			const float d = dot(rect.normal, rect.center);	// 面法線
			const float t = (d - dot(rect.normal, seg.start)) / n;	// 長方形から線分の始点までの距離
			/* 長方形の中心と線分の始点を面法線に対して射影し
			   線分を射影した長さnで割る
			   （線分の長さnで割ることで、tは線分に対する比率になる(tが0なら交点は始点 / tが1なら交点は終点)）*/

			if (0 <= t && t <= 1) {
				// 交点が長方形の範囲内にあるか調べる

				const vec3 p = seg.start + ab * t;
				const vec3 v = p - rect.center;	// 長方形の中心から交点へ向かうベクトル

				// 長方形の2つの軸ベクトルに射影
				const float dist1 = dot(v, rect.axis[0]);
				if (abs(dist1) <= rect.scale[0]) {
					const float dist2 = dot(v, rect.axis[1]);
					if (abs(dist2) <= rect.scale[1]) {
						// 交点が長方形内に有ることが判明したため、その座標を最近接点として返す
						return { p, p };
					}
				}
			} // if (0 <= t && t <= n)
		}


		// 平行でなく、交差もしていない場合

		// 線分の終点を面に射影
		const vec3 cb = seg.end - rect.center;
		const float b0 = dot(cb, rect.axis[0]);
		const float b1 = dot(cb, rect.axis[1]);

		// 線分の始点および終点の最近接点を求め
		// 長方形の範囲に含まれる点のうちより近いほうを「仮の最近接点」とする
		float distance = FLT_MAX;
		LineSegment result;

		// 始点
		if (abs(a0) <= rect.scale[0] && abs(a1) <= rect.scale[1]) {	// 長方形の範囲チェック
			distance = dot(rect.normal, ca);
			distance *= distance; // 比較のために2乗する
			result = { seg.start, rect.center + rect.axis[0] * a0 + rect.axis[1] * a1 };
		}

		// 終点
		if (abs(b0) <= rect.scale[0] && abs(b1) <= rect.scale[1]) {	// 長方形の範囲チェック
			float tmp = dot(rect.normal, cb);
			tmp *= tmp; // 比較のために2乗する
			if (tmp < distance) {
				distance = tmp;
				result = { seg.end, rect.center + rect.axis[0] * b0 + rect.axis[1] * b1 };
			}
		}


		// 長方形の4辺について、最近接点を計算
		const vec3 v1 = rect.axis[0] * rect.scale[0];
		const vec3 v2 = rect.axis[1] * rect.scale[1];
		const vec3 v[] = {
			rect.center + v1 + v2,
			rect.center + v1 - v2,
			rect.center - v1 - v2,
			rect.center - v1 + v2
		};

		for (int i = 0; i < 4; ++i) {
			const auto s = ClosestPoint(seg, LineSegment{ v[i], v[(i + 1) % 4] });
			const vec3 ab = s.end - s.start;
			const float distanceEdge = dot(ab, ab); // 長さの2乗
			if (distanceEdge < distance) { // 2乗同士の比較
				distance = distanceEdge;
				result = s;
			}
		}

		return result;
	}
}

/**
* AABBとAABBの衝突判定
*
* @param aabbA  衝突判定を行うAABBその1
* @param aabbB  衝突判定を行うAABBその2
* @param cp  衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const AABB& aabbA, const AABB& aabbB, ContactPoint& cp)
{
	const vec3 abA_min = aabbA.min + aabbA.position;
	const vec3 abA_max = aabbA.max + aabbA.position;

	const vec3 abB_min = aabbB.min + aabbB.position;
	const vec3 abB_max = aabbB.max + aabbB.position;

	// aの左側面がbの右側面より右にあるなら、交差していない
	const float dx0 = abB_max.x - abA_min.x;
	if (dx0 <= 0) {
		return false;
	}

	// aの右側面がbの左側面より左にあるなら、交差していない
	const float dx1 = abA_max.x - abB_min.x;
	if (dx1 <= 0) {
		return false;
	}
	// aの下面がbの上面より上にあるなら、交差していない
	const float dy0 = abB_max.y - abA_min.y;
	if (dy0 <= 0) {
		return false;
	}
	// aの上面がbの下面より下にあるなら、交差していない
	const float dy1 = abA_max.y - abB_min.y;
	if (dy1 <= 0) {
		return false;
	}

	// aの奥側面がbの手前側面より手前にあるなら、交差していない
	const float dz0 = abB_max.z - abA_min.z;
	if (dz0 <= 0) {
		return false;
	}
	// aの手前側面がbの奥側面より奥にあるなら、交差していない
	const float dz1 = abA_max.z - abB_min.z;
	if (dz1 <= 0) {
		return false;
	}

	/* この時点で交差が確定 */

	// XYZの各軸について、重なっている距離が短い方向を選択
	vec3 length = { dx1, dy1, dz1 }; // 貫通距離の絶対値
	vec3 signedLength = length;      // 符号付きの貫通距離

	if (dx0 < dx1) {	// 左右
		length.x = dx0;
		signedLength.x = -dx0;
	}
	if (dy0 < dy1) {	// 上下
		length.y = dy0;
		signedLength.y = -dy0;
	}
	if (dz0 < dz1) {	//奥手
		length.z = dz0;
		signedLength.z = -dz0;
	}

	// XYZのうち最も短い方向を選択
	if (length.x < length.y) {
		if (length.x < length.z) {
			cp.penetration = { signedLength.x, 0, 0 };
			return true;
		}
	}
	else if (length.y < length.z) {
		cp.penetration = { 0, signedLength.y, 0 };
		return true;
	}
	cp.penetration = { 0, 0, signedLength.z };
	
	// 衝突点を計算
	for (int i = 0; i < 3; ++i) {
		cp.position[i] =
			std::max(abA_min[i], abB_min[i]) + std::min(abA_max[i], abB_max[i]);
	}
	cp.position *= 0.5f;

	return true;
}

/**
* AABBと球体の衝突判定
*
* @param aabb    衝突判定を行うAABB
* @param sphere  衝突判定を行う球体
* @param cp      衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const AABB& aabb, const Sphere& sphere, ContactPoint& cp)
{
	// 最近接点までの距離が球体の半径より長ければ、交差していない
	const vec3 p = Calc::ClosestPoint(aabb, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	// 交差しているので、貫通ベクトルを求める

	if (d2 > 0) {
		// 距離が0より大きい場合、球体の中心はAABBの外側にある
		// 球体の中心座標から最近接点へ向かう方向から衝突したとみなす
		const float d = sqrt(d2);
		cp.penetration = v * ((sphere.radius - d) / d);
	}
	else {
		// 距離が0の場合、球体の中心はAABBの内部にある
		// 貫通距離が最も短い面から衝突したとみなす
		int faceIndex = 0; // 貫通方向を示すインデックス
		float distance = FLT_MAX; // 貫通距離
		for (int i = 0; i < 3; ++i) {
			// -面
			float t0 = p[i] - (aabb.min[i] + aabb.position[i]);
			if (t0 < distance) {
				faceIndex = i * 2;
				distance = t0;
			}

			// +面
			float t1 = (aabb.max[i] + aabb.position[i]) - p[i];
			if (t1 < distance) {
				faceIndex = i * 2 + 1;
				distance = t1;
			}
		}

		// 「AABBが球体に対してどれだけ貫通しているか」を示すベクトルが欲しいので
		// 面の外向きのベクトルを使う
		static const vec3 faceNormals[] = {
			{-1, 0, 0 }, { 1, 0, 0 }, // -X, +X,
			{ 0,-1, 0 }, { 0, 1, 0 }, // -Y, +Y,
			{ 0, 0,-1 }, { 0, 0, 1 }, // -Z, +Z,
		};
		cp.penetration = faceNormals[faceIndex] * distance;
	}

	// 衝突座標の計算
	cp.position = p - cp.penetration * 0.5f;

	return true;
}

/**
* AABBとOBBの衝突判定
*
* @param aabb 衝突判定を行うAABB
* @param box  衝突判定を行うOBB
* @param cp   衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const AABB& aabb, const Box& box, ContactPoint& cp)
{
	if (Intersect(box, aabb, cp)) {
		cp.penetration = -cp.penetration;
		return true;
	}
	return false;
}



/**
* 球体と球体の衝突判定
*
* @param sphereA  衝突判定を行う球体その1
* @param sphereB  衝突判定を行う球体その2
* @param cp  衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Sphere& sphereA, const Sphere& sphereB, ContactPoint& cp)
{
	// 中心の間の距離の2乗を計算
	const vec3 v = sphereB.position - sphereA.position; // aの中心からbの中心に向かうベクトル
	const float d2 = dot(v, v); // vの長さの2乗

	// d2が半径の合計より長い場合は交差していない
	const float r = sphereA.radius + sphereB.radius; // aとbの半径の合計
	if (d2 > r * r) { // 平方根を避けるため、2乗同士で比較する
		return false;
	}

	// 交差しているので貫通ベクトルを求める
	const float d = sqrt(d2); // 「長さの2乗」を「長さ」に変換
	const float t = (r - d) / d; // 「半径の合計 - 長さ」の「長さに対する比率」を求める
	cp.penetration = v * t; // 貫通ベクトルを計算

	// 衝突位置を球体の中間に設定
	cp.position = sphereA.position + (v * (sphereA.radius / d)); 

	return true;
}

/**
* 球体とAABBの衝突判定
*
* @param sphere  衝突判定を行う球体
* @param aabb    衝突判定を行うAABB
* @param cp      衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Sphere& sphere, const AABB& aabb, ContactPoint& cp)
{
	if (Intersect(aabb, sphere, cp)) {
		cp.penetration = -cp.penetration;
		return true;
	}
	return false;
}

/**
* 球体とOBBの衝突判定
*
* @param sphere  衝突判定を行う球体
* @param obb     衝突判定を行うOBB
* @param cp      衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Sphere& sphere, const Box& box, ContactPoint& cp)
{
	if (Intersect(box, sphere, cp)) {
		cp.penetration = -cp.penetration;
		return true;
	}
	return false;
}


/**
* OBBとOBBの衝突判定
*
* @param boxA 衝突判定を行うOBBその1
* @param boxB 衝突判定を行うOBBその2
* @param cp   衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Box& boxA, const Box& boxB, ContactPoint& cp)
{
	// OBBの頂点座標配列を取得
	const auto verticesA = Calc::GetVertices(boxA);
	const auto verticesB = Calc::GetVertices(boxB);

	vec3 direction = vec3(0);	// 衝突方向
	float distance = -FLT_MAX;	// 貫通距離

	// 分離軸判定用の軸ベクトルを計算
	vec3 axisList[15] = {
		boxA.axis[0], boxA.axis[1], boxA.axis[2],	// オブジェクトAのすべての面法線
		-boxB.axis[0], -boxB.axis[1], -boxB.axis[2],// オブジェクトBのすべての面法線
	};
	for (int i = 0; i < 3; ++i) {	// Aの全ての辺とBの全ての辺の外積によって得られるベクトル(軸ベクトル)を計算
		axisList[6 + i * 3 + 0] = cross(axisList[i], axisList[3]);
		axisList[6 + i * 3 + 1] = cross(axisList[i], axisList[4]);
		axisList[6 + i * 3 + 2] = cross(axisList[i], axisList[5]);
		/*辺同士の衝突は、2つの辺の最短距離を求めることで調べられる。
		最短距離を結ぶベクトルは常に両辺に垂直なので、外積で代用できる。*/
	}

	// すべての軸ベクトルに対して分離軸判定を実行
	for (int i = 0; i < 15; ++i) {
		// 外積によって作られた軸ベクトルの場合、元の軸が平行だと長さが0になる
		// この場合、AまたはBの軸による分離軸判定と等しいのでスキップする
		const float length2 = dot(axisList[i], axisList[i]);
		if (length2 < 0.0001f) {
			continue;
		}

		// 交差している距離を計算
		bool shouldFlip;
		float d = Calc::GetIntersectionLength(
			verticesA.data(), verticesB.data(), axisList[i], shouldFlip);

		// 距離が0以上なら分離超平面を入れる隙間がある(つまり交差していない)
		if (d >= 0) {
			return false; // 衝突していない
		}

		// 外積で作成した軸ベクトルの長さは1にならない(sinθなので)
		// 結果として、dは軸ベクトルの長さを単位とした値になる
		// しかし、単位が異なると長さを比較できないため、正規化して単位を揃える
		d /= std::sqrt(length2);	// 交差距離dを「軸ベクトルの長さ」で除算

		// 交差距離がより短い方向から衝突したとみなす
		if (d > distance) {
			distance = d;
			// 衝突法線は軸ベクトルの逆方向(軸ベクトルは「Intersect関数の第1引数として渡された図形」を基準としているため)
			if (shouldFlip) {
				direction = axisList[i];
			}
			else {
				direction = -axisList[i];
			}
		}
	}

	// 衝突法線を正規化
	direction = normalize(direction);


	// コンタクトポイントを計算

	// OBBとOBBの交点を求める(OBBを入れ替えて2回実行)
	std::vector<vec3> c0 = Calc::ClipEdges(verticesB, boxA);
	std::vector<vec3> c1 = Calc::ClipEdges(verticesA, boxB);
	c0.insert(c0.end(), c1.begin(), c1.end());

	// 貫通距離の中間に衝突平面を設定(衝突平面上の点pを求める)
	float min, max;
	Calc::ProjectBoxToAxis(verticesA.data(), direction, min, max);
	const vec3 p = 
		boxA.position - direction * (max - min + distance) * 0.5f;

	// 交点を衝突平面に射影しつつ、重複する交点を削除
	int count = 0;
	for (int i = 0; i < c0.size(); ++i, ++count) {
		// 交点を衝突平面に射影(2次元図形の作成)
		c0[count] = c0[i] + direction * dot(direction, p - c0[i]);

		// 重複する交点がある場合は削除
		for (int j = 0; j < count; ++j) {
			const vec3 v = c0[j] - c0[count];
			if (dot(v, v) < 0.0001f) {
				--count;
				break;
			}
		}
	}
	c0.resize(count);

	// 交点の重心を計算し、コンタクトポイントとする
	cp.position = vec3(0);
	for (const auto& e : c0) {
		cp.position += e;
	}
	cp.position /= static_cast<float>(c0.size());
	cp.penetration = direction * distance;

	return true; // 衝突している
}

/**
* OBBとAABBの衝突判定
*
* @param box  衝突判定を行うOBB
* @param aabb 衝突判定を行うAABB
* @param cp   衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Box& box, const AABB& aabb, ContactPoint& cp)
{
	Box boxB;
	boxB.position = aabb.position;
	boxB.scale = (aabb.max - aabb.min) * 0.5f;
	return Intersect(box, boxB, cp);
}

/**
* OBBと球体の衝突判定
*
* @param box     衝突判定を行うOBB
* @param sphere  衝突判定を行う球体
* @param cp      衝突結果を格納するコンタクトポイント
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool Intersect(const Box& box, const Sphere& sphere, ContactPoint& cp)
{
	// 最近接点から球体の中心までの距離が、球体の半径より大きければ衝突していない
	const vec3 p = Calc::ClosestPoint(box, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	if (d2 > 0.00001f) {
		// 距離が0より大きい場合、球体の中心はOBBの外側にある
		// この場合、最近接から球体の中心へ向かう方向から衝突したとみなす
		const float d = sqrt(d2);
		cp.penetration = v * ((sphere.radius - d) / d);

	}
	else {
		// 距離が0の場合、球体の中心はOBBの内部にある
		// この場合、貫通距離が最も短い面から衝突したとみなす
		const vec3 a = p - box.position; // OBB中心から球体中心へのベクトル
		int faceIndex = 0; // 貫通方向を示すインデックス
		float distance = FLT_MAX; // 貫通距離
		float sign = 1; // 貫通ベクトルの符号
		for (int i = 0; i < 3; ++i) {
			const float f = dot(box.axis[i], a); // aを軸ベクトルに射影
			const float t0 = f - (-box.scale[i]);
			if (t0 < distance) {
				faceIndex = i;
				distance = t0;
				sign = -1;
			}
			const float t1 = box.scale[i] - f;
			if (t1 < distance) {
				faceIndex = i;
				distance = t1;
				sign = 1;
			}
		}
		cp.penetration = box.axis[faceIndex] * (distance * sign);
	}

	cp.position = p - cp.penetration * 0.5f;

	return true;
}
