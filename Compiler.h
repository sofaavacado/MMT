#pragma once

#include <memory>
#include <map>
#include <regex>
#include <fstream>
#include "multi_tape_turing_machine.h"
#include "Sequence.h"

class TuringMachineCompiler {
public:
    struct ParsedTransition {
        std::string fromState;
        std::string toState;
        std::array<char, 3> readSymbols;
        std::array<char, 3> writeSymbols;
        std::array<int, 3> moves;

        ParsedTransition() : readSymbols{}, writeSymbols{}, moves{} {
            readSymbols.fill(' ');
            writeSymbols.fill(' ');
            moves.fill(0);
        }
    };

    static Sequence<ParsedTransition> Compile(const std::string& code, int tapeCount, std::string& error) {
        Sequence<ParsedTransition> transitions;
        error.clear();

        std::istringstream iss(code);
        std::string line;
        int lineNum = 0;

        std::regex transRegex(R"((\w+)\s*,\s*([\w\s\+])\s*,\s*([\w\s\+])\s*,\s*([\w\s\+])\s*->\s*(\w+)\s*,\s*([\w\s\+])\s*,\s*([\w\s\+])\s*,\s*([\w\s\+])\s*,\s*([RLS])\s*,\s*([RLS])\s*,\s*([RLS]))");

        while (std::getline(iss, line)) {
            lineNum++;

            if (line.empty() || line[0] == '#') continue;

            std::smatch match;
            if (!std::regex_search(line, match, transRegex)) {
                error += "Line " + std::to_string(lineNum) + ": Invalid format\n";
                continue;
            }

            ParsedTransition trans;
            trans.fromState = match[1].str();
            trans.toState = match[5].str();

            trans.readSymbols[0] = (match[2].str() == " ") ? ' ' : match[2].str()[0];
            trans.readSymbols[1] = (match[3].str() == " ") ? ' ' : match[3].str()[0];
            trans.readSymbols[2] = (match[4].str() == " ") ? ' ' : match[4].str()[0];

            trans.writeSymbols[0] = (match[6].str() == " ") ? ' ' : match[6].str()[0];
            trans.writeSymbols[1] = (match[7].str() == " ") ? ' ' : match[7].str()[0];
            trans.writeSymbols[2] = (match[8].str() == " ") ? ' ' : match[8].str()[0];

            trans.moves[0] = (match[9].str() == "R") ? 1 : (match[9].str() == "L") ? -1 : 0;
            trans.moves[1] = (match[10].str() == "R") ? 1 : (match[10].str() == "L") ? -1 : 0;
            trans.moves[2] = (match[11].str() == "R") ? 1 : (match[11].str() == "L") ? -1 : 0;

            transitions.Append(trans);
        }

        return transitions;
    }
};