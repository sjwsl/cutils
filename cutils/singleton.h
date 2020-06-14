#pragma once

namespace cutils {

template<typename type>
class Singleton {
private:
    Singleton(const Singleton&);
    void operator = (const Singleton&);
public:
    Singleton() {}
    static type* GetInstance() {
        static type ins;
        return &ins;
    }
};

} // namespace cutils
