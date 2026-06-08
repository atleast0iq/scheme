#include "interpreter/interpreter.h"
#include "runtime/error.h"
#include "runtime/heap.h"
#include "tests/fuzzer.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class SchemeTest : public ::testing::Test {
protected:
    void SetUp() override {
        Interpreter::Instance().Reset();
    }

    void ExpectEq(const std::string& expression, const std::string& result) {
        EXPECT_EQ(Interpreter::Instance().Run(expression), result);
    }

    void ExpectNoError(const std::string& expression) {
        EXPECT_NO_THROW(Interpreter::Instance().Run(expression));
    }

    void ExpectSyntaxError(const std::string& expression) {
        EXPECT_THROW(Interpreter::Instance().Run(expression), SyntaxError);
    }

    void ExpectRuntimeError(const std::string& expression) {
        EXPECT_THROW(Interpreter::Instance().Run(expression), RuntimeError);
    }

    void ExpectNameError(const std::string& expression) {
        EXPECT_THROW(Interpreter::Instance().Run(expression), NameError);
    }
};

TEST_F(SchemeTest, BooleansAreSelfEvaluating) {
    ExpectEq("#t", "#t");
    ExpectEq("#f", "#f");
}

TEST_F(SchemeTest, BooleanPredicate) {
    ExpectEq("(boolean? #t)", "#t");
    ExpectEq("(boolean? #f)", "#t");
    ExpectEq("(boolean? 1)", "#f");
    ExpectEq("(boolean? '())", "#f");
}

TEST_F(SchemeTest, NotFunction) {
    ExpectEq("(not #f)", "#t");
    ExpectEq("(not #t)", "#f");
    ExpectEq("(not 1)", "#f");
    ExpectEq("(not 0)", "#f");
    ExpectEq("(not '())", "#f");
}

TEST_F(SchemeTest, NotFunctionInvalidCall) {
    ExpectRuntimeError("(not)");
    ExpectRuntimeError("(not #t #t)");
}

TEST_F(SchemeTest, AndSyntax) {
    ExpectEq("(and)", "#t");
    ExpectEq("(and (= 2 2) (> 2 1))", "#t");
    ExpectEq("(and (= 2 2) (< 2 1))", "#f");
    ExpectEq("(and 1 2 'c '(f g))", "(f g)");
}

TEST_F(SchemeTest, AndOptimizesArgumentEvaluation) {
    ExpectNoError("(and #f (some-unknown-token-which-eval-will-crash))");
}

TEST_F(SchemeTest, OrOptimizesArgumentEvaluation) {
    ExpectNoError("(or #t (some-unknown-token-which-eval-will-crash))");
}

TEST_F(SchemeTest, OrSyntax) {
    ExpectEq("(or)", "#f");
    ExpectEq("(or (not (= 2 2)) (> 2 1))", "#t");
    ExpectEq("(or #f (< 2 1))", "#f");
    ExpectEq("(or #f 1)", "1");
}

TEST_F(SchemeTest, IfReturnValue) {
    ExpectEq("(if #t 0)", "0");
    ExpectEq("(if #f 0)", "()");
    ExpectEq("(if (= 2 2) (+ 1 10))", "11");
    ExpectEq("(if (= 2 3) (+ 1 10) 5)", "5");
}

TEST_F(SchemeTest, IfEvaluation) {
    ExpectNoError("(define x 1)");
    ExpectNoError("(if #f (set! x 2))");
    ExpectEq("x", "1");
    ExpectNoError("(if #t (set! x 4) (set! x 3))");
    ExpectEq("x", "4");
}

TEST_F(SchemeTest, IfSyntax) {
    ExpectSyntaxError("(if)");
    ExpectSyntaxError("(if 1 2 3 4)");
}

TEST_F(SchemeTest, Quote) {
    ExpectEq("(quote (1 2))", "(1 2)");
    ExpectEq("'(1 2)", "(1 2)");
}

TEST_F(SchemeTest, BeCareful) {
    ExpectRuntimeError("(())");
    ExpectRuntimeError("(+ ())");
    ExpectRuntimeError("('() ())");
    ExpectEq("'(())", "(())");
}

TEST_F(SchemeTest, IntegersAreSelfEvaluating) {
    ExpectEq("4", "4");
    ExpectEq("-14", "-14");
    ExpectEq("+14", "14");
}

TEST_F(SchemeTest, IntegerPredicate) {
    ExpectEq("(number? -1)", "#t");
    ExpectEq("(number? 1)", "#t");
    ExpectEq("(number? #t)", "#f");
}

TEST_F(SchemeTest, IntegerComparison) {
    ExpectEq("(=)", "#t");
    ExpectEq("(>)", "#t");
    ExpectEq("(<)", "#t");
    ExpectEq("(>=)", "#t");
    ExpectEq("(<=)", "#t");

    ExpectEq("(= 1 2)", "#f");
    ExpectEq("(= 1 1)", "#t");
    ExpectEq("(= 1 1 1)", "#t");
    ExpectEq("(= 1 1 2)", "#f");

    ExpectEq("(> 2 1)", "#t");
    ExpectEq("(> 1 1)", "#f");
    ExpectEq("(> 3 2 1)", "#t");
    ExpectEq("(> 3 2 3)", "#f");

    ExpectEq("(< 1 2)", "#t");
    ExpectEq("(< 1 1)", "#f");
    ExpectEq("(< 1 2 3)", "#t");
    ExpectEq("(< 1 2 1)", "#f");

    ExpectEq("(>= 2 1)", "#t");
    ExpectEq("(>= 1 2)", "#f");
    ExpectEq("(>= 3 3 2)", "#t");
    ExpectEq("(>= 3 3 4)", "#f");

    ExpectEq("(<= 2 1)", "#f");
    ExpectEq("(<= 1 2)", "#t");
    ExpectEq("(<= 3 3 4)", "#t");
    ExpectEq("(<= 3 3 2)", "#f");
}

TEST_F(SchemeTest, IntegerComparisonEdgeCases) {
    ExpectRuntimeError("(= 1 #t)");
    ExpectRuntimeError("(< 1 #t)");
    ExpectRuntimeError("(> 1 #t)");
    ExpectRuntimeError("(<= 1 #t)");
    ExpectRuntimeError("(>= 1 #t)");
}

TEST_F(SchemeTest, IntegerArithmetics) {
    ExpectEq("(+ 1 2)", "3");
    ExpectEq("(+ 1)", "1");
    ExpectEq("(+ 1 (+ 3 4 5))", "13");
    ExpectEq("(- 1 2)", "-1");
    ExpectEq("(- 2 1)", "1");
    ExpectEq("(- 2 1 1)", "0");
    ExpectEq("(* 5 6)", "30");
    ExpectEq("(* 5 6 7)", "210");
    ExpectEq("(/ 4 2)", "2");
    ExpectEq("(/ 4 2 2)", "1");
}

TEST_F(SchemeTest, IntegerArithmeticsEdgeCases) {
    ExpectRuntimeError("(+ 1 #t)");
    ExpectRuntimeError("(- 1 #t)");
    ExpectRuntimeError("(* 1 #t)");
    ExpectRuntimeError("(/ 1 #t)");

    ExpectEq("(+)", "0");
    ExpectEq("(*)", "1");
    ExpectRuntimeError("(/)");
    ExpectRuntimeError("(-)");
}

TEST_F(SchemeTest, IntegerMaxMin) {
    ExpectEq("(max 0)", "0");
    ExpectEq("(min 0)", "0");
    ExpectEq("(max 1 2)", "2");
    ExpectEq("(min 1 2)", "1");
    ExpectEq("(max 1 2 3 4 5)", "5");
    ExpectEq("(min 1 2 3 4 5)", "1");
}

TEST_F(SchemeTest, IntegerMaxMinEdgeCases) {
    ExpectRuntimeError("(max)");
    ExpectRuntimeError("(min)");
    ExpectRuntimeError("(max #t)");
    ExpectRuntimeError("(min #t)");
}

TEST_F(SchemeTest, IntegerAbs) {
    ExpectEq("(abs 10)", "10");
    ExpectEq("(abs -10)", "10");
}

TEST_F(SchemeTest, IntegerAbsEdgeCases) {
    ExpectRuntimeError("(abs)");
    ExpectRuntimeError("(abs #t)");
    ExpectRuntimeError("(abs 1 2)");
}

TEST_F(SchemeTest, SimpleLambda) {
    ExpectEq("((lambda (x) (+ 1 x)) 5)", "6");
}

TEST_F(SchemeTest, LambdaBodyHasImplicitBegin) {
    ExpectNoError("(define test (lambda (x) (set! x (* x 2)) (+ 1 x)))");
    ExpectEq("(test 20)", "41");
}

TEST_F(SchemeTest, SlowSum) {
    ExpectNoError("(define slow-add (lambda (x y) (if (= x 0) y (slow-add (- x 1) (+ y 1)))))");
    ExpectEq("(slow-add 3 3)", "6");
    ExpectEq("(slow-add 100 100)", "200");
}

TEST_F(SchemeTest, LambdaClosure) {
    ExpectNoError("(define x 1)");
    ExpectNoError("(define range (lambda (x) (lambda () (set! x (+ x 1)) x)))");
    ExpectNoError("(define my-range (range 10))");
    ExpectEq("(my-range)", "11");
    ExpectEq("(my-range)", "12");
    ExpectEq("(my-range)", "13");
    ExpectEq("x", "1");
}

TEST_F(SchemeTest, LambdaSyntax) {
    ExpectSyntaxError("(lambda)");
    ExpectSyntaxError("(lambda x)");
    ExpectSyntaxError("(lambda (x))");
}

TEST_F(SchemeTest, DefineLambdaSugar) {
    ExpectNoError("(define (inc x) (+ x 1))");
    ExpectEq("(inc -1)", "0");
    ExpectNoError("(define (add x y) (+ x y 1))");
    ExpectEq("(add -10 10)", "1");
    ExpectNoError("(define (zero) 0)");
    ExpectEq("(zero)", "0");
}

TEST_F(SchemeTest, LambdaMultipleRecurseCalls) {
    ExpectNoError("(define (fib x) (if (< x 3) 1 (+ (fib (- x 1)) (fib (- x 2)))))");
    ExpectEq("(fib 1)", "1");
    ExpectEq("(fib 2)", "1");
    ExpectEq("(fib 3)", "2");
    ExpectEq("(fib 7)", "13");
    ExpectEq("(fib 8)", "21");
}

TEST_F(SchemeTest, MutualCalls) {
    ExpectNoError("(define (foo x) (if (< x 2) 42 (bar (- x 1))))");
    ExpectNoError("(define (bar x) (if (< x 2) 24 (foo (/ x 2))))");
    ExpectEq("(foo 3)", "42");
    ExpectEq("(foo 6)", "24");
    ExpectEq("(bar 7)", "42");
    ExpectEq("(bar 13)", "24");
}

TEST_F(SchemeTest, LambdasShareContext) {
    ExpectNoError("(define (foo x) (cons (lambda () (set! x (+ x 1)) x) (lambda () (set! x (* x 2)) x)))");
    ExpectNoError("(define my-foo (foo 15))");
    ExpectEq("((cdr my-foo))", "30");
    ExpectEq("((car my-foo))", "31");
    ExpectEq("((car my-foo))", "32");
    ExpectEq("((cdr my-foo))", "64");
}

TEST_F(SchemeTest, CyclicLocalContextDependencies) {
    ExpectNoError("(define (foo x) (define (bar) (set! x (+ (* x 2) 2)) x) bar)");
    ExpectNoError("(define my-foo (foo 20))");
    ExpectNoError("(define foo 1543)");
    ExpectEq("(my-foo)", "42");
}

TEST_F(SchemeTest, DeepRecursion) {
    constexpr uint32_t kFnsCount = 100;
    std::vector<std::string> fns;
    fns.reserve(kFnsCount);
    for (uint32_t i = 0; i < kFnsCount; ++i) {
        std::string fn = "ahaha" + std::to_string(i);
        ExpectNoError("(define (" + fn + " x) (if (= x 0) 0 (+ 1 (" + fn + " (- x 1)))))");
        fns.push_back(std::move(fn));
    }
    for (const auto& fn : fns) {
        ExpectEq("(" + fn + " 100)", "100");
    }
}

TEST_F(SchemeTest, Redefinition) {
    ExpectEq("(+ 1 2 -3)", "0");
    ExpectNoError("(define plus +)");
    ExpectEq("(plus 1 2 -3)", "0");
    ExpectNoError("(define (+ x) (if (= x 0) 0 (plus 1 (+ (- x 1)))))");
    ExpectEq("(+ 8)", "8");
    ExpectRuntimeError("(+)");
    ExpectRuntimeError("(+ 1 2 3)");
}

TEST_F(SchemeTest, RedefinitionAndScoping) {
    ExpectNoError("(define / -)");
    ExpectEq("(+ 1 2 -3)", "0");
    ExpectNoError("(define (foo) (define (+ x y) (* x y)) (lambda (x y) (+ x y)))");
    ExpectNoError("(define (bar) (define (+ x y) (/ x y)) (lambda (x y) (+ x y)))");
    ExpectNoError("(define (foobar) (lambda (x y) (+ x y)))");
    ExpectEq("((foo) 1 2)", "2");
    ExpectEq("((bar) 1 2)", "-1");
    ExpectEq("((foobar) 1 2)", "3");
    ExpectEq("(+ 1 2 -3)", "0");
}

TEST_F(SchemeTest, ListsAreNotSelfEvaluating) {
    ExpectRuntimeError("(1)");
    ExpectRuntimeError("(1 2 3)");
    ExpectEq("'()", "()");
    ExpectEq("'(1)", "(1)");
    ExpectEq("'(1 2)", "(1 2)");
}

TEST_F(SchemeTest, ListSyntax) {
    ExpectEq("'(1 . 2)", "(1 . 2)");
    ExpectSyntaxError("(1 . 2 3)");
    ExpectEq("'(1 2 . 3)", "(1 2 . 3)");
    ExpectEq("'(1 2 . ())", "(1 2)");
    ExpectEq("'(1 . (2 . ()))", "(1 2)");
}

TEST_F(SchemeTest, ListInvalidSyntax) {
    ExpectSyntaxError("((1)");
    ExpectSyntaxError(")(1)");
    ExpectSyntaxError("(.)");
    ExpectSyntaxError("(1 .)");
    ExpectSyntaxError("(. 2)");
}

TEST_F(SchemeTest, PairPredicate) {
    ExpectEq("(pair? '(1 . 2))", "#t");
    ExpectEq("(pair? '(1 2))", "#t");
    ExpectEq("(pair? '())", "#f");
}

TEST_F(SchemeTest, NullPredicate) {
    ExpectEq("(null? '())", "#t");
    ExpectEq("(null? '(1 2))", "#f");
    ExpectEq("(null? '(1 . 2))", "#f");
}

TEST_F(SchemeTest, ListPredicate) {
    ExpectEq("(list? '())", "#t");
    ExpectEq("(list? '(1 2))", "#t");
    ExpectEq("(list? '(1 . 2))", "#f");
    ExpectEq("(list? '(1 2 3 4 . 5))", "#f");
}

TEST_F(SchemeTest, PairOperations) {
    ExpectEq("(cons 1 2)", "(1 . 2)");
    ExpectEq("(car '(1 . 2))", "1");
    ExpectEq("(car '(1))", "1");
    ExpectEq("(car '(1 2 3))", "1");
    ExpectEq("(cdr '(1 . 2))", "2");
    ExpectEq("(cdr '(1))", "()");
    ExpectEq("(cdr '(1 2))", "(2)");
    ExpectEq("(cdr '(1 2 3))", "(2 3)");
    ExpectEq("(cdr '(1 2 3 . 4))", "(2 3 . 4)");
    ExpectRuntimeError("(car '())");
    ExpectRuntimeError("(cdr '())");
}

TEST_F(SchemeTest, ListOperations) {
    ExpectEq("(list)", "()");
    ExpectEq("(list 1)", "(1)");
    ExpectEq("(list 1 2 3)", "(1 2 3)");
    ExpectEq("(list-ref '(1 2 3) 1)", "2");
    ExpectEq("(list-tail '(1 2 3) 1)", "(2 3)");
    ExpectEq("(list-tail '(1 2 3) 3)", "()");
    ExpectRuntimeError("(list-ref '(1 2 3) 3)");
    ExpectRuntimeError("(list-ref '(1 2 3) 10)");
    ExpectRuntimeError("(list-tail '(1 2 3) 10)");
}

TEST_F(SchemeTest, PairMutations) {
    ExpectNoError("(define x '(1 . 2))");
    ExpectNoError("(set-car! x 5)");
    ExpectEq("(car x)", "5");
    ExpectNoError("(set-cdr! x 6)");
    ExpectEq("(cdr x)", "6");
    ExpectNoError("(define z 1543)");
    ExpectNoError("(set-cdr! x z)");
    ExpectEq("(cdr x)", "1543");
    ExpectNoError("(set-car! x z)");
    ExpectEq("(car x)", "1543");
}

TEST_F(SchemeTest, SelfReferenceCar) {
    ExpectNoError("(define x '(1 . 2))");
    ExpectNoError("(set-car! x x)");
    ExpectEq("(cdr (car (car (car x))))", "2");
    ExpectNoError("(set-car! (car x) (car x))");
    ExpectEq("(cdr x)", "2");
    ExpectNoError("(set-cdr! (car (car x)) 1543)");
    ExpectEq("(cdr x)", "1543");
    ExpectNoError("(set-car! (car (car x)) 3)");
    ExpectEq("(car x)", "3");
}

TEST_F(SchemeTest, SelfReferenceCdr) {
    ExpectNoError("(define y '(1 . 2))");
    ExpectNoError("(set-cdr! y y)");
    ExpectEq("(car (cdr (cdr (cdr y))))", "1");
    ExpectNoError("(set-cdr! (cdr y) (cdr y))");
    ExpectEq("(car y)", "1");
    ExpectNoError("(set-car! (cdr (cdr y)) 1543)");
    ExpectEq("(car y)", "1543");
    ExpectNoError("(set-cdr! (cdr (cdr y)) 3)");
    ExpectEq("(cdr y)", "3");
}

TEST_F(SchemeTest, SymbolsAreNotSelfEvaluating) {
    ExpectNameError("x");
    ExpectEq("'x", "x");
    ExpectEq("(quote x)", "x");
}

TEST_F(SchemeTest, SymbolPredicate) {
    ExpectEq("(symbol? 'x)", "#t");
    ExpectEq("(symbol? 1)", "#f");
}

TEST_F(SchemeTest, SymbolsAreUsedAsVariableNames) {
    ExpectNoError("(define x (+ 1 2))");
    ExpectEq("x", "3");
    ExpectNoError("(define x (+ 2 2))");
    ExpectEq("x", "4");
}

TEST_F(SchemeTest, DefineInvalidSyntax) {
    ExpectSyntaxError("(define)");
    ExpectSyntaxError("(define 1)");
    ExpectSyntaxError("(define x 1 2)");
}

TEST_F(SchemeTest, SetOverrideVariables) {
    ExpectNameError("(set! x 2)");
    ExpectNameError("x");
    ExpectNoError("(define x 1)");
    ExpectEq("x", "1");
    ExpectNoError("(set! x (+ 2 4))");
    ExpectEq("x", "6");
}

TEST_F(SchemeTest, SetInvalidSyntax) {
    ExpectSyntaxError("(set!)");
    ExpectSyntaxError("(set! 1)");
    ExpectSyntaxError("(set! x 1 2)");
}

TEST_F(SchemeTest, CopySemantics) {
    ExpectNoError("(define x 1)");
    ExpectNoError("(define y x)");
    ExpectEq("x", "1");
    ExpectEq("y", "1");
    ExpectNoError("(define x y)");
    ExpectNoError("(set! y (+ 1 y))");
    ExpectEq("y", "2");
    ExpectEq("x", "1");
}

TEST_F(SchemeTest, EvaluationOrder) {
    ExpectNameError("(define x x)");
}

TEST_F(SchemeTest, SerializeCallable) {
    ExpectEq("+", "<built-in function +>");
    ExpectEq("if", "<special form if>");
    ExpectEq("(lambda (x) x)", "<lambda>");
    ExpectNoError("(define (f x) x)");
    ExpectEq("f", "<lambda>");
}

TEST_F(SchemeTest, ImproperArgumentList) {
    ExpectSyntaxError("(+ . 1)");
}

TEST_F(SchemeTest, EmptyListCall) {
    ExpectRuntimeError("(())");
}

TEST_F(SchemeTest, TrailingTokens) {
    EXPECT_THROW(Interpreter::Instance().Run("1 2"), SyntaxError);
}

TEST_F(SchemeTest, IfImproperArgList) {
    ExpectSyntaxError("(if . #t)");
}

TEST_F(SchemeTest, AndImproperArgList) {
    ExpectSyntaxError("(and . #t)");
}

TEST_F(SchemeTest, SetCarRequiresPair) {
    ExpectRuntimeError("(set-car! 1 2)");
}

TEST_F(SchemeTest, SetCdrRequiresPair) {
    ExpectRuntimeError("(set-cdr! 1 2)");
}

TEST_F(SchemeTest, DefineNonSymbolTarget) {
    ExpectSyntaxError("(define 1 2)");
}

TEST_F(SchemeTest, DefineFunctionNonSymbolName) {
    ExpectSyntaxError("(define (1 x) x)");
}

TEST_F(SchemeTest, SetNonSymbol) {
    ExpectSyntaxError("(set! 1 2)");
}

TEST_F(SchemeTest, LambdaVariadicParams) {
    ExpectEq("((lambda (x . rest) rest) 1 2 3)", "(2 3)");
    ExpectEq("((lambda args args) 1 2 3)", "(1 2 3)");
    ExpectEq("((lambda (x . rest) x) 1 2 3)", "1");
    ExpectSyntaxError("(lambda (1 . rest) rest)");
}

TEST_F(SchemeTest, LambdaParamsMustBeSymbols) {
    ExpectSyntaxError("(lambda (1) 1)");
}

TEST_F(SchemeTest, LambdaImproperBody) {
    ExpectNoError("(define (f x) x)");
    ExpectEq("(f 1)", "1");
}

TEST_F(SchemeTest, GcCollectsTemporaryObjects) {
    ExpectNoError("(define (loop n) (if (= n 0) 42 (loop (- n 1))))");
    constexpr uint32_t kIterations = 1000;
    for (uint32_t i = 0; i < kIterations; ++i) {
        ExpectEq("(loop 1000)", "42");
    }
}

TEST_F(SchemeTest, FuzzingDoesNotCrash) {
    static constexpr uint32_t kShotsCount = 10000;
    Fuzzer fuzzer;

    for (uint32_t i = 0; i < kShotsCount; ++i) {
        try {
            Interpreter::Instance().Run(fuzzer.Next());
        } catch (const SyntaxError&) {
        } catch (const RuntimeError&) {
        } catch (const NameError&) {
        }
    }
}
