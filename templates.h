#pragma once

#include <memory>
#include <map>
#include <regex>
#include <fstream>
#include "multi_tape_turing_machine.h"
#include "Sequence.h"

class TuringTemplates {
public:
    // Копирование английского алфавита (a-z)
    static std::string CopyEnglishAlphabet() {
        std::string code = "# Copy English Alphabet (a-z)\n";
        code += "# Tape 1: input, Tape 2: output\n\n";

        for (char c = 'a'; c <= 'z'; ++c) {
            code += "q0,";
            code += c;
            code += ", , ->q0,";
            code += c;
            code += ",";
            code += c;
            code += ", ,R,R,S\n";
        }

        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }

    // Копирование символов (A-Z, a-z, 0-9)
    static std::string CopyAlphanumeric() {
        std::string code = "# Copy Alphanumeric (A-Z, a-z, 0-9)\n";
        code += "# Tape 1: input, Tape 2: output\n\n";

        for (char c = 'A'; c <= 'Z'; ++c) {
            code += "q0,";
            code += c;
            code += ", , ->q0,";
            code += c;
            code += ",";
            code += c;
            code += ", ,R,R,S\n";
        }

        for (char c = 'a'; c <= 'z'; ++c) {
            code += "q0,";
            code += c;
            code += ", , ->q0,";
            code += c;
            code += ",";
            code += c;
            code += ", ,R,R,S\n";
        }

        for (char c = '0'; c <= '9'; ++c) {
            code += "q0,";
            code += c;
            code += ", , ->q0,";
            code += c;
            code += ",";
            code += c;
            code += ", ,R,R,S\n";
        }

        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }

    // Инвертор битов (0, 1)
    static std::string BinaryInverter() {
        std::string code = "# Binary Inverter (0->1, 1->0)\n";
        code += "# Tape 1: input/output\n\n";
        code += "q0,0, , ->q0,1, , ,R,S,S\n";
        code += "q0,1, , ->q0,0, , ,R,S,S\n";
        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }

    // Инвертор битов (0, 1) с использованием 2 лент
    static std::string BinaryInverter2Tapes() {
        std::string code = "# Binary Inverter (0->1, 1->0) - Two Tapes\n";
        code += "# Tape 1: input, Tape 2: inverted output\n\n";

        code += "q0,0, , ->q0,0,1, ,R,R,S\n";

        code += "q0,1, , ->q0,1,0, ,R,R,S\n";

        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }

    // Унарное сложение
    static std::string UnaryAddition() {
        std::string code = "# Unary Addition (e.g., 111+11 = 11111)\n";
        code += "# Tape 1: input (e.g., 111+11), Tape 2: output\n";
        code += "# Format: first number + second number separated by '+'\n\n";

        code += "# State q0: copy first number (1's)\n";
        code += "q0,1, , ->q0,1,1, ,R,R,S\n";

        code += "# Encounter +, move to state q1\n";
        code += "q0,+, , ->q1,+, , ,R,S,S\n\n";

        code += "# State q1: copy second number\n";
        code += "q1,1, , ->q1,1,1, ,R,R,S\n";

        code += "# Encounter space, finish\n";
        code += "q1, , , ->q2, , , ,S,S,S\n";

        return code;
    }

    static std::string BinaryCounter() {
        std::string code = "# Binary Counter: read bits from Tape 1, increment on Tape 2\n";
        code += "# Tape 1: input binary, Tape 2: counter (starts empty)\n\n";
        code += "q0,0, , ->q0,0,0, ,R,R,S\n";
        code += "q0,1, , ->q0,1,1, ,R,R,S\n";
        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }

    static std::string SimpleCopy() {
        std::string code = "# Simple Copy: Copy one symbol from Tape 1 to Tape 2\n";
        code += "q0,a, , ->q0,a,a, ,R,R,S\n";
        code += "q0, , , ->q1, , , ,S,S,S\n";
        return code;
    }
};