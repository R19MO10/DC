/**
* @file IInput.h
*/
#ifndef IINPUT_H_INCLUDED
#define IINPUT_H_INCLUDED

// 先行宣言
struct GLFWwindow;

/**
* インプット作成インターフェース
*/
class IInput 
{
public:
	virtual void StartUp(GLFWwindow* window) = 0;
	virtual void UpdeteStates(float deltaSeconds) = 0;
};

#endif // IINPUT_H_INCLUDED
