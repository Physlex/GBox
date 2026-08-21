/**
 * TODO: DOCS
 */

#include "gbox/proc/syn/tokens.hpp"

#include <memory>
#include <string>

using namespace gbox::tokens;

std::string TokenStream::toString(uint32_t &cursor) const {
    std::string res;

    for (const auto &node : *this) {
        node.match(
            overloaded{
                [&res, &cursor](const Ident &i) {
                    const auto start = i.span().start();
                    for (; cursor < start; cursor += 1) {
                        res.push_back(' ');
                    }
                    res.append(i.symbol());
                    cursor += i.span().length();
                },
                [&res, &cursor](const Literal &l) {
                    const auto start = l.span().start();
                    for (; cursor < start; cursor += 1) {
                        res.push_back(' ');
                    }
                    res.append(l.symbol());
                    cursor += l.span().length();
                },
                [&res, &cursor](const Punc &p) {
                    const auto start = p.span().start();
                    for (; cursor < start; cursor += 1) {
                        res.push_back(' ');
                    }
                    res.append((p.symbol() == ";") ? p.symbol() + "\n" : p.symbol());
                    cursor += p.span().length();
                },
                [&res, &cursor](const std::unique_ptr<Group> &g) {
                    std::string l_delim;
                    std::string r_delim;

                    switch (g->kind()) {
                        case Group::Delimiter::Brace: {
                            l_delim = "{\n";
                            r_delim = "}\n";
                            break;
                        };

                        case Group::Delimiter::Parenthesis: {
                            l_delim = "(";
                            r_delim = ")";
                            break;
                        };

                        case Group::Delimiter::Square: {
                            l_delim = "[";
                            r_delim = "]";
                            break;
                        };
                    }

                    const auto start = g->span().start();
                    for (; cursor < start; cursor += 1) {
                        res.push_back(' ');
                    }
                    res.append(l_delim);
                    cursor += 1;

                    res.append(g->tree().toString(cursor));

                    const auto length = g->span().length();
                    for (; cursor < length; cursor += 1) {
                        res.push_back(' ');
                    }
                    res.append(r_delim);
                    cursor += 1;
                },
            }
        );
    }

    return res;
}

TokenTree::TokenTree(Literal value) : value_(std::move(value)), kind_(Kind::Literal) {}
TokenTree::TokenTree(Ident value) : value_(std::move(value)), kind_(Kind::Ident) {}
TokenTree::TokenTree(Punc value) : value_(std::move(value)), kind_(Kind::Punc) {}
TokenTree::TokenTree(std::unique_ptr<Group> value)
    : value_(std::move(value)), kind_(Kind::Group) {}

TokenTree::Value::~Value() {}

TokenTree::Value::Value(Literal value) : literal(std::move(value)) {}
TokenTree::Value::Value(Ident value) : ident(std::move(value)) {}
TokenTree::Value::Value(Punc value) : punc(std::move(value)) {}
TokenTree::Value::Value(std::unique_ptr<Group> value) : group(std::move(value)) {}

TokenTree::~TokenTree() {
    switch (this->kind_) {
        case TokenTree::Kind::Group:
            this->value_.group.~unique_ptr<Group>();
            break;
        case TokenTree::Kind::Ident:
            this->value_.ident.~Ident();
            break;
        case TokenTree::Kind::Punc:
            this->value_.punc.~Punc();
            break;
        case TokenTree::Kind::Literal:
            this->value_.literal.~Literal();
            break;
    }
}
