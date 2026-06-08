#pragma once

#include "runtime/heap.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class Environment;

class Object : public GcNode {
public:
    virtual ~Object() = default;

    virtual Object* Eval(Environment& env) = 0;
    virtual std::string Serialize() const = 0;
};

template <class T> T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T> bool Is(Object* obj) {
    return As<T>(obj) != nullptr;
}

using CallableFactory = Object* (*)(Heap&);

struct BuiltinDefinition {
    const char* name;
    CallableFactory factory;
};

class SelfEvaluatingObject : public Object {
public:
    Object* Eval(Environment&) override {
        return this;
    }

    void Traverse(const std::function<void(GcNode*)>&) override {
    }
};

class Callable : public SelfEvaluatingObject {};

using RawArgsHandler = std::function<Object*(Object*, Environment&)>;
using EvaluatedArgsHandler = std::function<Object*(const std::vector<Object*>&, Environment&)>;

class SpecialForm : public Callable {
public:
    SpecialForm(std::string name, RawArgsHandler handler)
        : name_(std::move(name)), handler_(std::move(handler)) {
    }

    Object* Apply(Object* raw_args, Environment& env) {
        return handler_(raw_args, env);
    }

    std::string Serialize() const override {
        return "<special form " + name_ + ">";
    }

private:
    std::string name_;
    RawArgsHandler handler_;
};

class Function : public Callable {
public:
    virtual Object* Apply(const std::vector<Object*>& args, Environment& env) = 0;
};

class BuiltinFunction : public Function {
public:
    BuiltinFunction(std::string name, EvaluatedArgsHandler handler)
        : name_(std::move(name)), handler_(std::move(handler)) {
    }

    Object* Apply(const std::vector<Object*>& args, Environment& env) override {
        return handler_(args, env);
    }

    std::string Serialize() const override {
        return "<built-in function " + name_ + ">";
    }

private:
    std::string name_;
    EvaluatedArgsHandler handler_;
};

class Boolean : public SelfEvaluatingObject {
public:
    explicit Boolean(bool value) : value_(value) {
    }

    bool GetValue() const {
        return value_;
    }

    std::string Serialize() const override {
        return value_ ? "#t" : "#f";
    }

private:
    bool value_;
};

class Number : public SelfEvaluatingObject {
public:
    explicit Number(int64_t value) : value_(value) {
    }

    int64_t GetValue() const {
        return value_;
    }

    std::string Serialize() const override {
        return std::to_string(value_);
    }

private:
    int64_t value_;
};

class Symbol : public Object {
public:
    explicit Symbol(std::string name) : name_(std::move(name)) {
    }

    const std::string& GetName() const {
        return name_;
    }

    Object* Eval(Environment& env) override;

    std::string Serialize() const override {
        return name_;
    }

    void Traverse(const std::function<void(GcNode*)>&) override {
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(Object* first = nullptr, Object* second = nullptr) : first_(first), second_(second) {
    }

    Object* GetFirst() const {
        return first_;
    }

    Object* GetSecond() const {
        return second_;
    }

    void SetFirst(Object* first) {
        first_ = first;
    }

    void SetSecond(Object* second) {
        second_ = second;
    }

    Object* Eval(Environment& env) override;
    std::string Serialize() const override;
    void Traverse(const std::function<void(GcNode*)>& visit) override;

private:
    Object* first_;
    Object* second_;
};

class LambdaFunction : public Function {
public:
    LambdaFunction(std::vector<std::string> params, Object* body, Environment* closure)
        : params_(std::move(params)), body_(body), closure_(closure) {
    }

    Object* Apply(const std::vector<Object*>& args, Environment& env) override;

    std::string Serialize() const override {
        return "<lambda>";
    }

    void Traverse(const std::function<void(GcNode*)>& visit) override;

private:
    std::vector<std::string> params_;
    Object* body_;
    Environment* closure_;
};

class TailCall : public Object {
public:
    TailCall(Object* expr, Environment* env) : expr_(expr), env_(env) {
    }

    Object* GetExpr() const {
        return expr_;
    }

    Environment* GetEnv() const {
        return env_;
    }

    Object* Eval(Environment&) override {
        return this;
    }

    std::string Serialize() const override {
        return "<tail-call>";
    }

    void Traverse(const std::function<void(GcNode*)>& visit) override;

private:
    Object* expr_;
    Environment* env_;
};

Object* MakeBuiltinForm(Heap& heap, std::string name, RawArgsHandler handler);
Object* MakeBuiltinFunction(Heap& heap, std::string name, EvaluatedArgsHandler handler);
Object* MakeBoolean(Heap& heap, bool value);
Object* MakeNumber(Heap& heap, int64_t value);
Object* MakeSymbol(Heap& heap, std::string name);
Object* MakeCell(Heap& heap, Object* first, Object* second);

bool IsFalse(Object* object);
void AssertExpressionExists(std::string_view context, Object* object);
