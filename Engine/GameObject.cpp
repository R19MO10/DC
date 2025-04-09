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
* デストラクタ
*/
GameObject::~GameObject()
{
	// 死ぬ前に親子関係を解除する
	SetParent(nullptr);
	for (auto child : children) {
		child->parent = nullptr;
	}

	// 「リジッドボディ」と「UIレイアウト」コンポーネントを
	// nullにしておく
	rigidbody = nullptr;
}


// タグを設定
void GameObject::AddTag(const char* tag)
{
	MyFName ftag(tag);

	// 無名のタグなら処理しない
	if (ftag == MyFName()) {
		return;
	}

	const auto& someTag_itr =
		std::find_if(tags.begin(), tags.end(),
			[&ftag](const MyFName t) { return t == ftag; });
	// 同じタグ名がすでにあれば処理しない
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


// 親オブジェクトを設定する
void GameObject::SetParent(GameObject* parent)
{
	// 自身を設定されたら親を削除する
	if (parent == this) {
		parent = nullptr;
		return;
	}

	// 同じ親を指定された場合は何もしない
	if (parent == this->parent) {
		return;
	}

	// 別の親がある場合、その親との関係を解除する
	if (this->parent) {
		// 自分の位置を検索
		auto& c = this->parent->children;
		const auto& itr = std::find(c.begin(), c.end(), this);
		if (itr != c.end()) {
			c.erase(itr); // 配列から自分を削除
		}
	}

	// 新たな親子関係を設定
	if (parent) {
		parent->children.push_back(this);
	}

	this->parent = parent;
}

// 親オブジェクトを設定する
void GameObject::SetParent(const GameObjectPtr& parent)
{
	SetParent(parent.get());
}

/**
* ゲームオブジェクトから削除予定のコンポーネントを削除する
*/
void GameObject::RemoveDestroyedComponent()
{
	if (components.empty()) {
		return; // コンポーネントを持っていなければ何もしない
	}

	if (isDestroyed) {
		// 自身が削除されているなら処理しない(デストラクタで削除されるため)
		return;
	}

	// コンポーネント配列の破棄
	// 破棄予定の有無でコンポーネントを分ける	/*配列の前に条件を満たす(破棄予定ではない)要素を集める*/
	const auto& destCompBegin_itr = std::stable_partition(components.begin(), components.end(),
		[](const ComponentPtr& p) { return !p->IsDestroyed(); });

	if (destCompBegin_itr == components.end()) {
		return;	// 削除予定のコンポーネントがない
	}

	// 破棄予定のコンポーネントを別の配列に移動
	/*OnDestroy呼び出し時に新しいコンポーネントが追加される可能性があるため*/
	std::vector<ComponentPtr> destroyList(
		std::move_iterator(destCompBegin_itr), std::move_iterator(components.end()));

	// 配列から移動済みコンポーネントを削除
	components.erase(destCompBegin_itr, components.end());

	// 破棄予定のコンポーネントのOnDestoryを実行
	const Component* rigid = 
		static_cast<Component*>(rigidbody);
	for (auto& e : destroyList) {
		e->OnDestroy();

		// リジッドボディコンポーネントポインタと同じならnullにする
		if (e.get() == rigid) {
			rigidbody = nullptr;
		}
	}
	destroyList.clear();

	// 削除されているメッシュコンポーネントを削除する
	const auto& rBegin_itr = 
		std::partition(renderers.begin(), renderers.end(),
		[](const RendererPtr& p) { return !p->IsDestroyed(); });
	renderers.erase(rBegin_itr, renderers.end());

	// 削除されているコライダーコンポーネントを削除する
	const auto& cBegin_itr = 
		std::stable_partition(colliders.begin(), colliders.end(),
		[](const ColliderPtr& p) {return !p->IsDestroyed(); });
	colliders.erase(cBegin_itr, colliders.end());
}


/**
* スタートイベント
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
* 更新イベント
*
* @param deltaSeconds 前回の更新からの経過時間(秒)
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
* 衝突イベント
*
* @param self  衝突したコライダー(自分)
* @param other 衝突したコライダー(相手)
* @param contactPoint 衝突情報を保持する構造体
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
* 削除イベント
*/
void GameObject::OnDestroy()
{
	for (auto& e : components) {
		e->Destroy();
		e->OnDestroy();
	}

	// 子オブジェクトも削除する
	for (auto& child : children) {
		child->Destroy();;
	}
}

/**
* ImGuiに情報を出力
*/
void GameObject::DrawImGui()
{
	ImGui::SeparatorText("GameObject");

	// 名前表示
	ImGui::Text(name.GetName().c_str());
	ImGui::Spacing();

	// 活動状態の有無（チェック色をオレンジにしている）
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
	ImGui::Checkbox("Active", &isActive);
	ImGui::PopStyleColor();
	ImGui::Spacing();

	// 保有タグ表示
	if (ImGui::TreeNode("Tags")) {
		for (int i = 0; i < tags.size(); ++i) {
			ImGui::Text("%d: %s", i, tags[i].GetName().c_str());
		}
		ImGui::TreePop();
	}
	ImGui::Spacing();

	// 表示する座標系設定
	static int transformSpace = 0;
	ImGui::RadioButton("Local", &transformSpace, 0); 
	ImGui::SameLine();
	ImGui::RadioButton("World", &transformSpace, 1);
	
	// 座標
	if (transformSpace == 0) {
		// ローカル座標系
		ImGui::DragFloat3("Position", &position.x);
		vec3 r = radians_to_degrees(rotation);
		ImGui::DragFloat3("Rotation", &r.x);
		rotation = degrees_to_radians(r);
		ImGui::DragFloat3("Scale", &scale.x);
	}
	else {
		// ワールド座標系
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

	// 色
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::Spacing();

	// レンダーキュー
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

	// レイヤー
	const char* LNames[] = {
		"Default",
		"UI",
	};
	ImGui::Combo("Layer", &layer, LNames, IM_ARRAYSIZE(LNames));
	ImGui::Spacing();

	// コンポーネントを表示
	ImGui::SeparatorText("Component");
	for (int i = 0; i < components.size(); ++i) {
		Component& comp = *components[i];
		if (ImGui::TreeNode((std::to_string(i) + ":" + comp.name).c_str())) {

			// 活動状態の有無（チェック色をオレンジにしている）
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
			ImGui::Checkbox("Active", &comp.isActive);
			ImGui::PopStyleColor();
			ImGui::Spacing();

			// コンポーネント情報表示
			components[i]->DrawImGui();

			ImGui::SeparatorText("");
			ImGui::TreePop();
		}
	}
}

