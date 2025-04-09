/**
* @file IInput.h
*/
#ifndef IINPUT_H_INCLUDED
#define IINPUT_H_INCLUDED

// ��s�錾
struct GLFWwindow;

/**
* �C���v�b�g�쐬�C���^�[�t�F�[�X
*/
class IInput 
{
public:
	virtual void StartUp(GLFWwindow* window) = 0;
	virtual void UpdeteStates(float deltaSeconds) = 0;
};

#endif // IINPUT_H_INCLUDED
