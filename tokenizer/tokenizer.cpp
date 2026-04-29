#include "tokenizer/tokenizer.h"

#include "runtime/error.h"

#include <cctype>
#include <istream>

Tokenizer::Tokenizer(std::istream* in) : in_(in) {
    Next();
}

bool Tokenizer::IsEnd() const {
    return is_eof_;
}

void Tokenizer::Next() {
    SkipWhitespace();

    if (in_->eof()) {
        is_eof_ = true;
        return;
    }

    const char ch = in_->peek();
    if (ch == '(') {
        token_ = BracketToken::OPEN;
    } else if (ch == ')') {
        token_ = BracketToken::CLOSE;
    } else if (ch == '\'') {
        token_ = QuoteToken();
    } else if (ch == '.') {
        token_ = DotToken();
    } else if ((ch == '+' || ch == '-' || std::isdigit(ch)) && TryReadConstant()) {
        return;
    } else if (TryReadSymbol()) {
        return;
    } else {
        throw SyntaxError("Unexpected symbol");
    }
    in_->get();
}

Token Tokenizer::GetToken() const {
    return token_;
}

void Tokenizer::SkipWhitespace() const {
    while (in_->good() && std::isspace(in_->peek())) {
        in_->get();
    }
}

bool Tokenizer::TryReadConstant() {
    bool actual_constant = false;
    bool positive = true;
    int64_t value = 0;

    char ch = in_->get();
    if (ch == '-') {
        positive = false;
    } else if (std::isdigit(ch)) {
        actual_constant = true;
        value = ch - '0';
    } else if (ch != '+') {
        in_->putback(ch);
        return false;
    }

    if (std::isdigit(in_->peek())) {
        actual_constant = true;
    }

    if (!actual_constant) {
        in_->putback(ch);
        return false;
    }

    while (std::isdigit(ch = in_->peek())) {
        value = value * 10 + ch - '0';
        in_->get();
    }

    value = value * (positive ? 1 : -1);
    token_ = ConstantToken{value};
    return true;
}

bool Tokenizer::TryReadSymbol() {
    auto is_symbol_start = [](const char ch) -> bool {
        return std::isalpha(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '*' || ch == '/' ||
               ch == '#';
    };

    auto is_symbol_tail = [is_symbol_start](const char ch) -> bool {
        return is_symbol_start(ch) || std::isdigit(ch) || ch == '?' || ch == '!' || ch == '-';
    };

    char ch = in_->get();
    if (ch == '+' || ch == '-') {
        token_ = SymbolToken(std::string(1, ch));
        return true;
    }

    if (!is_symbol_start(ch)) {
        in_->putback(ch);
        return false;
    }

    std::string s;
    s += ch;

    while (is_symbol_tail(ch = in_->peek())) {
        in_->get();
        s += ch;
    }

    token_ = SymbolToken(s);
    return true;
}
