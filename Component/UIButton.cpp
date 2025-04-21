/**
* @file UIButton.cpp
*/
#include "UIButton.h"

#include "../Engine/Engine.h"
#include "../Engine/Input/InputManager.h"

/**
* �X�V�C�x���g
*
* @param deltaSeconds �O��̍X�V����̌o�ߎ���(�b)
*/
void UIButton::Update(float deltaSeconds)
{
	// ���N���X�̃����o�֐������s
	UILayout::Update(deltaSeconds);

	GameObject* owner = GetOwner();
	Engine* engine = owner->GetEngine();

	// �{�^���̗L���E��������
	if (!interactable) {
		std::copy_n(&normalColor.x, 3, &owner->color.x);
		owner->color.w = 0.75f; // �����ۂ����o�����ߏ��������ɂ���
		return;		
	}

	Mouse* mouse = InputManager::GetMouse();

	//�}�E�X���W���擾
	const vec2 mousePos = mouse->GetMousePosition();

	if (abs(mousePos.x - owner->position.x) < owner->scale.x &&	/*abs() ��Βl���ɕϊ�*/
		abs(mousePos.y - owner->position.y) < owner->scale.y) {
		// �J�[�\�����{�^���̈���ɂ���
		const bool buttonDown = mouse->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		if (buttonDown) {
			// ���{�^����������Ă���
			//�F�������ꂽ���̐F�ɂ���
			std::copy_n(&pressedColor.x, 3, &owner->color.x);	/*�C�e���[�^�͈�[first, first + n)���R�s�[����*/
		}
		else {
			// ���{�^����������Ă���
			//�F�𗣂��ꂽ���̐F�ɂ���
			std::copy_n(&highlightedColor.x, 3, &owner->color.x);

			// �{�^���N���b�N����
			if (prevButtonDown) {
				// �f���Q�[�g�����s
				for (auto& e : onClick) {
					e(this);
				}
			}
		} // if buttonDown

		// �}�E�X�{�^���̏�Ԃ��X�V
		prevButtonDown = buttonDown;
	}
	else {
		// �J�[�\�����{�^���̈�O�ɂ���
		//�F��ʏ�ɖ߂�
		std::copy_n(&normalColor.x, 3, &owner->color.x);
		prevButtonDown = false;
	}
}