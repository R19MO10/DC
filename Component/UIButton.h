/**
* @file UIButton.h
*/
#ifndef UIBUTTON_H_INCLUDED
#define UIBUTTON_H_INCLUDED
#include "UILayout.h"
#include <functional>

/**
* UI�p�̃{�^������R���|�[�l���g
*/
class UIButton : public UILayout
{
public:
	bool interactable = true; // �}�E�X�ɔ������Atrue=���� false=���Ȃ�
	vec3 normalColor = { 1.0f, 1.0f, 1.0f };		// �ʏ�F
	vec3 highlightedColor = { 1.3f, 1.3f, 1.3f };	// �I��F
	vec3 pressedColor = { 0.7f, 0.7f, 0.7f };		// �����F

	// �f���Q�[�g�i
	/* std::function<�߂�^(�������X�g)>;�@
		function�^�ɂ͊֐��A�֐��I�u�W�F�N�g�A�����_���������ĕϐ��Ƃ��Ĉ������Ƃ��ł���j*/
	using ClickEvent = std::function<void(UIButton*)>;
	std::vector<ClickEvent> onClick;

private:
	bool prevButtonDown = false; // �O��X�V���̃}�E�X���{�^��������

public:
	UIButton() 
		:UILayout("UIButton")
	{};
	virtual ~UIButton() = default;

public:
	virtual void Update(float deltaSeconds) override;
};
using UIButtonPtr = std::shared_ptr<UIButton>;


#endif // UIBUTTON_H_INCLUDED