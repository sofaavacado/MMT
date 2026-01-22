#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "DynamicArray.h"

template <typename T>
class Sequence : public DynamicArray<T> {
public:
    using DynamicArray<T>::DynamicArray;

    Sequence() : DynamicArray<T>() {}

    Sequence(size_t size) : DynamicArray<T>(size) {}

    Sequence(std::initializer_list<T> init) : DynamicArray<T>(init) {}

    T GetFirst() const {
        if (this->size_ == 0) throw IndexOutOfRangeException("Sequence is empty");
        return this->items[0];
    }

    T GetLast() const {
        if (this->size_ == 0) throw IndexOutOfRangeException("Sequence is empty");
        return this->items[this->size_ - 1];
    }

    Sequence<T> GetSubsequence(size_t startIndex, size_t endIndex) const {
        if (startIndex >= this->size_ || endIndex >= this->size_ || startIndex > endIndex)
            throw IndexOutOfRangeException("Invalid subsequence indices");

        size_t subSize = endIndex - startIndex + 1;
        T* subItems = new T[subSize];
        std::copy(this->items + startIndex, this->items + endIndex + 1, subItems);

        Sequence<T> result(subItems, subSize);
        delete[] subItems;
        return result;
    }

    void Prepend(const T& item) {
        this->Resize(this->size_ + 1);

        for (size_t i = this->size_ - 1; i > 0; --i) {
            this->items[i] = this->items[i - 1];
        }

        this->items[0] = item;
    }

    void InsertAt(size_t index, const T& item) {
        if (index > this->size_) throw IndexOutOfRangeException("Insert index out of range");

        if (index == 0) {
            Prepend(item);
            return;
        }

        if (index == this->size_) {
            this->Append(item);
            return;
        }

        this->Resize(this->size_ + 1);

        for (size_t i = this->size_ - 1; i > index; --i) {
            this->items[i] = this->items[i - 1];
        }

        this->items[index] = item;
    }

    void RemoveAt(size_t index) {
        if (index >= this->size_) throw IndexOutOfRangeException("Remove index out of range");

        for (size_t i = index; i < this->size_ - 1; ++i) {
            this->items[i] = this->items[i + 1];
        }

        this->Resize(this->size_ - 1);
    }

    // Удаление диапазона элементов
    void RemoveRange(size_t first, size_t last) {
        if (first >= last || last > this->size_) return;
        size_t count = last - first;
        for (size_t i = 0; i < count; ++i) {
            RemoveAt(first);
        }
    }

    int Find(const T& item) const {
        for (size_t i = 0; i < this->size_; ++i) {
            if (this->items[i] == item) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    bool Contains(const T& item) const {
        return Find(item) != -1;
    }

    void Replace(size_t index, const T& item) {
        this->Set(index, item);
    }

    void Swap(size_t index1, size_t index2) {
        if (index1 >= this->size_ || index2 >= this->size_)
            throw IndexOutOfRangeException("Swap indices out of range");

        if (index1 != index2) {
            std::swap(this->items[index1], this->items[index2]);
        }
    }

    Sequence<T> Concatenate(const Sequence<T>& other) const {
        size_t newSize = this->size_ + other.size_;
        T* newItems = new T[newSize];

        std::copy(this->items, this->items + this->size_, newItems);
        std::copy(other.items, other.items + other.size_, newItems + this->size_);

        Sequence<T> result(newItems, newSize);
        delete[] newItems;
        return result;
    }

    Sequence<T> operator+(const Sequence<T>& other) const {
        return Concatenate(other);
    }

    Sequence<T> Copy() const {
        return Sequence<T>(*this);
    }
};

#endif // SEQUENCE_H
