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
* 指定時間後に実行する関数を管理するクラス
* 
* ※指定する関数のインスタンスは必ず関数実行時にある必要がある
*/
class DelayManager : public Singleton<DelayManager>
{
    friend class Engine;

private:
    /* float:関数を実行するまでの時間, std::function<void()>:実行する関数 */
    using DelayFunc = std::pair<float, std::function<void()>>;

    std::vector<DelayFunc> functions;   // 遅延関数配列

public:
    /**
    * 指定時間後実行関数を追加する
    *
    * @param delayTime_s 遅延時間を秒単位で指定
    * @param func        遅延後に呼び出す関数への参照
    * @param args        funcに渡す引数の可変長引数
    */
    template <typename Function, typename... Args>
    static void AddDelayFunction(float delayTime_s, Function&& func, Args&&... args) {
        // インスタンスや引数を組み合わせて関数を作成
        auto boundFunction =
            std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

        // 遅延関数配列に追加
        DelayManager::GetInstance().AddFunc(DelayFunc(delayTime_s, boundFunction));
    }

    /* 使用例

    // メンバ関数
    // objはこの関数がメンバ関数のためインスタンスが必要
    MyClass obj;
    DelayManager::AddDelayFunction(1.0f, MyClass::SomeMemberFunction, obj);

    // 直接定義関数
    DelayManager::AddDelayFunction(2.0f, SomeFreeFunction);
    DelayManager::AddDelayFunction(1.0f, SomeFreeFunction, arg1, arg2);

    // static関数
    DelayManager::AddDelayFunction(3.0f, MyClass::SomeStaticFunction);



    // テスト用の関数
    void TestFunction(int a, float b) {
        std::cout << a << "," << b << std::endl;
    }

    int main() {
        // DelayManager のインスタンスを取得
        auto& delayManager = DelayManager::GetInstance();

        // 遅延関数の追加
        DelayManager::AddDelayFunction(1.0f, TestFunction, 42, 3.14f);
    }

    */

private:
    // 指定時間後実行関数を追加する
    inline void AddFunc(const DelayFunc& func) {
        functions.push_back(func);
    }


    // 更新処理
    inline void Updete(float deltaSeconds) {
        for (auto& func : functions) {
            func.first -= deltaSeconds;

            // 時間が0になったら関数を実行
            if (func.first <= 0) {
                func.second();
            }
        }

        // まだ関数を実行してない要素を前へ集める
        const auto deleteFuncBegin_itr =
            std::partition(functions.begin(), functions.end(),
                [](const DelayFunc& e) { return e.first > 0; });

        if (deleteFuncBegin_itr == functions.end()) {
            return;	// 削除する要素がない
        }

        // 配列から実行済み要素を削除
        functions.erase(deleteFuncBegin_itr, functions.end());
    }
};

#endif // DELAY_FUNCTION_MANAGER_H_INCLUDED
