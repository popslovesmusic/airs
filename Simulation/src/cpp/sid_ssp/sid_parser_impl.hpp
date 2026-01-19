/**
 * SID Expression Parser - C++ Implementation
 *
 * Parses SID expressions like "P(Freedom)", "O(S+(Peace))", etc.
 * Port from sids/sid_parser.py
 */

#pragma once

#include "sid_ast.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <set>

namespace sid {

// ParseError is defined in sid_ast.hpp

// Valid operators
static const std::set<std::string> OPERATORS = {"P", "S+", "S-", "O", "C", "T"};

// Token types
enum class TokenKind {
    OPERATOR,   // P, S+, S-, O, C, T
    IDENT,      // Identifier (DOF name or variable)
    LPAREN,     // (
    RPAREN,     // )
    COMMA,      // ,
    END         // End of input
};

struct Token {
    TokenKind kind;
    std::string value;
    size_t pos;

    Token(TokenKind k, const std::string& v, size_t p)
        : kind(k), value(v), pos(p) {}
};

/**
 * Tokenizer - converts string to tokens
 */
class Tokenizer {
private:
    std::string text_;
    size_t pos_;

    void skipWhitespace() {
        while (pos_ < text_.size() && std::isspace(text_[pos_])) {
            pos_++;
        }
    }

    bool tryParseOperator(Token& token) {
        // Try two-character operators first (S+, S-)
        if (pos_ + 1 < text_.size()) {
            std::string two_char = text_.substr(pos_, 2);
            if (OPERATORS.count(two_char)) {
                token = Token(TokenKind::OPERATOR, two_char, pos_);
                pos_ += 2;
                return true;
            }
        }

        // Try single-character operators (P, O, C, T)
        if (pos_ < text_.size()) {
            std::string one_char = text_.substr(pos_, 1);
            if (OPERATORS.count(one_char)) {
                token = Token(TokenKind::OPERATOR, one_char, pos_);
                pos_++;
                return true;
            }
        }

        return false;
    }

    bool tryParseIdentifier(Token& token) {
        if (pos_ >= text_.size()) return false;

        char first = text_[pos_];
        // Identifiers start with $ (for variables) or letter or underscore
        if (first != '$' && !std::isalpha(first) && first != '_') {
            return false;
        }

        size_t start = pos_;
        pos_++;

        // Continue with alphanumeric or underscore
        while (pos_ < text_.size() &&
               (std::isalnum(text_[pos_]) || text_[pos_] == '_')) {
            pos_++;
        }

        token = Token(TokenKind::IDENT, text_.substr(start, pos_ - start), start);
        return true;
    }

public:
    explicit Tokenizer(const std::string& text) : text_(text), pos_(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos_ < text_.size()) {
            skipWhitespace();
            if (pos_ >= text_.size()) break;

            size_t start_pos = pos_;
            Token token(TokenKind::END, "", pos_);

            // Try to parse operator
            if (tryParseOperator(token)) {
                tokens.push_back(token);
                continue;
            }

            // Try to parse identifier
            if (tryParseIdentifier(token)) {
                tokens.push_back(token);
                continue;
            }

            // Single character tokens
            char ch = text_[pos_];
            if (ch == '(') {
                tokens.push_back(Token(TokenKind::LPAREN, "(", pos_));
                pos_++;
                continue;
            }
            if (ch == ')') {
                tokens.push_back(Token(TokenKind::RPAREN, ")", pos_));
                pos_++;
                continue;
            }
            if (ch == ',') {
                tokens.push_back(Token(TokenKind::COMMA, ",", pos_));
                pos_++;
                continue;
            }

            // Unknown character
            throw ParseError("Unexpected character '" + std::string(1, ch) +
                           "' at position " + std::to_string(pos_));
        }

        return tokens;
    }
};

/**
 * Recursive descent parser
 */
class Parser {
private:
    std::vector<Token> tokens_;
    size_t pos_;

    const Token* current() const {
        if (pos_ >= tokens_.size()) return nullptr;
        return &tokens_[pos_];
    }

    Token consume(TokenKind expected) {
        const Token* tok = current();
        if (!tok) {
            throw ParseError("Unexpected end of input");
        }
        if (tok->kind != expected) {
            throw ParseError("Expected different token at position " +
                           std::to_string(tok->pos));
        }
        return tokens_[pos_++];
    }

    void validateArity(const std::string& op, size_t num_args, size_t pos) {
        // Operator arity constraints
        if (op == "P" || op == "O" || op == "T") {
            if (num_args != 1) {
                throw ParseError(op + " requires exactly 1 argument, got " +
                               std::to_string(num_args) + " at position " +
                               std::to_string(pos));
            }
        } else if (op == "C") {
            if (num_args != 2) {
                throw ParseError("C requires exactly 2 arguments, got " +
                               std::to_string(num_args) + " at position " +
                               std::to_string(pos));
            }
        } else if (op == "S+" || op == "S-") {
            if (num_args < 1) {
                throw ParseError(op + " requires at least 1 argument at position " +
                               std::to_string(pos));
            }
        }
    }

    ASTNode parseExpr() {
        const Token* tok = current();
        if (!tok) {
            throw ParseError("Empty expression");
        }

        if (tok->kind == TokenKind::OPERATOR) {
            std::string op = tok->value;
            size_t op_pos = tok->pos;
            consume(TokenKind::OPERATOR);

            // Check for parentheses
            std::vector<ASTNode> args;
            const Token* next = current();
            if (next && next->kind == TokenKind::LPAREN) {
                consume(TokenKind::LPAREN);
                args = parseExprList();
                consume(TokenKind::RPAREN);
            }

            // Validate arity
            validateArity(op, args.size(), op_pos);

            return ASTNode::makeOp(op, args);
        }

        if (tok->kind == TokenKind::IDENT) {
            std::string name = tok->value;
            consume(TokenKind::IDENT);
            return ASTNode::makeAtom(name);
        }

        throw ParseError("Unexpected token at position " + std::to_string(tok->pos));
    }

    std::vector<ASTNode> parseExprList() {
        std::vector<ASTNode> exprs;
        exprs.push_back(parseExpr());

        while (true) {
            const Token* tok = current();
            if (!tok || tok->kind != TokenKind::COMMA) break;
            consume(TokenKind::COMMA);
            exprs.push_back(parseExpr());
        }

        return exprs;
    }

public:
    explicit Parser(const std::vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}

    ASTNode parse() {
        ASTNode expr = parseExpr();

        // Check for trailing tokens
        if (current()) {
            throw ParseError("Unexpected trailing tokens at position " +
                           std::to_string(current()->pos));
        }

        return expr;
    }
};

/**
 * Parse SID expression string to AST
 *
 * @param text Expression string (e.g., "P(Freedom)", "O(S+(Peace))")
 * @return Parsed AST node
 * @throws ParseError on syntax error
 */
inline ASTNode parseExpression(const std::string& text) {
    Tokenizer tokenizer(text);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    return parser.parse();
}

} // namespace sid
