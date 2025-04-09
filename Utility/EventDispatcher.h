/**
* @file EventDispatcher.h
*/
#ifndef EVENTDISPATCHER_H_INCLUDED
#define EVENTDISPATCHER_H_INCLUDED
#include <functional>
#include <vector>

// �C�x���g�^
template<typename... Args>
using Event = std::function<void(Args...)>;

// �C�x���g�f�B�X�p�b�`���[
template<typename... Args>
class EventDispatcher 
{
private:
    // �C�x���g�z��
    std::vector<Event<Args...>> eventList;

public:
    EventDispatcher() = default;
    virtual ~EventDispatcher() = default;

    // �C�x���g��o�^����
    void AddEvent(const Event<Args...>& event) {
        eventList.push_back(event);
    }

    // �C�x���g�𔭉΂���
    void DispatchEvent(Args... args) {
        for (const auto& e : eventList) {
            e(args...);
        }
    }
};

#endif // EVENTDISPATCHER_H_INCLUDED