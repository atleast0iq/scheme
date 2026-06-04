#pragma once

#include <string>

class Environment;

class Interpreter {
public:
    static Interpreter& Instance();

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;
    Interpreter(Interpreter&&) = delete;
    Interpreter& operator=(Interpreter&&) = delete;

    std::string Run(const std::string& input);
    void Reset();

private:
    Interpreter();
    ~Interpreter();

    Environment* env_;
};
