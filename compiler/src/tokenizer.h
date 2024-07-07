#pragma once

#include "macros.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace proglang {

  enum class TokenType {
    BRACKET_OPEN,
    BRACKET_CLOSE,
    CURLY_BRACKET_OPEN,
    CURLY_BRACKET_CLOSE,
    SQUARE_BRACKET_OPEN,
    SQUARE_BRACKET_CLOSE,
    COMMA,
    SEMICOLON,
    EQUAL,
    BIGGER_THAN,
    SMALLER_THAN,
    PLUS,
    MINUS,
    MODULO,
    RETURN,
    IF,
    WHILE,
    EXTERNAL_DEFINITION,
    VOID_T,
    I1_T,
    I8_T,
    I16_T,
    I32_T,
    STR_T,
    VOID_PTR,
    STRING_LITERAL,
    TILDE,
    IDENTIFIER
  };

  struct Token {
    TokenType token_type;
    std::optional<std::string> data;
    int line;
    std::string toString() {
      if (token_type == TokenType::BRACKET_OPEN) {
        return "BRACKET_OPEN";
      } else if (token_type == TokenType::BRACKET_CLOSE) {
        return "BRACKET_CLOSE";
      } else if (token_type == TokenType::CURLY_BRACKET_OPEN) {
        return "CURLY_BRACKET_OPEN";
      } else if (token_type == TokenType::CURLY_BRACKET_CLOSE) {
        return "CURLY_BRACKET_CLOSE";
      } else if (token_type == TokenType::SQUARE_BRACKET_OPEN) {
        return "SQUARE_BRACKET_OPEN";
      } else if (token_type == TokenType::SQUARE_BRACKET_CLOSE) {
        return "SQUARE_BRACKET_CLOSE";
      } else if (token_type == TokenType::COMMA) {
        return "COMMA";
      } else if (token_type == TokenType::SEMICOLON) {
        return "SEMICOLON";
      } else if (token_type == TokenType::EQUAL) {
        return "EQUAL";
      } else if (token_type == TokenType::BIGGER_THAN) {
        return "BIGGER_THAN";
      } else if (token_type == TokenType::SMALLER_THAN) {
        return "SMALLER_THAN";
      } else if (token_type == TokenType::PLUS) {
        return "PLUS";
      } else if (token_type == TokenType::MINUS) {
        return "MINUS";
      } else if (token_type == TokenType::MODULO) {
        return "MODULO";
      } else if (token_type == TokenType::RETURN) {
        return "RETURN";
      } else if (token_type == TokenType::IF) {
        return "IF";
      } else if (token_type == TokenType::WHILE) {
        return "WHILE";
      } else if (token_type == TokenType::EXTERNAL_DEFINITION) {
        return "EXTERNAL_DEFINITION";
      } else if (token_type == TokenType::VOID_T) {
        return "VOID_T";
      } else if (token_type == TokenType::I1_T) {
        return "I1_T";
      } else if (token_type == TokenType::I8_T) {
        return "I8_T";
      }else if (token_type == TokenType::I16_T) {
        return "I16_T";
      } else if (token_type == TokenType::I32_T) {
        return "I32_T";
      } else if (token_type == TokenType::STR_T) {
        return "STR_T";
      } else if (token_type == TokenType::VOID_PTR) {
        return "VOID_PTR";
      } else if (token_type == TokenType::STRING_LITERAL) {
        return "STRING_LITERAL: " + data.value();
      } else if (token_type == TokenType::TILDE) {
        return "TILDE";
      } else if (token_type == TokenType::IDENTIFIER) {
        if (data.has_value()) {
          return "IDENTIFIER: " + data.value();
        } else {
          return "UNKNOWN IDENTIFIER";
        }
      }
      return "<UNKNOWN>";
    }
    bool isType() {
      return token_type == TokenType::VOID_T || token_type == TokenType::I1_T || token_type == TokenType::I8_T || token_type == TokenType::I16_T || token_type == TokenType::I32_T || token_type == TokenType::STR_T || token_type == TokenType::VOID_PTR;
    }
    std::string typeToPrimitive() {
      if (token_type == TokenType::VOID_T) {
        return "void";
      } else if (token_type == TokenType::I1_T) {
        return "i1";
      } else if (token_type == TokenType::I8_T) {
        return "i8";
      } else if (token_type == TokenType::I16_T) {
        return "i16";
      } else if (token_type == TokenType::I32_T) {
        return "i32";
      } else if (token_type == TokenType::STR_T || token_type == TokenType::VOID_PTR) {
        return "ptr";
      } else {
        ERROR("\"" << toString() << "\" is not a primitive type." << std::endl);
        return "err";
      }
    }
    std::string typeToValidator() {
      if (token_type == TokenType::I1_T) {
        return "bool";
      } else if (token_type == TokenType::I8_T || token_type == TokenType::I16_T || token_type == TokenType::I32_T) {
        return "int.";
      } else if (token_type == TokenType::STR_T) {
        return "str.";
      } else {
        ERROR("\"" << toString() << "\" is not a primitive type avialable for inline expressions." << std::endl);
        return "err.";
      }
    }
    int primitiveSize() {
      if (token_type == TokenType::I1_T) {
        return 1;
      } else if (token_type == TokenType::I8_T) {
        return 1;
      } else if (token_type == TokenType::I16_T) {
        return 2;
      } else if (token_type == TokenType::I32_T) {
        return 4;
      } else if (token_type == TokenType::STR_T) {
        return 8;
      } else {
        ERROR("\"" << toString() << "\" is not a primitive type to which a definite size can be assigned." << std::endl);
        return 0;
      }
    }
    bool isComparingOperator() {
      return token_type == TokenType::BIGGER_THAN || token_type == TokenType::SMALLER_THAN || token_type == TokenType::EQUAL;
    }
    std::string comparingOperatorToPrimitive() {
      if (token_type == TokenType::BIGGER_THAN) {
        return "sgt";
      } else if (token_type == TokenType::SMALLER_THAN) {
        return "slt";
      } else if (token_type == TokenType::EQUAL) {
        return "eq";
      } else {
        ERROR("\"" << toString() << "\" is not a primitive comparing operator." << std::endl);
        return "err";
      }
    }
  };

  class Tokenizer {
  public:
    Tokenizer(std::ifstream&);
    ~Tokenizer();

    std::vector<Token> getCombinedTokens();
  private:
    Token bufToToken(std::string&, int);

    std::vector<proglang::Token> tokens;
  };

}
