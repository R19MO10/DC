/**
* @file EventDispatcher.h
*/
#ifndef EVENTDISPATCHER_H_INCLUDED
#define EVENTDISPATCHER_H_INCLUDED
#include <functional>
#include <vector>

// イベント型
template<typename... Args>
using Event = std::function<void(Args...)>;

// イベントディスパッチャー
template<typename... Args>
class EventDispatcher 
{
private:
    // イベント配列
    std::vector<Event<Args...>> eventList;

public:
    EventDispatcher() = default;
    virtual ~EventDispatcher() = default;

    // イベントを登録する
    void AddEvent(const Event<Args...>& event) {
        eventList.push_back(event);
    }

    // イベントを発火する
    void DispatchEvent(Args... args) {
        for (const auto& e : eventList) {
            e(args...);
        }
    }
};

#endif // EVENTDISPATCHER_H_INCLUDED