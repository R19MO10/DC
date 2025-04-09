/**
* @file Collision.cpp
*/
#include "Collision.h"

#include <algorithm>
#include <cmath>
#include <vector>

/**
* �Փ˔���p�̍\���̂�֐����i�[���閼�O���
*/
namespace Calc {

	// �������O���
	namespace {

		/**
		* OBB�̒��_���W���擾����
		*
		* @param box ���_���W�����߂�OBB
		*
		* @return box�̒��_���W�z��
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
		* OBB�̕ӂ��擾����
		*
		* @param vertices ���_���W�����߂�OBB
		*
		* @return box�̕ӂ̔z��
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
				// X������
				{ vertices[0], vertices[1] },
				{ vertices[2], vertices[3] },
				{ vertices[4], vertices[5] },
				{ vertices[6], vertices[7] },

				// Y������
				{ vertices[0], vertices[2] },
				{ vertices[1], vertices[3] },
				{ vertices[4], vertices[6] },
				{ vertices[5], vertices[7] },

				// Z������
				{ vertices[0], vertices[4] },
				{ vertices[1], vertices[5] },
				{ vertices[2], vertices[6] },
				{ vertices[3], vertices[7] },
			};
		}

		/**
		* OBB�̖ʂ��擾����
		*
		* @param box �ʂ����߂�OBB
		*
		* @return box�̖ʂ̔z��
		*/
		std::vector<Plane> GetPlanes(const Box& box)
		{
			const vec3 ax = box.axis[0] * box.scale.x;
			const vec3 ay = box.axis[1] * box.scale.y;
			const vec3 az = box.axis[2] * box.scale.z;

			return {	// ���ʏ�̍��W��ʖ@���Ɏˉe
				{ box.axis[0], dot(box.axis[0], box.position + ax) }, // +x
				{-box.axis[0],-dot(box.axis[0], box.position - ax) }, // -x
				{ box.axis[1], dot(box.axis[1], box.position + ay) }, // +y
				{-box.axis[1],-dot(box.axis[1], box.position - ay) }, // -y
				{ box.axis[2], dot(box.axis[2], box.position + az) }, // +z
				{-box.axis[2],-dot(box.axis[2], box.position - az) }, // -z
			};
		}

		/**
		* OBB�̒��_�����Ɏˉe���A�ő�l�ƍŏ��l�����߂�
		*
		* @param vertices OBB�̒��_�z��
		* @param axis     �ˉe��̎��x�N�g��
		* @param min      �ŏ��l���i�[����ϐ�
		* @param max      �ő�l���i�[����ϐ�
		*/
		void ProjectBoxToAxis(const vec3* vertices, const vec3& axis,
			float& min, float& max)
		{
			min = max = dot(axis, vertices[0]);

			for (int i = 0; i < 8; ++i) {
				float f = dot(axis, vertices[i]);	// ���W���x�N�g���Ɏˉe����
				min = std::min(min, f);
				max = std::max(max, f);
			}
		}

		/**
		* 2��OBB�����x�N�g���Ɏˉe���A�������Ă��鋗�������߂�
		*
		* @param verticesA  OBB����1�̒��_�z��
		* @param verticesB  OBB����2�̒��_�z��
		* @param axis       �ˉe��̎��x�N�g��
		* @param shouldFlip �Փ˖@���̔��]�̗L��
		*
		* @return �������Ă��鋗��(�v�Z�̓s���Ń}�C�i�X�l�ɂȂ��Ă��邱�Ƃɒ���)
		*/
		float GetIntersectionLength(const vec3* verticesA, const vec3* verticesB,
			const vec3& axis, bool& shouldFlip)
		{
			// 2��OBB�����Ɏˉe
			float minA, maxA, minB, maxB;
			ProjectBoxToAxis(verticesA, axis, minA, maxA);
			ProjectBoxToAxis(verticesB, axis, minB, maxB);

			// �ˉe�����͈͂��������Ă��Ȃ��Ȃ�0��Ԃ�
			if (minB > maxA && minA > maxB) {
				return 0;
			}

			// �Փ˖@���́uOBB����1�v�����A�uOBB����2�v���E�ɂ���Ƒz�肵�Ă���̂ŁA
			// �������t�ɂȂ��Ă���ꍇ�A�Փ˖@�{�̌����𔽓]����K�v������A
			// �Ƃ������Ƃ��A�Ăяo�����ɓ`����
			shouldFlip = minB < minA;

			// �������Ă��鋗�����v�Z
			float lenA = maxA - minA;
			float lenB = maxB - minB;
			float min = std::min(minA, minB);
			float max = std::max(maxA, maxB);
			float length = max - min;
			return length - lenA - lenB;
		}

		/**
		* �_��OBB���ɂ��邩�ǂ����𒲂ׂ�
		*
		* @param point ���ׂ�_�̍��W
		* @param box   ���ׂ�OBB
		*
		* @retval true  point��box�̓����ɂ���
		* @retval false point��box�̊O���ɂ���
		*/
		bool IsPointInsideBox(const vec3& point, const Box& box)
		{
			const vec3 v = point - box.position;	// �_�Ɍ������x�N�g�����v�Z
			for (int i = 0; i < 3; ++i) {
				const float d = dot(v, box.axis[i]);	// ���Ɏˉe
				if (d < -box.scale[i] || d > box.scale[i]) {
					return false; // �_��OBB�̊O���ɗL��
				}
			}
			return true; // �_��OBB�̓����ɂ���
		}

		/**
		* �����Ɩʂ̌�_�����߂�
		*
		* @param seg   ����
		* @param plane ��
		* @param p     ��_���i�[����ϐ�
		*
		* @retval true  ��_�͐�����ɂ���
		* @retval false ��_�͐����̊O�ɂ���
		*/
		bool ClipLineSegment(const LineSegment& seg, const Plane& plane, vec3& p)
		{
			// �Ӄx�N�g����ʖ@���Ɏˉe
			const vec3 ab = seg.end - seg.start;
			float n = dot(plane.normal, ab);
			if (abs(n) < 0.0001f) {
				return false; // ���s�Ȃ̂Ō����Ȃ�
			}

			// ��_�܂ł̋��������߂�
			float t = plane.d - dot(plane.normal, seg.start);
			if (t < 0 || t > n) {
				return false; // ��_�������̊O�ɂ���̂Ō����Ȃ�
			}

			// ��_���W���v�Z
			t /= n; // ���[���h���W�n�̒�������x�N�g��ab�ɑ΂���䗦�ɕϊ�
			p = seg.start + ab * t;	// �n�_�����_�܂ł̃x�N�g��
			return true;
		}

		/**
		* OBB�̑S�Ă̕ӂɂ��āA�������OBB�̖ʂƂ̌�_�����߂�
		*
		* @param vertices OBB����1�̒��_�z��
		* @param box      OBB����2
		*
		* @return edges��box�̌�_
		*/
		std::vector<vec3> ClipEdges(const std::vector<vec3>& vertices, const Box& box)
		{
			// �ӂƖʂ��擾
			const auto edges = GetEdges(vertices.data());
			const auto planes = GetPlanes(box);

			// ��_�p�̃�������\��
			// 2��OBB�������[���������Ă��Ȃ�����A��_�̐��͕ӂ̐��ȉ��ɂȂ�͂�
			std::vector<vec3> result;
			result.reserve(edges.size());

			// �ӂƖʂ̌�_�����߂�
			/*�œK���F�Ⴆ�΁A�Փ˖@���Ƌt�����̖ʂƕӂ͖����ł���ł��傤�B*/
			for (const auto& plane : planes) {
				for (const auto& edge : edges) {
					vec3 p;
					if (ClipLineSegment(edge, plane, p)) {
						// ��_��OBB����2�̓����ɂ���Ȃ�A�L���Ȍ�_�Ƃ��ċL�^
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
	* AABB����_�ւ̍ŋߐړ_
	*
	* @param aabb  ����Ώۂ�AABB
	* @param point ����Ώۂ̓_
	*
	* @return AABB�̒��ōł��_�ɋ߂����W
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
	* OBB����_�ւ̍ŋߐړ_
	*
	* @param box   ����Ώۂ�OBB
	* @param point ����Ώۂ̓_
	*
	* @return �����`�̒��ōł��_�ɋ߂����W
	*/
	vec3 ClosestPoint(const Box& box, const vec3& point)
	{
		// OBB����_�Ɍ������x�N�g��
		const vec3 v = point - box.position;

		vec3 result = box.position;
		for (int i = 0; i < 3; ++i) {
			// �x�N�g����OBB�̎��Ɏˉe
			float d = dot(v, box.axis[i]);

			// �ˉe�œ���ꂽ�l��OBB�͈͓̔��ɐ���
			if (box.scale[i] > 0) {
				d = std::clamp(d, -box.scale[i], box.scale[i]);
			}
			else {
				d = std::clamp(d, box.scale[i], -box.scale[i]);
			}

			// �ŋߐړ_���X�V
			result += d * box.axis[i];
		}
		return result;
	}


	/**
	* ������̍ŋߐړ_�����߂�
	*
	* @param seg   �ŋߐړ_�����߂����
	* @param point �ŋߐړ_�����߂���W
	*
	* @return point����seg�ւ̍ŋߐړ_
	*/
	vec3 ClosestPoint(const LineSegment& seg, const vec3& point)
	{
		// �_������Ɏˉe
		const vec3 d = seg.end - seg.start;
		const float t = dot(point - seg.start, d);

		// �����͈̔͂ɐ���
		if (t <= 0) {
			return seg.start;
		}
		const float d2 = dot(d, d); // �����̒�����2��
		if (t >= d2) {
			return seg.end;
		}

		// �ŋߐړ_���v�Z���ĕԂ�
		return seg.start + d * (t / d2);
	}

	/**
	* 2�̐����̍ŋߐړ_�����߂�
	*
	* @param segA  �ŋߐړ_�����߂��������1
	* @param segB  �ŋߐړ_�����߂��������2
	* @param denom segA��̍ŋߐړ_�̔䗦
	*
	* @return segA��̍ŋߐړ_��segB��̍ŋߐړ_
	*/
	LineSegment ClosestPoint(const LineSegment& segA, const LineSegment& segB)
	{
		// ����A�̕����x�N�g���ƒ������v�Z
		const vec3 directionA = segA.end - segA.start;
		const float lengthSquaredA = dot(directionA, directionA);	// ����A�̒�����2��

		// ����B�̕����x�N�g���ƒ������v�Z
		const vec3 directionB = segB.end - segB.start;
		const float lengthSquaredB = dot(directionB, directionB); // ����B�̒�����2��

		// ����A��B�̎n�_�Ԃ̃x�N�g�����v�Z
		const vec3 relativePosition = segA.start - segB.start;

		// ����A��B�̕����x�N�g���̓��ς��v�Z
		const float dotAB = dot(directionA, directionB);
		const float dotARelative = dot(directionA, relativePosition);
		const float dotBRelative = dot(directionB, relativePosition);


		// ����A��B�̗����̒�����0�̓���P�[�X�������i������0�̐������u�_�v�Ƃ݂Ȃ��Čv�Z�j
		if (lengthSquaredA < 0.0001f && lengthSquaredB < 0.0001f) {
			/* ����A��B���Ƃ��ɓ_�̏ꍇ�A���̓_���m��Ԃ� */
			return { segA.start, segB.start };
		}


		// ����A�̒�����0
		if (lengthSquaredA < 0.0001f) {
			if (dotBRelative <= 0) {
				/* ����A�̎n�_�������B�ւ̍ŒZ���������̏ꍇ�A
				   ����A�̎n�_��Ԃ� */
				return { segA.start, segB.start };
			}
			else if (dotBRelative >= lengthSquaredB) {
				/* ����A�̎n�_�������B�ւ̍ŒZ����������B�̒����𒴂���ꍇ�A
				�@ ����A�̎n�_�������B�̏I�_��Ԃ� */
				return { segA.start, segB.end };
			}

			/* ����A�̎n�_�������B�ւ̍ŒZ����������B�̒������Ɏ��܂�ꍇ�A
			   ����A�̎n�_�������B��̍ŋߐړ_��Ԃ� */
			return { segA.start, segB.start + directionB * (dotBRelative / lengthSquaredB) };
		}


		// ����B�̒�����0
		if (lengthSquaredB < 0.0001f) {
			if (-dotARelative <= 0) {
				/* ����B�̎n�_�������A�ւ̍ŒZ���������̏ꍇ�A
				   ����A�̎n�_��Ԃ� */
				return { segA.start, segB.start };
			}
			else if (-dotARelative >= lengthSquaredA) {
				/* ����B�̎n�_�������A�ւ̍ŒZ����������A�̒����𒴂���ꍇ�A
				   ����A�̏I�_�������B�̎n�_��Ԃ� */
				return { segA.end, segB.start };
			}

			/* ����B�̎n�_�������A�ւ̍ŒZ����������A�̒������Ɏ��܂�ꍇ�A
			   ����A��̍ŋߐړ_�������B�̎n�_��Ԃ� */
			return { segA.start + directionA * (-dotARelative / lengthSquaredA), segB.start };
		}


		// �����̒�����0���傫���ƕ���������A���������s���ǂ������`�F�b�N����
		// ���������s�ȏꍇ�As�����߂鎮�̕��ꂪ0�ɂȂ�v�Z�ł��Ȃ�����
		const float denom = lengthSquaredA * lengthSquaredB - dotAB * dotAB;


		// ���������s�łȂ��ꍇ
		if (denom) {
			// ����A�̍ŋߐړ_�����߁A����A�͈̔͂ɐ���
			float s = std::clamp((dotBRelative * dotAB - dotARelative * lengthSquaredB) / denom, 0.0f, 1.0f);

			// ����B�̍ŋߐړ_�����߂�
			float t = dotAB * s + dotBRelative;

			// t������B�͈̔͊O�̏ꍇ�̏���
			LineSegment result;
			if (t < 0) {
				// t�����̏ꍇ�A����B�̎n�_���ŋߐړ_�ɂȂ�
				result.end = segB.start;
				s = std::clamp(-dotARelative / lengthSquaredA, 0.0f, 1.0f);	// ����A�͈͓̔��ɐ���
			}
			else if (t > lengthSquaredB) {
				// t������B�̒����𒴂���ꍇ�A����B�̏I�_���ŋߐړ_�ɂȂ�
				result.end = segB.end;
				s = std::clamp((dotAB - dotARelative) / lengthSquaredA, 0.0f, 1.0f);	// ����A�͈͓̔��ɐ���
			}
			else {
				// t�����B�͈͓̔��ɐ������A���̈ʒu�ɂ��������B��̓_���v�Z���A�ŋߐړ_�Ƃ��Đݒ�
				result.end = segB.start + directionB * (t / lengthSquaredB);
			}

			// ����A��̍ŋߐړ_���v�Z���A���߂��ŋߐړ_��Ԃ�
			result.start = segA.start + directionA * s;
			return result;

		}


		// ���������s�ȏꍇ�A�����̏d������͈͂̒��_��I��

		// ����B�����A�Ɏˉe
		float u = dot(directionA, segB.start - segA.start);
		float v = dot(directionA, segB.end - segA.start);

		// �ˉe�������W�����A�͈̔͂ɐ���
		u = std::clamp(u, 0.0f, lengthSquaredA);
		v = std::clamp(v, 0.0f, lengthSquaredA);

		// u��v�̒��_������A�̍ŋߐړ_�ɂȂ�
		const float s = (u + v) / lengthSquaredA * 0.5f;
		LineSegment result;
		result.start = segA.start + directionA * s;

		// ����A�̍ŋߐړ_�����B�Ɏˉe
		const float t = dot(directionB, result.start - segB.start);

		// �ˉe�������W�����B�͈̔͂ɐ���
		if (t <= 0) {
			/* t�����̏ꍇ�A����A�̍ŋߐړ_������B�̎n�_�̋t�����Ɉʒu���Ă��邽�߁A
			   �ŋߐړ_�����B�̎n�_�ɐݒ� */
			result.end = segB.start;
		}
		else if (t >= lengthSquaredB) {
			/* t������B�̒����ȏ�̏ꍇ�A����A�̍ŋߐړ_������B�̏I�_�̋t�����Ɉʒu���Ă��邽�߁A
			   �ŋߐړ_�����B�̏I�_�ɐݒ� */
			result.end = segB.end;
		}
		else {
			// t�����B�͈͓̔��ɐ������A���̈ʒu�ɂ��������B��̓_���v�Z���A�ŋߐړ_�Ƃ��Đݒ�
			result.end = segB.start + directionB * (t / lengthSquaredB);
		}

		return result;
	}

	/**
	* 3D�����`�Ɛ����̍ŋߐړ_�����߂�
	*
	* @param rect �ŋߐړ_�����߂�3D�����`
	* @param seg  �ŋߐړ_�����߂����
	*
	* @return seg��̍ŋߐړ_��rect��̍ŋߐړ_
	*/
	LineSegment ClosestPoint(const Rectangle& rect, const LineSegment& seg)
	{
		// �����̎n�_��ʂɎˉe
		const vec3 ca = seg.start - rect.center;
		const float a0 = dot(ca, rect.axis[0]);
		const float a1 = dot(ca, rect.axis[1]);

		// �����`�Ɛ����̕��s���𒲂ׂ�
		const vec3 ab = seg.end - seg.start;
		const float n = dot(rect.normal, ab);
		if (abs(n) <= 0.0001f) {
			// ���s�ȏꍇ�A�����������`�̏�ɏ���Ă��邩�ǂ����𒲂ׂ�
			float tmin = -FLT_MAX;
			float tmax = FLT_MAX;
			const float list[] = { a0, a1 };
			for (int i = 0; i < 2; ++i) {
				// ������axis[i]�Ƃ̕��s���𒲂ׂ�
				const float d = dot(ab, rect.axis[i]);
				if (abs(d) <= 0.0001f) {
					continue; // ���s�Ȃ̂Ō�_���Ȃ�
				}
				// �����`���S����̋���������̔䗦�Ƃ��ĕ\��
				float t0 = (-rect.scale[i] - list[i]) / d;
				float t1 = (rect.scale[i] - list[i]) / d;
				if (t0 > t1) {
					std::swap(t0, t1);
				}
				// �����`�ƌ������Ă���͈͂��v�Z
				tmin = std::max(t0, tmin);
				tmax = std::min(t1, tmax);
			}

			// �����̎n�_�ƏI�_�͈̔͂ɐ�������
			tmin = std::max(0.0f, tmin);
			tmax = std::min(1.0f, tmax);

			// �����������`�ɏd�Ȃ��Ă���ꍇ
			// �d���͈͂̒��S���ŋߐړ_�Ƃ���
			if (tmin <= tmax) {
				const vec3 p = seg.start + ab * ((tmin + tmax) * 0.5f);
				const float d = dot(rect.normal, p - rect.center);
				return { p, p - rect.normal * d };
			}
		} // if (abs(n) <= 0.0001f)

		// �������ʂƌ������Ă��邩�`�F�b�N
		// �������Ă���ꍇ�A�ŋߐړ_�͌�_�ɂȂ�
		{
			const float d = dot(rect.normal, rect.center);	// �ʖ@��
			const float t = (d - dot(rect.normal, seg.start)) / n;	// �����`��������̎n�_�܂ł̋���
			/* �����`�̒��S�Ɛ����̎n�_��ʖ@���ɑ΂��Ďˉe��
			   �������ˉe��������n�Ŋ���
			   �i�����̒���n�Ŋ��邱�ƂŁAt�͐����ɑ΂���䗦�ɂȂ�(t��0�Ȃ��_�͎n�_ / t��1�Ȃ��_�͏I�_)�j*/

			if (0 <= t && t <= 1) {
				// ��_�������`�͈͓̔��ɂ��邩���ׂ�

				const vec3 p = seg.start + ab * t;
				const vec3 v = p - rect.center;	// �����`�̒��S�����_�֌������x�N�g��

				// �����`��2�̎��x�N�g���Ɏˉe
				const float dist1 = dot(v, rect.axis[0]);
				if (abs(dist1) <= rect.scale[0]) {
					const float dist2 = dot(v, rect.axis[1]);
					if (abs(dist2) <= rect.scale[1]) {
						// ��_�������`���ɗL�邱�Ƃ������������߁A���̍��W���ŋߐړ_�Ƃ��ĕԂ�
						return { p, p };
					}
				}
			} // if (0 <= t && t <= n)
		}


		// ���s�łȂ��A���������Ă��Ȃ��ꍇ

		// �����̏I�_��ʂɎˉe
		const vec3 cb = seg.end - rect.center;
		const float b0 = dot(cb, rect.axis[0]);
		const float b1 = dot(cb, rect.axis[1]);

		// �����̎n�_����яI�_�̍ŋߐړ_������
		// �����`�͈̔͂Ɋ܂܂��_�̂������߂��ق����u���̍ŋߐړ_�v�Ƃ���
		float distance = FLT_MAX;
		LineSegment result;

		// �n�_
		if (abs(a0) <= rect.scale[0] && abs(a1) <= rect.scale[1]) {	// �����`�͈̔̓`�F�b�N
			distance = dot(rect.normal, ca);
			distance *= distance; // ��r�̂��߂�2�悷��
			result = { seg.start, rect.center + rect.axis[0] * a0 + rect.axis[1] * a1 };
		}

		// �I�_
		if (abs(b0) <= rect.scale[0] && abs(b1) <= rect.scale[1]) {	// �����`�͈̔̓`�F�b�N
			float tmp = dot(rect.normal, cb);
			tmp *= tmp; // ��r�̂��߂�2�悷��
			if (tmp < distance) {
				distance = tmp;
				result = { seg.end, rect.center + rect.axis[0] * b0 + rect.axis[1] * b1 };
			}
		}


		// �����`��4�ӂɂ��āA�ŋߐړ_���v�Z
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
			const float distanceEdge = dot(ab, ab); // ������2��
			if (distanceEdge < distance) { // 2�擯�m�̔�r
				distance = distanceEdge;
				result = s;
			}
		}

		return result;
	}
}

/**
* AABB��AABB�̏Փ˔���
*
* @param aabbA  �Փ˔�����s��AABB����1
* @param aabbB  �Փ˔�����s��AABB����2
* @param cp  �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const AABB& aabbA, const AABB& aabbB, ContactPoint& cp)
{
	const vec3 abA_min = aabbA.min + aabbA.position;
	const vec3 abA_max = aabbA.max + aabbA.position;

	const vec3 abB_min = aabbB.min + aabbB.position;
	const vec3 abB_max = aabbB.max + aabbB.position;

	// a�̍����ʂ�b�̉E���ʂ��E�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dx0 = abB_max.x - abA_min.x;
	if (dx0 <= 0) {
		return false;
	}

	// a�̉E���ʂ�b�̍����ʂ�荶�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dx1 = abA_max.x - abB_min.x;
	if (dx1 <= 0) {
		return false;
	}
	// a�̉��ʂ�b�̏�ʂ���ɂ���Ȃ�A�������Ă��Ȃ�
	const float dy0 = abB_max.y - abA_min.y;
	if (dy0 <= 0) {
		return false;
	}
	// a�̏�ʂ�b�̉��ʂ�艺�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dy1 = abA_max.y - abB_min.y;
	if (dy1 <= 0) {
		return false;
	}

	// a�̉����ʂ�b�̎�O���ʂ���O�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dz0 = abB_max.z - abA_min.z;
	if (dz0 <= 0) {
		return false;
	}
	// a�̎�O���ʂ�b�̉����ʂ�艜�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dz1 = abA_max.z - abB_min.z;
	if (dz1 <= 0) {
		return false;
	}

	/* ���̎��_�Ō������m�� */

	// XYZ�̊e���ɂ��āA�d�Ȃ��Ă��鋗�����Z��������I��
	vec3 length = { dx1, dy1, dz1 }; // �ђʋ����̐�Βl
	vec3 signedLength = length;      // �����t���̊ђʋ���

	if (dx0 < dx1) {	// ���E
		length.x = dx0;
		signedLength.x = -dx0;
	}
	if (dy0 < dy1) {	// �㉺
		length.y = dy0;
		signedLength.y = -dy0;
	}
	if (dz0 < dz1) {	//����
		length.z = dz0;
		signedLength.z = -dz0;
	}

	// XYZ�̂����ł��Z��������I��
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
	
	// �Փ˓_���v�Z
	for (int i = 0; i < 3; ++i) {
		cp.position[i] =
			std::max(abA_min[i], abB_min[i]) + std::min(abA_max[i], abB_max[i]);
	}
	cp.position *= 0.5f;

	return true;
}

/**
* AABB�Ƌ��̂̏Փ˔���
*
* @param aabb    �Փ˔�����s��AABB
* @param sphere  �Փ˔�����s������
* @param cp      �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const AABB& aabb, const Sphere& sphere, ContactPoint& cp)
{
	// �ŋߐړ_�܂ł̋��������̂̔��a��蒷����΁A�������Ă��Ȃ�
	const vec3 p = Calc::ClosestPoint(aabb, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	// �������Ă���̂ŁA�ђʃx�N�g�������߂�

	if (d2 > 0) {
		// ������0���傫���ꍇ�A���̂̒��S��AABB�̊O���ɂ���
		// ���̂̒��S���W����ŋߐړ_�֌�������������Փ˂����Ƃ݂Ȃ�
		const float d = sqrt(d2);
		cp.penetration = v * ((sphere.radius - d) / d);
	}
	else {
		// ������0�̏ꍇ�A���̂̒��S��AABB�̓����ɂ���
		// �ђʋ������ł��Z���ʂ���Փ˂����Ƃ݂Ȃ�
		int faceIndex = 0; // �ђʕ����������C���f�b�N�X
		float distance = FLT_MAX; // �ђʋ���
		for (int i = 0; i < 3; ++i) {
			// -��
			float t0 = p[i] - (aabb.min[i] + aabb.position[i]);
			if (t0 < distance) {
				faceIndex = i * 2;
				distance = t0;
			}

			// +��
			float t1 = (aabb.max[i] + aabb.position[i]) - p[i];
			if (t1 < distance) {
				faceIndex = i * 2 + 1;
				distance = t1;
			}
		}

		// �uAABB�����̂ɑ΂��Ăǂꂾ���ђʂ��Ă��邩�v�������x�N�g�����~�����̂�
		// �ʂ̊O�����̃x�N�g�����g��
		static const vec3 faceNormals[] = {
			{-1, 0, 0 }, { 1, 0, 0 }, // -X, +X,
			{ 0,-1, 0 }, { 0, 1, 0 }, // -Y, +Y,
			{ 0, 0,-1 }, { 0, 0, 1 }, // -Z, +Z,
		};
		cp.penetration = faceNormals[faceIndex] * distance;
	}

	// �Փˍ��W�̌v�Z
	cp.position = p - cp.penetration * 0.5f;

	return true;
}

/**
* AABB��OBB�̏Փ˔���
*
* @param aabb �Փ˔�����s��AABB
* @param box  �Փ˔�����s��OBB
* @param cp   �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
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
* ���̂Ƌ��̂̏Փ˔���
*
* @param sphereA  �Փ˔�����s�����̂���1
* @param sphereB  �Փ˔�����s�����̂���2
* @param cp  �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const Sphere& sphereA, const Sphere& sphereB, ContactPoint& cp)
{
	// ���S�̊Ԃ̋�����2����v�Z
	const vec3 v = sphereB.position - sphereA.position; // a�̒��S����b�̒��S�Ɍ������x�N�g��
	const float d2 = dot(v, v); // v�̒�����2��

	// d2�����a�̍��v��蒷���ꍇ�͌������Ă��Ȃ�
	const float r = sphereA.radius + sphereB.radius; // a��b�̔��a�̍��v
	if (d2 > r * r) { // ������������邽�߁A2�擯�m�Ŕ�r����
		return false;
	}

	// �������Ă���̂Ŋђʃx�N�g�������߂�
	const float d = sqrt(d2); // �u������2��v���u�����v�ɕϊ�
	const float t = (r - d) / d; // �u���a�̍��v - �����v�́u�����ɑ΂���䗦�v�����߂�
	cp.penetration = v * t; // �ђʃx�N�g�����v�Z

	// �Փˈʒu�����̂̒��Ԃɐݒ�
	cp.position = sphereA.position + (v * (sphereA.radius / d)); 

	return true;
}

/**
* ���̂�AABB�̏Փ˔���
*
* @param sphere  �Փ˔�����s������
* @param aabb    �Փ˔�����s��AABB
* @param cp      �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
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
* ���̂�OBB�̏Փ˔���
*
* @param sphere  �Փ˔�����s������
* @param obb     �Փ˔�����s��OBB
* @param cp      �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
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
* OBB��OBB�̏Փ˔���
*
* @param boxA �Փ˔�����s��OBB����1
* @param boxB �Փ˔�����s��OBB����2
* @param cp   �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const Box& boxA, const Box& boxB, ContactPoint& cp)
{
	// OBB�̒��_���W�z����擾
	const auto verticesA = Calc::GetVertices(boxA);
	const auto verticesB = Calc::GetVertices(boxB);

	vec3 direction = vec3(0);	// �Փ˕���
	float distance = -FLT_MAX;	// �ђʋ���

	// ����������p�̎��x�N�g�����v�Z
	vec3 axisList[15] = {
		boxA.axis[0], boxA.axis[1], boxA.axis[2],	// �I�u�W�F�N�gA�̂��ׂĂ̖ʖ@��
		-boxB.axis[0], -boxB.axis[1], -boxB.axis[2],// �I�u�W�F�N�gB�̂��ׂĂ̖ʖ@��
	};
	for (int i = 0; i < 3; ++i) {	// A�̑S�Ă̕ӂ�B�̑S�Ă̕ӂ̊O�ςɂ���ē�����x�N�g��(���x�N�g��)���v�Z
		axisList[6 + i * 3 + 0] = cross(axisList[i], axisList[3]);
		axisList[6 + i * 3 + 1] = cross(axisList[i], axisList[4]);
		axisList[6 + i * 3 + 2] = cross(axisList[i], axisList[5]);
		/*�ӓ��m�̏Փ˂́A2�̕ӂ̍ŒZ���������߂邱�ƂŒ��ׂ���B
		�ŒZ���������ԃx�N�g���͏�ɗ��ӂɐ����Ȃ̂ŁA�O�ςő�p�ł���B*/
	}

	// ���ׂĂ̎��x�N�g���ɑ΂��ĕ�������������s
	for (int i = 0; i < 15; ++i) {
		// �O�ςɂ���č��ꂽ���x�N�g���̏ꍇ�A���̎������s���ƒ�����0�ɂȂ�
		// ���̏ꍇ�AA�܂���B�̎��ɂ�镪��������Ɠ������̂ŃX�L�b�v����
		const float length2 = dot(axisList[i], axisList[i]);
		if (length2 < 0.0001f) {
			continue;
		}

		// �������Ă��鋗�����v�Z
		bool shouldFlip;
		float d = Calc::GetIntersectionLength(
			verticesA.data(), verticesB.data(), axisList[i], shouldFlip);

		// ������0�ȏ�Ȃ番�������ʂ����錄�Ԃ�����(�܂�������Ă��Ȃ�)
		if (d >= 0) {
			return false; // �Փ˂��Ă��Ȃ�
		}

		// �O�ςō쐬�������x�N�g���̒�����1�ɂȂ�Ȃ�(sin�ƂȂ̂�)
		// ���ʂƂ��āAd�͎��x�N�g���̒�����P�ʂƂ����l�ɂȂ�
		// �������A�P�ʂ��قȂ�ƒ������r�ł��Ȃ����߁A���K�����ĒP�ʂ𑵂���
		d /= std::sqrt(length2);	// ��������d���u���x�N�g���̒����v�ŏ��Z

		// �������������Z����������Փ˂����Ƃ݂Ȃ�
		if (d > distance) {
			distance = d;
			// �Փ˖@���͎��x�N�g���̋t����(���x�N�g���́uIntersect�֐��̑�1�����Ƃ��ēn���ꂽ�}�`�v����Ƃ��Ă��邽��)
			if (shouldFlip) {
				direction = axisList[i];
			}
			else {
				direction = -axisList[i];
			}
		}
	}

	// �Փ˖@���𐳋K��
	direction = normalize(direction);


	// �R���^�N�g�|�C���g���v�Z

	// OBB��OBB�̌�_�����߂�(OBB�����ւ���2����s)
	std::vector<vec3> c0 = Calc::ClipEdges(verticesB, boxA);
	std::vector<vec3> c1 = Calc::ClipEdges(verticesA, boxB);
	c0.insert(c0.end(), c1.begin(), c1.end());

	// �ђʋ����̒��ԂɏՓ˕��ʂ�ݒ�(�Փ˕��ʏ�̓_p�����߂�)
	float min, max;
	Calc::ProjectBoxToAxis(verticesA.data(), direction, min, max);
	const vec3 p = 
		boxA.position - direction * (max - min + distance) * 0.5f;

	// ��_���Փ˕��ʂɎˉe���A�d�������_���폜
	int count = 0;
	for (int i = 0; i < c0.size(); ++i, ++count) {
		// ��_���Փ˕��ʂɎˉe(2�����}�`�̍쐬)
		c0[count] = c0[i] + direction * dot(direction, p - c0[i]);

		// �d�������_������ꍇ�͍폜
		for (int j = 0; j < count; ++j) {
			const vec3 v = c0[j] - c0[count];
			if (dot(v, v) < 0.0001f) {
				--count;
				break;
			}
		}
	}
	c0.resize(count);

	// ��_�̏d�S���v�Z���A�R���^�N�g�|�C���g�Ƃ���
	cp.position = vec3(0);
	for (const auto& e : c0) {
		cp.position += e;
	}
	cp.position /= static_cast<float>(c0.size());
	cp.penetration = direction * distance;

	return true; // �Փ˂��Ă���
}

/**
* OBB��AABB�̏Փ˔���
*
* @param box  �Փ˔�����s��OBB
* @param aabb �Փ˔�����s��AABB
* @param cp   �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const Box& box, const AABB& aabb, ContactPoint& cp)
{
	Box boxB;
	boxB.position = aabb.position;
	boxB.scale = (aabb.max - aabb.min) * 0.5f;
	return Intersect(box, boxB, cp);
}

/**
* OBB�Ƌ��̂̏Փ˔���
*
* @param box     �Փ˔�����s��OBB
* @param sphere  �Փ˔�����s������
* @param cp      �Փˌ��ʂ��i�[����R���^�N�g�|�C���g
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool Intersect(const Box& box, const Sphere& sphere, ContactPoint& cp)
{
	// �ŋߐړ_���狅�̂̒��S�܂ł̋������A���̂̔��a���傫����ΏՓ˂��Ă��Ȃ�
	const vec3 p = Calc::ClosestPoint(box, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius) {
		return false;
	}

	if (d2 > 0.00001f) {
		// ������0���傫���ꍇ�A���̂̒��S��OBB�̊O���ɂ���
		// ���̏ꍇ�A�ŋߐڂ��狅�̂̒��S�֌�������������Փ˂����Ƃ݂Ȃ�
		const float d = sqrt(d2);
		cp.penetration = v * ((sphere.radius - d) / d);

	}
	else {
		// ������0�̏ꍇ�A���̂̒��S��OBB�̓����ɂ���
		// ���̏ꍇ�A�ђʋ������ł��Z���ʂ���Փ˂����Ƃ݂Ȃ�
		const vec3 a = p - box.position; // OBB���S���狅�̒��S�ւ̃x�N�g��
		int faceIndex = 0; // �ђʕ����������C���f�b�N�X
		float distance = FLT_MAX; // �ђʋ���
		float sign = 1; // �ђʃx�N�g���̕���
		for (int i = 0; i < 3; ++i) {
			const float f = dot(box.axis[i], a); // a�����x�N�g���Ɏˉe
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
