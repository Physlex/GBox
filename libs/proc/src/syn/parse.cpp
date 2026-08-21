/**
 * @brief This module implements the parser type.
 */

#include "gbox/proc/syn/parse.hpp"

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TokenKinds.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/LiteralSupport.h>
#include <llvm/Support/MemoryBufferRef.h>

#include <memory>

#include "clang/Lex/Token.h"
#include "gbox/proc/syn/tokens.hpp"
#include "llvm/ADT/StringRef.h"

using namespace gbox::parse;
using namespace gbox::tokens;

namespace tok {

static inline bool isLeftDelimiter(const clang::tok::TokenKind kind) {
    return clang::tok::l_paren == kind || clang::tok::l_brace == kind ||
           clang::tok::l_square == kind;
}

static inline bool isRightDelimiter(const clang::tok::TokenKind kind) {
    return clang::tok::r_paren == kind || clang::tok::r_brace == kind ||
           clang::tok::r_square == kind;
}

bool isDelimiter(const clang::tok::TokenKind kind) {
    return isRightDelimiter(kind) || isLeftDelimiter(kind);
}
}  // namespace tok

bool Parser::parse(
    TokenStream &in, const clang::Token tok, const llvm::StringRef symbol, Span span
) {
    // Determine what highest-level AST node the token is going to be parsed into

    if (tok.isAnyIdentifier()) {
        return this->parse_identifiers(in, tok, symbol, span);
    }

    if (tok.isLiteral()) {
        return this->parse_literals(in, tok, symbol, span);
    } else if (tok::isLeftDelimiter(tok.getKind())) {
        return this->parse_group(in, tok, symbol, span);
    } else if (tok::isRightDelimiter(tok.getKind())) {
        llvm::outs() << "Parser failed to notice left delim for token: " << tok.getKind()
                     << "\n";
        return false;
    } else {
        return this->parse_punctuation(in, tok, symbol, span);
    }
}

bool Parser::parse_identifiers(
    TokenStream &in, const clang::Token tok, const llvm::StringRef symbol, Span span
) {
    // Identifiers are easy, just check if they are user-defined or not
    in.emplace_back(Ident(symbol.str(), span, tok.is(clang::tok::raw_identifier)));
    this->idx_ += 1;
    return true;
}

bool Parser::parse_literals(
    TokenStream &in, const clang::Token tok, const llvm::StringRef symbol, Span span
) {
    const auto kind = tok.getKind();

    // If it's a string literal, store the whole text
    if (clang::tok::isStringLiteral(kind)) {
        in.emplace_back(Literal(symbol.str(), span, Literal::Kind::String));
        this->idx_ += 1;
        return true;
    }

    // Else, check the kind of literal. If it's a number, parse the type of number
    // (Floating, Integral)
    auto reduced_literal_kind = Literal::Kind::String;
    switch (kind) {
        case clang::tok::char_constant:
        case clang::tok::wide_char_constant:
        case clang::tok::utf8_char_constant:
        case clang::tok::utf16_char_constant:
        case clang::tok::utf32_char_constant: {
            reduced_literal_kind = Literal::Kind::Char;
            break;
        }
        case clang::tok::binary_data: {
            reduced_literal_kind = Literal::Kind::Binary;
            break;
        }
        case clang::tok::numeric_constant: {
            return this->parse_numerics(in, tok, symbol, span);
            break;
        }
        default: {
            llvm::outs()
                << "parse_literals: TODO: Add a diagnostics invocation here...\n";
            return false;
        }
    }

    in.emplace_back(Literal(symbol.str(), span, reduced_literal_kind));
    this->idx_ += 1;
    return true;
}

bool Parser::parse_group(
    TokenStream &in, const clang::Token tok, const llvm::StringRef slice, Span span
) {
    // Store the group kind and associated matching token

    auto reduced_kind = Group::Delimiter::Brace;
    clang::tok::TokenKind matching_kind = clang::tok::r_brace;
    const auto kind = tok.getKind();
    switch (kind) {
        case clang::tok::l_brace:
            reduced_kind = Group::Delimiter::Brace;
            matching_kind = clang::tok::r_brace;
            break;
        case clang::tok::l_paren:
            reduced_kind = Group::Delimiter::Parenthesis;
            matching_kind = clang::tok::r_paren;
            break;
        case clang::tok::l_square: {
            reduced_kind = Group::Delimiter::Square;
            matching_kind = clang::tok::r_square;
            break;
        }
        default: {
            llvm::outs() << "Clang failed to match group from token: " << slice << "\n";
            return false;
        }
    }

    // Find the (head, tail) representing the slice of the buffer for our next group
    // TODO: There has to be a way to solve this that doesn't decimate the branch check

    const size_t head = this->idx_;
    size_t tail = head;
    int depth = 1;
    const size_t raw_tokens_cnt = this->clang_.raw_tokens.size();
    while (depth != 0) {
        tail += 1;
        if (tail >= raw_tokens_cnt) {
            llvm::outs() << "No matching kind found for delimiter\n";
            return false;
        }

        const clang::Token tail_match = this->clang_.raw_tokens.at(tail);
        const auto tail_kind = tail_match.getKind();
        if (tail_kind == kind) {
            depth++;
        } else if (tail_kind == matching_kind) {
            depth--;
        }
    }

    const clang::Token most_recent_match = this->clang_.raw_tokens.at(tail);
    const uint32_t r_delim_start =
        this->clang_.sm.getFileOffset(most_recent_match.getLocation());

    // skip the delimiter
    this->idx_ += 1;

    // Recursively parse each token within the inner group
    TokenStream inner;
    while ((this->idx_ < tail) && (this->parse_aux(inner))) {
        ;
    }
    if (this->idx_ != tail) {
        return false;
    }

    in.emplace_back(
        std::make_unique<Group>(
            std::move(inner), Span(span.start(), r_delim_start), reduced_kind
        )
    );

    this->idx_ += 1;
    return true;
}

bool Parser::parse_punctuation(
    TokenStream &in, const clang::Token tok, const llvm::StringRef symbol, Span span
) {
    // TODO: Add support for joinable operators
    in.emplace_back(Punc(symbol.str(), span, false));
    this->idx_ += 1;
    return true;
}

bool Parser::parse_numerics(
    TokenStream &in, const clang::Token tok, const llvm::StringRef symbol, Span span
) {
    auto number = clang::NumericLiteralParser(
        symbol, tok.getLocation(), this->clang_.sm,
        this->clang_.res.Context->getLangOpts(),
        this->clang_.res.Context->getTargetInfo(), this->clang_.diagnostics
    );

    if (number.isFloatingLiteral()) {
        in.emplace_back(Literal(symbol.str(), span, Literal::Kind::Float));
    } else if (number.isIntegerLiteral()) {
        in.emplace_back(Literal(symbol.str(), span, Literal::Kind::Integer));
    } else {
        return false;
    }

    this->idx_ += 1;
    return true;
}
