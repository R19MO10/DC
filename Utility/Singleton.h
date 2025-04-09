/**
* @file Singleton.h
*/
#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED
#include "../Engine/DebugLog.h"
#include <memory>
#include <cassert>

/**
* �V���O���g���p�e���v���[�g
* 
* @tparam Class �V���O���g���ɂ������ΏۃN���X
*/
template<typename Class>
class Singleton 
{
private:
	static std::unique_ptr<Class> instance;

protected:
	// �C���X�^���X�̐����ƍ폜���O�����琧��
	Singleton() = default;
	virtual ~Singleton() = default;

	// �V���O���g���C���X�^���X�𐶐�
	static void Create() {
		// �C���X�^���X�����łɂ���ꍇ�x�����o��
		assert(!instance);
		if (instance) {
			LOG_WARNING("SingletonInstance���d����������܂���");
			return;
		}

		instance.reset(new Class());
	}

public:
	// �R�s�[�Ƒ�����֎~
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;


	// �V���O���g���C���X�^���X���擾
	static constexpr Class& GetInstance() {
		if (!instance) {
			Create();
		}
		return *instance;
	}


	// �V���O���g���C���X�^���X��C�ӂŔj��
	static void DestroyInstance() {
		instance.reset();
	}
};

template <typename Class>
std::unique_ptr<Class> Singleton<Class>::instance = nullptr;

#endif // SINGLETON_H_INCLUDED