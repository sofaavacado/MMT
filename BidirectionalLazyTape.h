#pragma once

#include "Sequence.h"
#include <unordered_map>  
#include <set>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <optional>
#include <sstream>
#include <array>
#include <limits> 
#include <algorithm> 

template <typename T>
class BidirectionalLazyTape {
private:
    // Структура для хранения ячейки 
    struct Cell {
        T value; // Значение в ячейке
        bool is_modified;  // Флаг, была ли ячейка изменена пользователем

        Cell() : value(T()), is_modified(false) {}
        Cell(const T& val, bool modified = false) : value(val), is_modified(modified) {}
    };

   // Хэш-таблица для хранения ячеек
    mutable std::unordered_map<int, Cell> cells;
    T blank_symbol; // символ пустой ячейки
    std::string initial_input; // входная строка

public:
    BidirectionalLazyTape(T blank = T())
        : blank_symbol(blank) {
    }

    T Get(int index) const {
        // Ищем ячейку в хэш-таблице
        auto it = cells.find(index);

        if (it != cells.end()) {
            return it->second.value;
        }

        // Ленивая материализация: создаём ячейку при первом обращении
        T value;
        if (index >= 0 && index < static_cast<int>(initial_input.length())) {
            value = initial_input[index];
        }
        else {
            value = blank_symbol;
        }

        // Сохраняем в хэш-таблицу с пометкой "не модифицирована"
        cells[index] = Cell(value, false);
        return value;
    }

    void Set(int index, T value) {
        auto it = cells.find(index);

        if (it != cells.end()) {
            it->second.value = value;
            it->second.is_modified = true;
        }
        else {
            // Создаём новую ячейку с пометкой "модифицирована"
            cells[index] = Cell(value, true);
        }
    }

    void Initialize(const std::string& input) {
        cells.clear(); 
        initial_input = input;

        for (size_t i = 0; i < input.length(); ++i) {
            cells[static_cast<int>(i)] = Cell(input[i], false);
        }
    }

    size_t GetMaterializedCount() const {
        return cells.size();
    }

    // получить количество модифицированных ячеек
    size_t GetModifiedCount() const {
        size_t count = 0;
        for (const auto& pair : cells) {
            if (pair.second.is_modified) {
                ++count;
            }
        }
        return count;
    }

    std::string GetContent(int from, int to) const {
        std::string result;
        for (int i = from; i <= to; ++i) {
            result += Get(i);
        }
        return result;
    }

    int GetMinIndex() const {
        if (cells.empty()) return 0;

        int min_index = std::numeric_limits<int>::max();
        for (const auto& pair : cells) {
            if (pair.first < min_index) {
                min_index = pair.first;
            }
        }
        return min_index;
    }

    int GetMaxIndex() const {
        if (cells.empty()) return 0;

        int max_index = std::numeric_limits<int>::min();
        for (const auto& pair : cells) {
            if (pair.first > max_index) {
                max_index = pair.first;
            }
        }
        return max_index;
    }

    void ClearMaterialized() {
        cells.clear();
    }

    // получить все индексы в отсортированном порядке
    Sequence<int> GetSortedIndices() const {
        Sequence<int> indices;

        for (const auto& pair : cells) {
            indices.Append(pair.first);  
        }

        int* data = indices.GetData();
        std::sort(data, data + indices.GetSize());
        return indices;
    }
};
