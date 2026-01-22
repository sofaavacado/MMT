#ifndef MULTI_TAPE_TURING_MACHINE_H
#define MULTI_TAPE_TURING_MACHINE_H

#include "Sequence.h"
#include "exceptions.h"
#include "BidirectionalLazyTape.h"
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

class MultiTapeTuringMachine {
public:
    static constexpr size_t MAX_TAPES = 3;

    const BidirectionalLazyTape<char>* GetTape(size_t i) const {
        if (i >= active_tapes) throw InvalidTapeException();
        return &tapes[i];
    }

    struct Transition {
        std::string state_from;
        std::array<char, MAX_TAPES> read_symbols;
        std::string state_to;
        std::array<char, MAX_TAPES> write_symbols;
        std::array<int, MAX_TAPES> moves;

        Transition()
            : state_from(), state_to() {
            for (size_t i = 0; i < MAX_TAPES; ++i) {
                read_symbols[i] = 0;
                write_symbols[i] = 0;
                moves[i] = 0;
            }
        }

        Transition(const std::string& from,
            const std::array<char, MAX_TAPES>& read,
            const std::string& to,
            const std::array<char, MAX_TAPES>& write,
            const std::array<int, MAX_TAPES>& move)
            : state_from(from), read_symbols(read), state_to(to),
            write_symbols(write), moves(move) {
        }

        Transition(const std::string& from,
            char r0, char r1, char r2,
            const std::string& to,
            char w0, char w1, char w2,
            int m0, int m1, int m2)
            : state_from(from), state_to(to) {
            read_symbols[0] = r0;
            read_symbols[1] = r1;
            read_symbols[2] = r2;
            write_symbols[0] = w0;
            write_symbols[1] = w1;
            write_symbols[2] = w2;
            moves[0] = m0;
            moves[1] = m1;
            moves[2] = m2;
        }
    };

private:
    std::map<std::pair<std::string, std::array<char, MAX_TAPES>>, Transition> transitions;
    std::string current_state;
    std::array<int, MAX_TAPES> head_positions;
    std::array<BidirectionalLazyTape<char>, MAX_TAPES> tapes;
    std::string start_state;
    std::set<std::string> accept_states;
    std::set<std::string> all_states;
    char blank_symbol;
    size_t step_count;
    size_t max_steps;
    size_t active_tapes;

public:
    MultiTapeTuringMachine(const std::string& start,
        size_t num_tapes = 1,
        char blank = ' ',
        size_t max_steps_limit = 1000000)
        : current_state(start),
        blank_symbol(blank),
        start_state(start),
        step_count(0),
        max_steps(max_steps_limit),
        active_tapes(num_tapes) {

        if (num_tapes < 1 || num_tapes > MAX_TAPES) {
            throw std::invalid_argument("Number of tapes must be between 1 and 3");
        }

        for (size_t i = 0; i < MAX_TAPES; ++i) {
            tapes[i] = BidirectionalLazyTape<char>(blank);
            head_positions[i] = 0;
        }

        all_states.insert(start);
    }

    // Добавить переход (все ленты) 
    void AddTransition(const std::string& from,
        const std::array<char, MAX_TAPES>& read,
        const std::string& to,
        const std::array<char, MAX_TAPES>& write,
        const std::array<int, MAX_TAPES>& moves) {
        Transition t(from, read, to, write, moves);
        transitions[{from, read}] = t;
        all_states.insert(from);
        all_states.insert(to);
    }

    // Добавить переход (по одной ленте) 
    void AddTransitionForTape(const std::string& from,
        size_t tape_idx,
        char read_sym,
        const std::string& to,
        char write_sym,
        int move) {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }

        std::array<char, MAX_TAPES> read_array = GetDefaultReadArray();
        std::array<char, MAX_TAPES> write_array = GetDefaultWriteArray();
        std::array<int, MAX_TAPES> move_array = {};

        auto it = transitions.begin();
        while (it != transitions.end()) {
            if (it->first.first == from) {
                read_array = it->second.read_symbols;
                write_array = it->second.write_symbols;
                move_array = it->second.moves;
                transitions.erase(it);
                break;
            }
            ++it;
        }

        read_array[tape_idx] = read_sym;
        write_array[tape_idx] = write_sym;
        move_array[tape_idx] = move;

        AddTransition(from, read_array, to, write_array, move_array);
    }

    void SetAcceptState(const std::string& state) {
        accept_states.insert(state);
        all_states.insert(state);
    }

    void InitializeTape(size_t tape_idx, const std::string& input) {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        head_positions[tape_idx] = 0;
        tapes[tape_idx].Initialize(input);
    }

    void InitializeTapes(const Sequence<std::string>& inputs) {
        if (inputs.GetSize() != active_tapes) {
            throw std::invalid_argument("Number of inputs must match number of active tapes");
        }
        for (size_t i = 0; i < active_tapes; ++i) {
            InitializeTape(i, inputs[i]);
        }
    }

    bool ExecuteStep() {
        if (step_count >= max_steps) {
            throw std::runtime_error("Maximum steps exceeded");
        }

        std::array<char, MAX_TAPES> current_symbols;
        current_symbols.fill(blank_symbol);

        for (size_t i = 0; i < active_tapes; ++i) {
            current_symbols[i] = tapes[i].Get(head_positions[i]);
        }

        auto key = std::make_pair(current_state, current_symbols);
        auto it = transitions.find(key);

        if (it == transitions.end()) {
            return false;
        }

        const Transition& t = it->second;

        for (size_t i = 0; i < active_tapes; ++i) {
            tapes[i].Set(head_positions[i], t.write_symbols[i]);
            head_positions[i] += t.moves[i];
        }

        current_state = t.state_to;
        step_count++;
        return true;
    }

    bool Run() {
        while (true) {
            if (accept_states.count(current_state)) {
                return true;
            }
            if (!ExecuteStep()) {
                return false;
            }
        }
    }

    bool Run(size_t max_steps_override) {
        size_t local_step_count = 0;
        while (local_step_count < max_steps_override) {
            if (accept_states.count(current_state)) {
                return true;
            }
            if (!ExecuteStep()) {
                return false;
            }
            local_step_count++;
        }
        throw std::runtime_error("Maximum steps exceeded");
    }

    std::string GetTapeContent(size_t tape_idx, int from = -10, int to = 10) const {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        return tapes[tape_idx].GetContent(from, to);
    }

    int GetHeadPosition(size_t tape_idx) const {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        return head_positions[tape_idx];
    }

    std::array<int, MAX_TAPES> GetHeadPositions() const {
        return head_positions;
    }

    std::string GetCurrentState() const {
        return current_state;
    }

    size_t GetStepCount() const {
        return step_count;
    }

    bool IsAcceptState() const {
        return accept_states.count(current_state) > 0;
    }

    size_t GetMaterializedCellsCount(size_t tape_idx) const {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        return tapes[tape_idx].GetMaterializedCount();
    }

    // Получить количество модифицированных ячеек
    size_t GetModifiedCellsCount(size_t tape_idx) const {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        return tapes[tape_idx].GetModifiedCount();
    }

    size_t GetTotalMaterializedCellsCount() const {
        size_t total = 0;
        for (size_t i = 0; i < active_tapes; ++i) {
            total += tapes[i].GetMaterializedCount();
        }
        return total;
    }

    // получить общее количество модифицированных ячеек
    size_t GetTotalModifiedCellsCount() const {
        size_t total = 0;
        for (size_t i = 0; i < active_tapes; ++i) {
            total += tapes[i].GetModifiedCount();
        }
        return total;
    }

    void Reset(const Sequence<std::string>& new_inputs = Sequence<std::string>()) {
        current_state = start_state;
        step_count = 0;

        for (size_t i = 0; i < MAX_TAPES; ++i) {
            head_positions[i] = 0;
            tapes[i].ClearMaterialized();
        }

        if (!new_inputs.IsEmpty()) {
            InitializeTapes(new_inputs);
        }
    }

    std::string VisualizeTapes(int window_size = 10) const {
        std::stringstream ss;
        for (size_t i = 0; i < active_tapes; ++i) {
            ss << "Tape " << (i + 1) << ": ";

            int min_pos = tapes[i].GetMinIndex();
            int max_pos = tapes[i].GetMaxIndex();
            int start = std::min({ min_pos, head_positions[i] - window_size });
            int end = std::max({ max_pos, head_positions[i] + window_size });

            for (int j = start; j <= end; ++j) {
                if (j == head_positions[i]) {
                    ss << "[" << tapes[i].Get(j) << "]";
                }
                else {
                    ss << tapes[i].Get(j);
                }
            }
            ss << "\n";
        }
        return ss.str();
    }

    void PrintState() const {
        std::cout << "State: " << current_state << " | Steps: " << step_count << "\n";
        std::cout << "Head positions: ";
        for (size_t i = 0; i < active_tapes; ++i) {
            std::cout << "Tape" << (i + 1) << "=" << head_positions[i];
            if (i < active_tapes - 1) std::cout << ", ";
        }
        std::cout << " | Total materialized cells: " << GetTotalMaterializedCellsCount()
            << " | Modified cells: " << GetTotalModifiedCellsCount() << "\n";
    }

    struct TapeStatistics {
        size_t tape_index;
        size_t materialized_cells;
        size_t modified_cells; 
        int min_index;
        int max_index;
        int current_position;

        std::string ToString() const {
            std::stringstream ss;
            ss << "Tape " << (tape_index + 1) << " Statistics:\n"
                << "  Materialized cells: " << materialized_cells << "\n"
                << "  Modified cells: " << modified_cells << "\n"
                << "  Min index: " << min_index << "\n"
                << "  Max index: " << max_index << "\n"
                << "  Current position: " << current_position;
            return ss.str();
        }
    };

    Sequence<TapeStatistics> GetTapeStatistics() const {
        Sequence<TapeStatistics> stats;
        for (size_t i = 0; i < active_tapes; ++i) {
            stats.Append({
                i,
                tapes[i].GetMaterializedCount(),
                tapes[i].GetModifiedCount(),
                tapes[i].GetMinIndex(),
                tapes[i].GetMaxIndex(),
                head_positions[i]
                });
        }
        return stats;
    }

    size_t GetActiveTapeCount() const {
        return active_tapes;
    }

    char GetSymbolAtHead(size_t tape_idx) const {
        if (tape_idx >= active_tapes) {
            throw InvalidTapeException();
        }
        return tapes[tape_idx].Get(head_positions[tape_idx]);
    }

private:
    std::array<char, MAX_TAPES> GetDefaultReadArray() const {
        std::array<char, MAX_TAPES> arr;
        arr.fill(blank_symbol);
        return arr;
    }

    std::array<char, MAX_TAPES> GetDefaultWriteArray() const {
        std::array<char, MAX_TAPES> arr;
        arr.fill(blank_symbol);
        return arr;
    }
};

#endif // MULTI_TAPE_TURING_MACHINE_H
