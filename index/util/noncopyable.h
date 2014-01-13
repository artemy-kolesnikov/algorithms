#pragma once

class Noncopyable {
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);
};
