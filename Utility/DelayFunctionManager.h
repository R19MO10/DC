/**
* @file DelayFunctionManager.h
*/
#ifndef DELAY_FUNCTION_MANAGER_H_INCLUDED
#define DELAY_FUNCTION_MANAGER_H_INCLUDED
#include "Singleton.h"
#include <vector>
#include <utility>
#include <functional>
#include <algorithm>


/**
* �w�莞�Ԍ�Ɏ��s����֐����Ǘ�����N���X
* 
* ���w�肷��֐��̃C���X�^���X�͕K���֐����s���ɂ���K�v������
*/
class DelayManager : public Singleton<DelayManager>
{
    friend class Engine;

private:
    /* float:�֐������s����܂ł̎���, std::function<void()>:���s����֐� */
    using DelayFunc = std::pair<float, std::function<void()>>;

    std::vector<DelayFunc> functions;   // �x���֐��z��

public:
    /**
    * �w�莞�Ԍ���s�֐���ǉ�����
    *
    * @param delayTime_s �x�����Ԃ�b�P�ʂŎw��
    * @param func        �x����ɌĂяo���֐��ւ̎Q��
    * @param args        func�ɓn�������̉ϒ�����
    */
    template <typename Function, typename... Args>
    static void AddDelayFunction(float delayTime_s, Function&& func, Args&&... args) {
        // �C���X�^���X�������g�ݍ��킹�Ċ֐����쐬
        auto boundFunction =
            std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

        // �x���֐��z��ɒǉ�
        DelayManager::GetInstance().AddFunc(DelayFunc(delayTime_s, boundFunction));
    }

    /* �g�p��

    // �����o�֐�
    // obj�͂��̊֐��������o�֐��̂��߃C���X�^���X���K�v
    MyClass obj;
    DelayManager::AddDelayFunction(1.0f, MyClass::SomeMemberFunction, obj);

    // ���ڒ�`�֐�
    DelayManager::AddDelayFunction(2.0f, SomeFreeFunction);
    DelayManager::AddDelayFunction(1.0f, SomeFreeFunction, arg1, arg2);

    // static�֐�
    DelayManager::AddDelayFunction(3.0f, MyClass::SomeStaticFunction);



    // �e�X�g�p�̊֐�
    void TestFunction(int a, float b) {
        std::cout << a << "," << b << std::endl;
    }

    int main() {
        // DelayManager �̃C���X�^���X���擾
        auto& delayManager = DelayManager::GetInstance();

        // �x���֐��̒ǉ�
        DelayManager::AddDelayFunction(1.0f, TestFunction, 42, 3.14f);
    }

    */

private:
    // �w�莞�Ԍ���s�֐���ǉ�����
    inline void AddFunc(const DelayFunc& func) {
        functions.push_back(func);
    }


    // �X�V����
    inline void Updete(float deltaSeconds) {
        for (auto& func : functions) {
            func.first -= deltaSeconds;

            // ���Ԃ�0�ɂȂ�����֐������s
            if (func.first <= 0) {
                func.second();
            }
        }

        // �܂��֐������s���ĂȂ��v�f��O�֏W�߂�
        const auto deleteFuncBegin_itr =
            std::partition(functions.begin(), functions.end(),
                [](const DelayFunc& e) { return e.first > 0; });

        if (deleteFuncBegin_itr == functions.end()) {
            return;	// �폜����v�f���Ȃ�
        }

        // �z�񂩂���s�ςݗv�f���폜
        functions.erase(deleteFuncBegin_itr, functions.end());
    }
};

#endif // DELAY_FUNCTION_MANAGER_H_INCLUDED
