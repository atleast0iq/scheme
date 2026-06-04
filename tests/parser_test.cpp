#include "parser/parser.h"
#include "runtime/error.h"
#include "runtime/object.h"
#include "tokenizer/tokenizer.h"

#include <gtest/gtest.h>
#include <random>
#include <sstream>

namespace {

Object* ReadFull(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    auto* obj = Read(&tokenizer);
    EXPECT_TRUE(tokenizer.IsEnd());
    return obj;
}

std::string RandomSymbol(std::default_random_engine* rng) {
    std::uniform_int_distribution<int> symbol('a', 'z');
    std::string s;
    for (int i = 0; i < 5; ++i) {
        s.push_back(static_cast<char>(symbol(*rng)));
    }
    return s;
}

} // namespace

TEST(Parser, ReadNumber) {
    auto* node = ReadFull("5");
    ASSERT_TRUE(Is<Number>(node));
    EXPECT_EQ(As<Number>(node)->GetValue(), 5);

    node = ReadFull("-5");
    ASSERT_TRUE(Is<Number>(node));
    EXPECT_EQ(As<Number>(node)->GetValue(), -5);
}

TEST(Parser, ReadSymbolPlus) {
    auto* node = ReadFull("+");
    ASSERT_TRUE(Is<Symbol>(node));
    EXPECT_EQ(As<Symbol>(node)->GetName(), "+");
}

TEST(Parser, ReadSymbolRandom) {
    std::default_random_engine rng{42};
    for (int i = 0; i < 10; ++i) {
        auto name = RandomSymbol(&rng);
        auto* node = ReadFull(name);
        ASSERT_TRUE(Is<Symbol>(node));
        EXPECT_EQ(As<Symbol>(node)->GetName(), name);
    }
}

TEST(Parser, EmptyList) {
    auto* null = ReadFull("()");
    EXPECT_EQ(null, nullptr);
}

TEST(Parser, Pair) {
    auto* pair = ReadFull("(1 . 2)");
    ASSERT_TRUE(Is<Cell>(pair));

    auto* first = As<Cell>(pair)->GetFirst();
    ASSERT_TRUE(Is<Number>(first));
    EXPECT_EQ(As<Number>(first)->GetValue(), 1);

    auto* second = As<Cell>(pair)->GetSecond();
    ASSERT_TRUE(Is<Number>(second));
    EXPECT_EQ(As<Number>(second)->GetValue(), 2);
}

TEST(Parser, SimpleList) {
    auto* list = ReadFull("(1 2)");
    ASSERT_TRUE(Is<Cell>(list));

    auto* first = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(first));
    EXPECT_EQ(As<Number>(first)->GetValue(), 1);

    list = As<Cell>(list)->GetSecond();
    auto* second = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(second));
    EXPECT_EQ(As<Number>(second)->GetValue(), 2);

    EXPECT_EQ(As<Cell>(list)->GetSecond(), nullptr);
}

TEST(Parser, ListWithOperator) {
    auto* list = ReadFull("(+ 1 2)");
    ASSERT_TRUE(Is<Cell>(list));

    auto* first = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Symbol>(first));
    EXPECT_EQ(As<Symbol>(first)->GetName(), "+");

    list = As<Cell>(list)->GetSecond();
    auto* second = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(second));
    EXPECT_EQ(As<Number>(second)->GetValue(), 1);

    list = As<Cell>(list)->GetSecond();
    second = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(second));
    EXPECT_EQ(As<Number>(second)->GetValue(), 2);

    EXPECT_EQ(As<Cell>(list)->GetSecond(), nullptr);
}

TEST(Parser, ListWithFunnyEnd) {
    auto* list = ReadFull("(1 2 . 3)");
    ASSERT_TRUE(Is<Cell>(list));

    auto* first = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(first));
    EXPECT_EQ(As<Number>(first)->GetValue(), 1);

    list = As<Cell>(list)->GetSecond();
    auto* second = As<Cell>(list)->GetFirst();
    ASSERT_TRUE(Is<Number>(second));
    EXPECT_EQ(As<Number>(second)->GetValue(), 2);

    auto* last = As<Cell>(list)->GetSecond();
    ASSERT_TRUE(Is<Number>(last));
    EXPECT_EQ(As<Number>(last)->GetValue(), 3);
}

TEST(Parser, ComplexLists) {
    EXPECT_NO_THROW(ReadFull("(1 . ())"));
    EXPECT_NO_THROW(ReadFull("(1 2 . ())"));
    EXPECT_NO_THROW(ReadFull("(1 . (2 . ()))"));
    EXPECT_NO_THROW(ReadFull("(1 2 (3 4) (()))"));
    EXPECT_NO_THROW(ReadFull("(+ 1 2 (- 3 4))"));
}

TEST(Parser, TrickyCell) {
    auto* cell = ReadFull("(())");
    ASSERT_TRUE(Is<Cell>(cell));
    EXPECT_EQ(As<Cell>(cell)->GetFirst(), nullptr);
    EXPECT_EQ(As<Cell>(cell)->GetSecond(), nullptr);
}

TEST(Parser, InvalidInput) {
    EXPECT_THROW(ReadFull(""), SyntaxError);
    EXPECT_THROW(ReadFull("'"), SyntaxError);
    EXPECT_THROW(ReadFull("("), SyntaxError);
    EXPECT_THROW(ReadFull("(1"), SyntaxError);
    EXPECT_THROW(ReadFull("(1 ."), SyntaxError);
    EXPECT_THROW(ReadFull("( ."), SyntaxError);
    EXPECT_THROW(ReadFull("(1 . ()"), SyntaxError);
    EXPECT_THROW(ReadFull("(1 . )"), SyntaxError);
    EXPECT_THROW(ReadFull("(1 . 2 3)"), SyntaxError);
}
