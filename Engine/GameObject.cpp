/**
* @file GameObject.cpp
*/
#include "GameObject.h"

#include "Engine.h"
#include "../Component/MeshRenderer.h"
#include "../Component/Collider.h"
#include "../Component/Rigidbody.h"
#include "Collision/ContactPoint.h"
#include <algorithm>

/**
* �f�X�g���N�^
*/
GameObject::~GameObject()
{
	// ���ʑO�ɐe�q�֌W����������
	SetParent(nullptr);
	for (auto child : children) {
		child->parent = nullptr;
	}

	// �u���W�b�h�{�f�B�v�ƁuUI���C�A�E�g�v�R���|�[�l���g��
	// null�ɂ��Ă���
	rigidbody = nullptr;
}


// �^�O��ݒ�
void GameObject::AddTag(const char* tag)
{
	MyFName ftag(tag);

	// �����̃^�O�Ȃ珈�����Ȃ�
	if (ftag == MyFName()) {
		return;
	}

	const auto& someTag_itr =
		std::find_if(tags.begin(), tags.end(),
			[&ftag](const MyFName t) { return t == ftag; });
	// �����^�O�������łɂ���Ώ������Ȃ�
	if (tags.end() != someTag_itr) {
		return;
	}

	if (engine->GameObjectAddTag(ftag, this)) {
		tags.push_back(ftag);
	}
}

void GameObject::RemoveTag(size_t index)
{
	if (index < tags.size()) {
		engine->GameObjectRemoveTag(tags[index], this);
	}

	tags.erase(tags.begin() + index);
}

void GameObject::CleanTags()
{
	for (const auto& tag : tags) {
		engine->GameObjectRemoveTag(tag, this);
	}
	tags.clear();
}


// �e�I�u�W�F�N�g��ݒ肷��
void GameObject::SetParent(GameObject* parent)
{
	// ���g��ݒ肳�ꂽ��e���폜����
	if (parent == this) {
		parent = nullptr;
		return;
	}

	// �����e���w�肳�ꂽ�ꍇ�͉������Ȃ�
	if (parent == this->parent) {
		return;
	}

	// �ʂ̐e������ꍇ�A���̐e�Ƃ̊֌W����������
	if (this->parent) {
		// �����̈ʒu������
		auto& c = this->parent->children;
		const auto& itr = std::find(c.begin(), c.end(), this);
		if (itr != c.end()) {
			c.erase(itr); // �z�񂩂玩�����폜
		}
	}

	// �V���Ȑe�q�֌W��ݒ�
	if (parent) {
		parent->children.push_back(this);
	}

	this->parent = parent;
}

// �e�I�u�W�F�N�g��ݒ肷��
void GameObject::SetParent(const GameObjectPtr& parent)
{
	SetParent(parent.get());
}

/**
* �Q�[���I�u�W�F�N�g����폜�\��̃R���|�[�l���g���폜����
*/
void GameObject::RemoveDestroyedComponent()
{
	if (components.empty()) {
		return; // �R���|�[�l���g�������Ă��Ȃ���Ή������Ȃ�
	}

	if (isDestroyed) {
		// ���g���폜����Ă���Ȃ珈�����Ȃ�(�f�X�g���N�^�ō폜����邽��)
		return;
	}

	// �R���|�[�l���g�z��̔j��
	// �j���\��̗L���ŃR���|�[�l���g�𕪂���	/*�z��̑O�ɏ����𖞂���(�j���\��ł͂Ȃ�)�v�f���W�߂�*/
	const auto& destCompBegin_itr = std::stable_partition(components.begin(), components.end(),
		[](const ComponentPtr& p) { return !p->IsDestroyed(); });

	if (destCompBegin_itr == components.end()) {
		return;	// �폜�\��̃R���|�[�l���g���Ȃ�
	}

	// �j���\��̃R���|�[�l���g��ʂ̔z��Ɉړ�
	/*OnDestroy�Ăяo�����ɐV�����R���|�[�l���g���ǉ������\�������邽��*/
	std::vector<ComponentPtr> destroyList(
		std::move_iterator(destCompBegin_itr), std::move_iterator(components.end()));

	// �z�񂩂�ړ��ς݃R���|�[�l���g���폜
	components.erase(destCompBegin_itr, components.end());

	// �j���\��̃R���|�[�l���g��OnDestory�����s
	const Component* rigid = 
		static_cast<Component*>(rigidbody);
	for (auto& e : destroyList) {
		e->OnDestroy();

		// ���W�b�h�{�f�B�R���|�[�l���g�|�C���^�Ɠ����Ȃ�null�ɂ���
		if (e.get() == rigid) {
			rigidbody = nullptr;
		}
	}
	destroyList.clear();

	// �폜����Ă��郁�b�V���R���|�[�l���g���폜����
	const auto& rBegin_itr = 
		std::partition(renderers.begin(), renderers.end(),
		[](const RendererPtr& p) { return !p->IsDestroyed(); });
	renderers.erase(rBegin_itr, renderers.end());

	// �폜����Ă���R���C�_�[�R���|�[�l���g���폜����
	const auto& cBegin_itr = 
		std::stable_partition(colliders.begin(), colliders.end(),
		[](const ColliderPtr& p) {return !p->IsDestroyed(); });
	colliders.erase(cBegin_itr, colliders.end());
}


/**
* �X�^�[�g�C�x���g
*/
void GameObject::Start()
{
	for (int i = 0; i < components.size(); ++i) {
		if (components[i]->isActive) {
			if (!components[i]->isStarted) {
				components[i]->Start();
				components[i]->isStarted = true;
			}
		}
	}
}

/**
* �X�V�C�x���g
*
* @param deltaSeconds �O��̍X�V����̌o�ߎ���(�b)
*/
void GameObject::Update(float deltaSeconds)
{
	for (int i = 0; i < components.size(); ++i) {
		if (components[i]->isActive) {
			components[i]->Update(deltaSeconds);
		}
	}
	RemoveDestroyedComponent();
}

void GameObject::EndUpdate(float deltaSeconds)
{
	for (int i = 0; i < components.size(); ++i) {
		if (components[i]->isActive) {
			components[i]->EndUpdate(deltaSeconds, worldTransMat);
		}
	}
}

/**
* �Փ˃C�x���g
*
* @param self  �Փ˂����R���C�_�[(����)
* @param other �Փ˂����R���C�_�[(����)
* @param contactPoint �Փˏ���ێ�����\����
*/
void GameObject::OnCollisionHit(
	const ColliderPtr& self, const ColliderPtr& other, const ContactPoint& contactPoint)
{
	for (auto& e : components) {
		if (e->isActive) {
			e->OnCollisionHit(self, other, contactPoint);
		}
	}
}

/**
* �폜�C�x���g
*/
void GameObject::OnDestroy()
{
	for (auto& e : components) {
		e->Destroy();
		e->OnDestroy();
	}

	// �q�I�u�W�F�N�g���폜����
	for (auto& child : children) {
		child->Destroy();;
	}
}

/**
* ImGui�ɏ����o��
*/
void GameObject::DrawImGui()
{
	ImGui::SeparatorText("GameObject");

	// ���O�\��
	ImGui::Text(name.GetName().c_str());
	ImGui::Spacing();

	// ������Ԃ̗L���i�`�F�b�N�F���I�����W�ɂ��Ă���j
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
	ImGui::Checkbox("Active", &isActive);
	ImGui::PopStyleColor();
	ImGui::Spacing();

	// �ۗL�^�O�\��
	if (ImGui::TreeNode("Tags")) {
		for (int i = 0; i < tags.size(); ++i) {
			ImGui::Text("%d: %s", i, tags[i].GetName().c_str());
		}
		ImGui::TreePop();
	}
	ImGui::Spacing();

	// �\��������W�n�ݒ�
	static int transformSpace = 0;
	ImGui::RadioButton("Local", &transformSpace, 0); 
	ImGui::SameLine();
	ImGui::RadioButton("World", &transformSpace, 1);
	
	// ���W
	if (transformSpace == 0) {
		// ���[�J�����W�n
		ImGui::DragFloat3("Position", &position.x);
		vec3 r = radians_to_degrees(rotation);
		ImGui::DragFloat3("Rotation", &r.x);
		rotation = degrees_to_radians(r);
		ImGui::DragFloat3("Scale", &scale.x);
	}
	else {
		// ���[���h���W�n
		vec3 pos, rot, sce;
		Decompose(worldTransMat, pos, rot, sce);

		ImGui::DragFloat3("Position ", &pos.x);
		vec3 r = radians_to_degrees(rot);
		ImGui::DragFloat3("Rotation ", &r.x);
		rot = degrees_to_radians(r);
		ImGui::DragFloat3("Scale ", &sce.x);

		worldTransMat = TransformMatrix(pos, rot, sce);
	}
	ImGui::Spacing();

	// �F
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::Spacing();

	// �����_�[�L���[
	const char* RQNames[] = {
		"StoneShadow",
		"Geometry",
		"Sprcular",
		"Effect",
		"Transparent",
		"Overlay",
	};
	ImGui::Combo("RenderQueue", &renderQueue, RQNames, IM_ARRAYSIZE(RQNames));
	ImGui::Spacing();

	// ���C���[
	const char* LNames[] = {
		"Default",
		"UI",
	};
	ImGui::Combo("Layer", &layer, LNames, IM_ARRAYSIZE(LNames));
	ImGui::Spacing();

	// �R���|�[�l���g��\��
	ImGui::SeparatorText("Component");
	for (int i = 0; i < components.size(); ++i) {
		Component& comp = *components[i];
		if (ImGui::TreeNode((std::to_string(i) + ":" + comp.name).c_str())) {

			// ������Ԃ̗L���i�`�F�b�N�F���I�����W�ɂ��Ă���j
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
			ImGui::Checkbox("Active", &comp.isActive);
			ImGui::PopStyleColor();
			ImGui::Spacing();

			// �R���|�[�l���g���\��
			components[i]->DrawImGui();

			ImGui::SeparatorText("");
			ImGui::TreePop();
		}
	}
}

