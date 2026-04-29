#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <variant>

struct ConstantToken {
    int64_t value;

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    }
};

enum class BracketToken { OPEN, CLOSE };

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    }
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    }
};

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    }
};

using Token = std::variant<ConstantToken, BracketToken, QuoteToken, DotToken, SymbolToken>;

class Tokenizer {
public:
    explicit Tokenizer(std::istream* in);

    bool IsEnd() const;

    void Next();

    Token GetToken() const;

private:
    std::istream* in_;
    Token token_ = ConstantToken{0};
    bool is_eof_ = false;

    void SkipWhitespace() const;
    bool TryReadConstant();
    bool TryReadSymbol();
};
