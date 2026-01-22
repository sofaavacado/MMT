#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class IndexOutOfRangeException : public std::exception {
private:
    std::string message;
public:
    IndexOutOfRangeException(const std::string& msg = "Index out of range")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class InvalidStateException : public std::exception {
private:
    std::string message;
public:
    InvalidStateException(const std::string& msg = "Invalid state")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class InvalidTapeException : public std::exception {
private:
    std::string message;
public:
    InvalidTapeException(const std::string& msg = "Invalid tape index")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class EmptyContainerException : public std::exception {
private:
    std::string message;
public:
    EmptyContainerException(const std::string& msg = "Container is empty")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class InvalidArgumentException : public std::exception {
private:
    std::string message;
public:
    InvalidArgumentException(const std::string& msg = "Invalid argument")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class MachineHaltedException : public std::exception {
private:
    std::string message;
public:
    MachineHaltedException(const std::string& msg = "Turing machine halted")
        : message(msg) {
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

#endif // EXCEPTIONS_H