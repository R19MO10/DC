/**
* @file GameObject.h
*/
#ifndef GAMEOBJECT_H_INCLUDED
#define GAMEOBJECT_H_INCLUDED
#include "../Math/MyMath.h"
#include "DebugLog.h"
#include "Component.h"
#include <string>
#include <vector>
#include <memory>

// ��s�錾
class Engine;

class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>; // �Q�[���I�u�W�F�N�g�̃|�C���^
using GameObjectList = std::vector<GameObjectPtr>; // �Q�[���I�u�W�F�N�g�|�C���^�̔z��
using GameObject_WeakPtr = std::weak_ptr<GameObject>;	//�Q�[���I�u�W�F�N�g�̃E�B�[�N�|�C���^
using GameObject_WeakList = std::vector<GameObject_WeakPtr>;	//�Q�[���I�u�W�F�N�g�E�B�[�N�|�C���^�̔z��

class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
using RendererList = std::vector<RendererPtr>;
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;
using ColliderList = std::vector<ColliderPtr>;
class Rigidbody;

/**
* �`��̏���
*/
enum RenderQueue : uint8_t
{
	RenderQueue_stoneShadow,// �΂̉e���f�}�`

	RenderQueue_geometry,	// ��ʓI�Ȑ}�`
	RenderQueue_sprcular,	// �X�y�L�������f�}�`
	RenderQueue_effect,		// �G�t�F�N�g
	RenderQueue_transparent,// �������Ȑ}�`
	RenderQueue_overlay,	// UI, �S��ʃG�t�F�N�g�Ȃ�
};

/**
* ���C���[
*/
enum Layer
{
	Layer_Default,
	Layer_UI,
};


/**
* �Q�[���ɓo�ꂷ�邳�܂��܂ȃI�u�W�F�N�g��\����{�N���X
*/
class GameObject
{
	friend Engine;
public:
	bool isActive = true;	// �I�u�W�F�N�g�̊������

	MyFName name;				// �I�u�W�F�N�g��
	vec3 position = vec3(0);    // ���̂̈ʒu
	vec3 rotation = vec3(0);    // ���̂̉�]�p�x
	vec3 scale = vec3(1);		// ���̂̊g�嗦
	vec4 color = vec4(1);		// ���̂̐F

	int renderQueue = RenderQueue_geometry; // �`�揇
	int layer = Layer_Default;		// ���C���[

private:
	Engine* engine = nullptr;       // �G���W���̃A�h���X
	std::vector<MyFName> tags;		// �^�O��

	bool isDestroyed = false;       // ���S�t���O

	mat4 localTransMat = mat4(1);	// ���[�J�����W�ϊ��s��	
	mat4 worldTransMat = mat4(1);	// ���[���h���W�ϊ��s��

	GameObject* parent = nullptr;		// �e�I�u�W�F�N�g
	std::vector<GameObject*> children;  // �q�I�u�W�F�N�g

	ComponentList components;	// �R���|�[�l���g�z��
	RendererList renderers;		// �����_�[�z��
	ColliderList colliders;		// �R���C�_�[�z��

	Rigidbody* rigidbody = nullptr;	// ���W�b�h�{�f�B�R���|�[�l���g

public:
	GameObject() = default;
	virtual ~GameObject();

	// �R�s�[�Ƒ�����֎~
	GameObject(GameObject&) = delete;
	GameObject& operator=(GameObject&) = delete;


	// �^�O���擾
	inline MyFName GetTag(size_t index = 0) const { 
		if (index < tags.size()) {
			return tags[index];
		}

		return MyFName();
	}
	inline std::vector<MyFName> GetTags() const {
		return tags;
	}

	// �^�O��ǉ�����
	void AddTag(const char* tag);

	// �^�O���폜����
	void RemoveTag(size_t index);

	// �^�O��S�č폜����
	void CleanTags();


	/**
	* �e�I�u�W�F�N�g�̊�����Ԃ܂Œ��ׂāA���g��������Ԃł��邩���ׂ�
	*/
	inline bool CheckActive() const {
		if (isActive) {
			if (!parent) {
				// �e�����Ȃ��ꍇ
				return true;
			}
			return parent->CheckActive();
		}
		return false;
	}

	// �Q�[���G���W�����擾
	inline Engine* GetEngine() const { return engine; }

	// �Q�[���I�u�W�F�N�g���G���W������폜����
	inline void Destroy() { isDestroyed = true; }

	// �Q�[���I�u�W�F�N�g���j�󂳂�Ă�����true��Ԃ�
	inline bool IsDestroyed() const { return isDestroyed; }

	/*
	* �e�I�u�W�F�N�g���擾����
	*
	* @retval nullptr     �e�����Ȃ�
	* @retval nullptr�ȊO �e�I�u�W�F�N�g�̃A�h���X
	*/
	inline GameObject* GetParent() const { return parent; }

	/**
	* �e�I�u�W�F�N�g��ݒ肷��
	*
	* @param parent �e�ɂ���Q�[���I�u�W�F�N�g
	*               nullptr���w�肷��Ɛe�q�֌W����������
	*/
	void SetParent(GameObject* parent);
	void SetParent(const GameObjectPtr& parent);

	/**
	* �q�I�u�W�F�N�g�̐����擾����
	*
	* @return �q�I�u�W�F�N�g�̐�
	*/
	inline size_t GetChildCount() const { return children.size(); }

	/**
	* �q�I�u�W�F�N�g���擾����
	*
	* @param index �q�I�u�W�F�N�g�z��̓Y��
	*
	* @return index�Ԗڂ̎q�I�u�W�F�N�g
	*/
	inline GameObject* GetChild(size_t index) const {
		if (GetChildCount() <= index) {
			// �q�I�u�W�F�N�g�̐��ȏ�̒l���w�肳�ꂽ��null��Ԃ�
			return nullptr;
		}
		return children[index];
	}

	/**
	* �q�I�u�W�F�N�g��S�Ď擾����
	*/
	inline std::vector<GameObject*> GetChildren() const {
		return children;
	}

	// ���[�J�����W�ϊ��s����擾����
	inline const mat4& GetLocalTransformMatrix() const { return localTransMat; }

	// ���[���h���W�ϊ��s����擾����
	inline const mat4& GetWorldTransformMatrix() const { return worldTransMat; }

	// �R���C�_�[�z����擾����
	inline const ColliderList& GetColliders() const { return colliders; }

	// ���W�b�h�{�f�B�R���|�[�l���g���擾����
	inline Rigidbody* GetRigidbody() const { return rigidbody; }

	// �Q�[���I�u�W�F�N�g�ɃR���|�[�l���g��ǉ�����
	template<typename Comp>
	inline std::shared_ptr<Comp> AddComponent() {
		if constexpr (!std::is_base_of<Component, Comp>::value) {
			LOG_WARNING("�R���|�[�l���g�ł͂Ȃ��e���v���[�g���w�肳��܂���");
			return nullptr;
		}

		const auto& p = std::make_shared<Comp>();
		if (!p) {
			LOG_WARNING("�R���|�[�l���g�̒ǉ��Ɏ��s���܂���");
			return nullptr;
		}
		p->owner = this;

		if constexpr (std::is_base_of_v<Renderer, Comp>) {
			renderers.push_back(p);
		}

		if constexpr (std::is_base_of_v<Collider, Comp>) {
			colliders.push_back(p);
		}
		
		if constexpr (std::is_base_of_v<Rigidbody, Comp>) {
			if (!rigidbody) {
				rigidbody = p.get();
			}
			else {
				// ���łɃ��W�b�h�{�f�B�R���|�[�l���g���ݒ肳��Ă��邽��
				// �R���|�[�l���g��ǉ����Ȃ�
				LOG_WARNING("���W�b�h�{�f�B�R���|�[�l���g��1�����ǉ��ł��܂���");
				return nullptr;
			}
		}

		components.push_back(p);

		p->Awake();

		return p;
	}

	/**
	* �R���|�[�l���g����������
	*
	* �����̃R���|�[�l���g�����Q�[���I�u�W�F�N�g�ɑ΂��Ďg�p����ƁA
	* �����Ɏ��Ԃ�������ꍇ������̂Œ��ӁB
	*
	* @tparam T ��������R���|�[�l���g�̌^
	*
	* @retval nullptr�ȊO T�^�̍ŏ��̃R���|�[�l���g�ւ̃|�C���^
	* @retval nullptr     T�^�̃R���|�[�l���g�������Ă��Ȃ�
	*/
	template<typename Comp>
	inline std::shared_ptr<Comp> GetComponent() {
		if constexpr (!std::is_base_of<Component, Comp>::value) {
			LOG_WARNING("�R���|�[�l���g�ł͂Ȃ��e���v���[�g���w�肳��܂���");
			return nullptr;
		}

		for (const auto& e : components) {
			// shared_ptr�̏ꍇ��dynamic_cast�ł͂Ȃ�dynamic_pointer_cast���g��
			const auto& p = std::dynamic_pointer_cast<Comp>(e);
			if (p) {
				return p; // ���������R���|�[�l���g��Ԃ�
			}
		}
		return nullptr; // ������Ȃ������̂�nullptr��Ԃ�
	}

	/**
	* �R���|�[�l���g���擾����
	* 
	* @param index �v�f�ԍ�
	* 
	* @retval nullptr�ȊO �v�f�ԍ��̃R���|�[�l���g�ւ̃|�C���^
	* @retval nullptr	  �v�f�ԍ����͈͊O
	*/
	inline ComponentPtr GetComponent(const size_t& index) {
		// �C���f�b�N�X���R���|�[�l���g�̐��ȏ�Ȃ�nullptr��Ԃ�
		if (index < components.size()) {
			return components[index];
		}
		return nullptr;
	}

	// �Q�[���I�u�W�F�N�g����R���|�[�l���g���폜����
	void RemoveDestroyedComponent();

public:
	// �C�x���g����

	// �ŏ���Update�̒��O�ŌĂяo�����
	virtual void Start();

	// ���t���[��1��Ăяo�����
	virtual void Update(float deltaSeconds);

	// ���[���h���W�v�Z��̏���
	virtual void EndUpdate(float deltaSeconds);

	/**
	* �R���C�_�[�Փˎ��ɌĂяo�����
	*
	* @param self  �Փ˂����R���C�_�[(����)
	* @param other �Փ˂����R���C�_�[(����)
	* @param contactPoint �Փˏ���ێ�����\����
	*/
	virtual void OnCollisionHit(
		const ColliderPtr& self, const ColliderPtr& other, const ContactPoint& contactPoint);

	// �폜�����Ƃ��ɌĂяo�����
	virtual void OnDestroy();

	// ImGui�ɃQ�[���I�u�W�F�N�g����\������
	virtual void DrawImGui();
};

#endif // GAMEOBJECT_H_INCLUDED