#ifndef EVENTABLE_H
#define EVENTABLE_H


class Eventable
{
public:
    void on(const std::string& event, Callback callback, TestCallback test = nullptr);

    void one(const std::string& event, Callback callback, TestCallback test = nullptr);

    void off(const std::string& event, Callback callback);

protected:
    template<class Class, typename CallbackSignature>
    constexpr static void register(const char* event)
    {
        Eventable::events Event<Class, CallbackSignature>::
    }

    template<typename ...Args>
    void trigger(const std::string& event, Args&&... args)
    {
        this->_trigger<>(std::forward<Args>(args)...);
    }

private:

};

#endif
