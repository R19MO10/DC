/**
* @file Singleton.h
*/
#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED
#include "../Engine/DebugLog.h"
#include <memory>
#include <cassert>

/**
* シングルトン用テンプレート
* 
* @tparam Class シングルトンにしたい対象クラス
*/
template<typename Class>
class Singleton 
{
private:
	static std::unique_ptr<Class> instance;

protected:
	// インスタンスの生成と削除を外部から制御
	Singleton() = default;
	virtual ~Singleton() = default;

	// シングルトンインスタンスを生成
	static void Create() {
		// インスタンスがすでにある場合警告を出す
		assert(!instance);
		if (instance) {
			LOG_WARNING("SingletonInstanceが重複生成されました");
			return;
		}

		instance.reset(new Class());
	}

public:
	// コピーと代入を禁止
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;


	// シングルトンインスタンスを取得
	static constexpr Class& GetInstance() {
		if (!instance) {
			Create();
		}
		return *instance;
	}


	// シングルトンインスタンスを任意で破棄
	static void DestroyInstance() {
		instance.reset();
	}
};

template <typename Class>
std::unique_ptr<Class> Singleton<Class>::instance = nullptr;

#endif // SINGLETON_H_INCLUDED