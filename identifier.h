#pragma once

#include <memory>
#include <map>
#include <regex>
#include <fstream>

// Идендификаторы событий
enum {
    ID_RUN = 1001,
    ID_STEP,
    ID_RESET,
    ID_STOP,
    ID_TAPE_COUNT, // Количество лент
    ID_TIMER, // Таймер автоматического выполнения
    ID_SPEED_SLIDER, // Скорость выполнения 
    ID_ADD_TRANSITION,
    ID_REMOVE_TRANSITION,
    ID_TRANSITIONS_GRID, // Таблица переходов
    ID_COMPILE,
    ID_LOAD_FILE,
    ID_SAVE_FILE,
    ID_PROGRAM_TEXT, // Текстовое поле с программой
    ID_TEMPLATE_ALPHABET,
    ID_TEMPLATE_ALPHANUMERIC,
    ID_TEMPLATE_BINARY_INVERTER,
    ID_TEMPLATE_BINARY_INVERTER_2TAPES,
    ID_TEMPLATE_UNARY_ADDITION,
    ID_TEMPLATE_COUNTER,
    ID_TEMPLATE_SIMPLE,
};