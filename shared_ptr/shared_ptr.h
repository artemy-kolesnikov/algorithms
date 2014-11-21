#pragma once

#include <memory>
#include <atomic>

template <typename T>
class sp_counted {
public:
    explicit sp_counted(T *p) noexcept
        : shared_count(1),
          weak_count(1),
          ptr(p) {}

    void add_ref() noexcept {
        ++shared_count;
    }

    void release() noexcept {
        if (!--shared_count) {
            // Если последний shared_ptr удалился, удаляем объект
            delete ptr;
            if (!--weak_count) {
                // Если нет слабых ссылок, удаляем объект счётчика
                destroy();
            }
        }
    }

    void add_ref_weak() noexcept {
        ++weak_count;
    }

    void release_weak() noexcept {
        if (!--weak_count) {
            // Если последний weak_ptr удалился, удаляем объект счётчика
            // Т.к. shared_ptr тоже увеличивает weak_count при создании,
            // нет необходимости проверять значение shared_count
            destroy();
        }
    }

    size_t use_count() const noexcept {
        return shared_count.load();
    }

    // Попытка увеличить счётчик shared_count из weak_ptr
    // Потокобезопасен, lock-free
    void add_ref_lock() {
        // Сохраняем текущее значение shared_count
        size_t cur_value(shared_count.load());
        do {
            // Если счётчик сильных ссылок равен нулю (т.е. нет больше живых shared_ptr),
            // то новый shared_ptr создавать не из чего.
            if (!cur_value) {
                throw std::bad_weak_ptr();
            }
          // Пытаемся увеличить счётчик shared_count на единицу
          // Если в промежутке между сохранением shared_count в cur_value, shared_count изменился,
          // то операция compare_exchange_weak вернёт false, запишет новое значение shared_count в cur_value
          // и цикл повторится
        } while (shared_count.compare_exchange_weak(cur_value, cur_value + 1));
    }

private:
    void destroy() noexcept {
        delete this;
    }

private:
    // Счётчик ссылок shared_ptr
    std::atomic<size_t> shared_count;
    // Счётчик ссылок weak_ptr
    std::atomic<size_t> weak_count;
    T *ptr;
};

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
    friend class weak_ptr<T>;
public:
    shared_ptr() noexcept : ptr(nullptr), counted(nullptr) {}

    // excaption safe конструктор
    explicit shared_ptr(T *p) {
        std::unique_ptr<T> holder(p);
        // new может кинуть исключение, и если p не передать в unique_ptr,
        // память под p потеряется
        counted = new sp_counted<T>(holder.get());
        ptr = holder.release();
    }

    ~shared_ptr() noexcept {
        release();
    }

    shared_ptr(const shared_ptr<T> &other) noexcept : ptr(other.ptr), counted(other.counted) {
        add_ref();
    }

    shared_ptr(const weak_ptr<T> &p) : ptr(p.ptr), counted(p.counted) {
        if (counted) {
            // Пытаемся увеличить счётчик ссылок объекта
            // В случае неудачи сгенерируется исключение std::bad_weak_ptr()
            counted->add_ref_lock();
        } else {
            throw std::bad_weak_ptr();
        }
    }

    shared_ptr &operator=(const shared_ptr<T> &other) noexcept {
        // Освобождаем владение предыдущим указателем
        release();

        // Выполняем присваивание
        ptr = other.ptr;
        counted = other.counted;

        // Устанавливаем владение новым указателем
        add_ref();

        // Ура! Я не забыл вернуть *this!
        return *this;
    }

    T *get() const noexcept {
        return ptr;
    }

    size_t use_count() const noexcept {
        return counted != nullptr ? counted->use_count() : 0;
    }

private:
    void add_ref() noexcept {
        if (counted) {
            counted->add_ref();
        }
    }

    void release() noexcept {
        if (counted) {
            counted->release();
        }
    }

private:
    T *ptr;
    sp_counted<T> *counted;
};

template <typename T>
class weak_ptr {
    friend class shared_ptr<T>;
public:
    weak_ptr() noexcept : ptr(nullptr), counted(nullptr) {}

    weak_ptr(const weak_ptr &other) noexcept : ptr(other.ptr), counted(other.counted) {
        add_ref_weak();
    }

    weak_ptr(const shared_ptr<T> &p) noexcept : ptr(p.ptr), counted(p.counted) {
        add_ref_weak();
    }

    weak_ptr &operator=(const weak_ptr &other) noexcept {
        release_weak();

        ptr = other.ptr;
        counted = other.counted;

        add_ref_weak();

        return *this;
    }

    weak_ptr &operator=(const shared_ptr<T> &p) {
        release_weak();

        ptr = p.ptr;
        counted = p.counted;

        add_ref_weak();

        return *this;
    }

    // Пытаемся сделать shared_ptr. Для этого вызывается конструктор shared_ptr(const weak_ptr &amp;);
    // В случае невозможности создать shared_ptr возвращается пустой объект
    shared_ptr<T> lock() noexcept {
        try {
            return shared_ptr<T>(*this);
        } catch (const std::bad_weak_ptr &) {
            return shared_ptr<T>();
        }
    }

    size_t use_count() const noexcept {
        return counted != nullptr ? counted->use_count() : 0;
    }

private:
    void add_ref_weak() noexcept {
        if (counted) {
            counted->add_ref_weak();
        }
    }

    void release_weak() noexcept {
        if (counted) {
            counted->release_weak();
        }
    }

private:
    T *ptr;
    sp_counted<T> *counted;
};
