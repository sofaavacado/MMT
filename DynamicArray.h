#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <algorithm>
#include <initializer_list>
#include <utility>
#include <stdexcept>
#include <limits>
#include "exceptions.h"

template <typename T>
class DynamicArray {
protected:
    T* items;
    size_t size_;
    size_t capacity_;

    void ensureCapacity(size_t newSize) {
        if (newSize > capacity_) {
            size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
            while (newCapacity < newSize) newCapacity *= 2;
            T* newItems = new T[newCapacity];
            if (items != nullptr) {
                std::copy(items, items + size_, newItems);
                delete[] items;
            }
            items = newItems;
            capacity_ = newCapacity;
        }
    }

public:
    DynamicArray() : items(nullptr), size_(0), capacity_(0) {}

    DynamicArray(T* items, size_t count) : size_(count), capacity_(count) {
        if (count > std::numeric_limits<size_t>::max())
            throw IndexOutOfRangeException("Size too large");

        if (count == 0) {
            this->items = nullptr;
            return;
        }
        this->items = new T[count];
        std::copy(items, items + count, this->items);
    }

    DynamicArray(size_t size) : size_(size), capacity_(size) {
        if (size > std::numeric_limits<size_t>::max())
            throw IndexOutOfRangeException("Size too large");

        if (size == 0) {
            items = nullptr;
            return;
        }
        items = new T[size];
    }

    DynamicArray(const DynamicArray& other) : size_(other.size_), capacity_(other.size_) {
        if (other.size_ == 0) {
            items = nullptr;
            return;
        }
        items = new T[size_];
        std::copy(other.items, other.items + size_, items);
    }

    DynamicArray(std::initializer_list<T> init) : size_(init.size()), capacity_(init.size()) {
        if (size_ == 0) {
            items = nullptr;
            return;
        }
        items = new T[size_];
        std::copy(init.begin(), init.end(), items);
    }

    virtual ~DynamicArray() {
        delete[] items;
    }

    T Get(size_t index) const {
        if (index >= size_) throw IndexOutOfRangeException("Index out of range");
        return items[index];
    }

    size_t GetSize() const { return size_; }
    size_t GetCapacity() const { return capacity_; }

    void Set(size_t index, const T& value) {
        if (index >= size_) throw IndexOutOfRangeException("Index out of range");
        items[index] = value;
    }

    void Resize(size_t newSize) {
        if (newSize > std::numeric_limits<size_t>::max())
            throw IndexOutOfRangeException("Size too large");

        if (newSize == size_) return;

        ensureCapacity(newSize);

        if (newSize > size_) {
            for (size_t i = size_; i < newSize; ++i) {
                items[i] = T();
            }
        }

        size_ = newSize;
    }

    T& operator[](size_t index) {
        if (index >= size_) throw IndexOutOfRangeException("Index out of range");
        return items[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) throw IndexOutOfRangeException("Index out of range");
        return items[index];
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] items;
            size_ = other.size_;
            capacity_ = other.size_;

            if (other.size_ == 0) {
                items = nullptr;
                return *this;
            }

            items = new T[size_];
            std::copy(other.items, other.items + size_, items);
        }
        return *this;
    }

    DynamicArray(DynamicArray&& other) noexcept
        : items(other.items), size_(other.size_), capacity_(other.capacity_) {
        other.items = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] items;
            items = other.items;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.items = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    bool IsEmpty() const { return size_ == 0; }

    T* GetData() { return items; }
    const T* GetData() const { return items; }

    void Clear() {
        delete[] items;
        items = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void Append(const T& value) {
        Resize(size_ + 1);
        items[size_ - 1] = value;
    }

    void Append(T&& value) {
        Resize(size_ + 1);
        items[size_ - 1] = std::move(value);
    }

    void RemoveLast() {
        if (size_ > 0) {
            Resize(size_ - 1);
        }
    }
};

#endif // DYNAMIC_ARRAY_H