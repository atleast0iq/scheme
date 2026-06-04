#include "tokenizer/tokenizer.h"
#include "runtime/error.h"

#include <gtest/gtest.h>
#include <sstream>

TEST(Tokenizer, SimpleCase) {
    std::stringstream ss{"4+)'."};
    Tokenizer tokenizer{&ss};

    EXPECT_FALSE(tokenizer.IsEnd());
    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

    tokenizer.Next();
    EXPECT_FALSE(tokenizer.IsEnd());
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

    tokenizer.Next();
    EXPECT_FALSE(tokenizer.IsEnd());
    EXPECT_EQ(tokenizer.GetToken(), Token{BracketToken::CLOSE});

    tokenizer.Next();
    EXPECT_FALSE(tokenizer.IsEnd());
    EXPECT_EQ(tokenizer.GetToken(), Token{QuoteToken{}});

    tokenizer.Next();
    EXPECT_FALSE(tokenizer.IsEnd());
    EXPECT_EQ(tokenizer.GetToken(), Token{DotToken{}});

    tokenizer.Next();
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, NegativeNumbers) {
    std::stringstream ss{"-2 - 2"};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{-2}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"-"}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});
}

TEST(Tokenizer, SymbolNames) {
    std::stringstream ss{"foo bar zog-zog? Am1good?"};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"foo"}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"bar"}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"zog-zog?"}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"Am1good?"}});
}

TEST(Tokenizer, GetTokenIsNotMoving) {
    std::stringstream ss{"1234+4"};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{1234}});
    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{1234}});
}

TEST(Tokenizer, IsStreaming) {
    std::stringstream ss;
    ss << "2 ";

    Tokenizer tokenizer{&ss};
    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});

    ss << "* ";
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"*"}});

    ss << "2";
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});
}

TEST(Tokenizer, JustSpaces) {
    std::stringstream ss{"      "};
    Tokenizer tokenizer{&ss};

    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, SpacesBetweenTokens) {
    std::stringstream ss{"  4 +  "};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

    tokenizer.Next();
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, NewlinesHandled) {
    std::string input = R"EOF(
                               )EOF";
    std::stringstream ss{input};
    Tokenizer tokenizer{&ss};

    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, NewlinesWithTokens) {
    std::string input = R"EOF(
                        4 +
                        )EOF";
    std::stringstream ss{input};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

    tokenizer.Next();
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, EmptyString) {
    std::stringstream ss;
    Tokenizer tokenizer{&ss};

    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, OpenBracket) {
    std::stringstream ss{"("};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{BracketToken::OPEN});
    tokenizer.Next();
    EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(Tokenizer, InvalidSymbol) {
    std::stringstream ss{"@"};
    EXPECT_THROW(Tokenizer{&ss}, SyntaxError);
}

TEST(Tokenizer, PositiveNumberWithSign) {
    std::stringstream ss{"+42"};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{42}});
}

TEST(Tokenizer, SpecialSymbolNames) {
    std::stringstream ss{"< > = / set!"};
    Tokenizer tokenizer{&ss};

    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"<"}});
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{">"}});
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"="}});
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"/"}});
    tokenizer.Next();
    EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"set!"}});
}
