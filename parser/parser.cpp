#include "parser/parser.h"

#include "runtime/error.h"
#include "runtime/object.h"
#include "tokenizer/tokenizer.h"

#include <variant>

namespace {

void ThrowUnexpectedEOF() {
    throw SyntaxError("Unexpected end of input");
}

void ThrowUnexpectedToken() {
    throw SyntaxError("Unexpected token");
}

void ThrowUnclosedBracket() {
    throw SyntaxError("Expected closing bracket");
}

Object* ReadList(Tokenizer* tokenizer) {
    auto& heap = Heap::Instance();

    if (tokenizer->IsEnd()) {
        ThrowUnexpectedEOF();
    }

    if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        tokenizer->Next();
        return nullptr;
    }

    auto* head = Read(tokenizer);

    if (tokenizer->IsEnd()) {
        ThrowUnexpectedEOF();
    }

    if (tokenizer->GetToken() == Token{DotToken{}}) {
        tokenizer->Next();
        if (tokenizer->IsEnd()) {
            ThrowUnexpectedEOF();
        }
        auto* tail = Read(tokenizer);
        if (tokenizer->IsEnd() || tokenizer->GetToken() != Token{BracketToken::CLOSE}) {
            ThrowUnclosedBracket();
        }
        tokenizer->Next();
        return MakeCell(heap, head, tail);
    }

    return MakeCell(heap, head, ReadList(tokenizer));
}

} // namespace

Object* Read(Tokenizer* tokenizer) {
    auto& heap = Heap::Instance();

    if (tokenizer->IsEnd()) {
        ThrowUnexpectedEOF();
    }

    Token token = tokenizer->GetToken();

    if (auto* constant = std::get_if<ConstantToken>(&token)) {
        tokenizer->Next();
        return MakeNumber(heap, constant->value);
    }

    if (auto* symbol = std::get_if<SymbolToken>(&token)) {
        tokenizer->Next();
        if (symbol->name == "#t") {
            return MakeBoolean(heap, true);
        }
        if (symbol->name == "#f") {
            return MakeBoolean(heap, false);
        }
        return MakeSymbol(heap, symbol->name);
    }

    if (std::holds_alternative<QuoteToken>(token)) {
        tokenizer->Next();
        auto* quoted = Read(tokenizer);
        return MakeCell(heap, MakeSymbol(heap, "quote"), MakeCell(heap, quoted, nullptr));
    }

    if (std::holds_alternative<DotToken>(token) || token == Token{BracketToken::CLOSE}) {
        ThrowUnexpectedToken();
    }

    if (token != Token{BracketToken::OPEN}) {
        ThrowUnexpectedToken();
    }

    tokenizer->Next();
    return ReadList(tokenizer);
}
