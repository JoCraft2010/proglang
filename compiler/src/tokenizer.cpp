#include "tokenizer.h"

proglang::Tokenizer::Tokenizer(std::ifstream& stream, std::filesystem::path _path) {
  path = _path;
  char c;
  std::string buf = "";
  int cline;
  int nline = 1;
  while (stream.get(c)) {
    buf.push_back(c);
    cline = nline;
    if (c == '\n') {
      nline++;
    }
    if (buf.substr(0, 2) == "//") {
      if (c == '\n') {
        buf = "";
      }
      continue;
    }
    if (buf.substr(0, 2) == "/*") {
      if (buf[buf.size() - 2] != '*' || c != '/') {
        continue;
      } else if (buf.size() > 1) {
        buf = "";
      }
    }
    if (buf.substr(0, 1) == "\"") {
      if (c != '"') {
        continue;
      } else if (buf.size() > 1) {
        tokens.push_back({ token_type: proglang::TokenType::STRING_LITERAL, data: buf.substr(1, buf.size() - 2), line: cline });
        buf = "";
      }
    }
    if (c == ' ' || c == '\n' || c == '\t') {
      if (buf.size() == 1) {
        buf = "";
      } else {
        buf = buf.substr(0, buf.size() - 1);
        tokens.push_back(bufToToken(buf, cline));
        buf = "";
      }
      continue;
    }
    if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ';' || c == ',' || c == '>' || c == '<' || c == '=' || c == '+' || c == '-' || c == '%') {
      if (buf.size() > 1) {
        buf = buf.substr(0, buf.size() - 1);
        tokens.push_back(bufToToken(buf, cline));
        buf = c;
      }
      tokens.push_back(bufToToken(buf, cline));
      buf = "";
    }
  }
}

proglang::Tokenizer::~Tokenizer() { }

proglang::Token proglang::Tokenizer::bufToToken(std::string& buf, int l) {
  if (buf == "(") {
    return { token_type: TokenType::BRACKET_OPEN, line: l };
  } else if (buf == ")") {
    return { token_type: TokenType::BRACKET_CLOSE, line: l };
  } else if (buf == "{") {
    return { token_type: TokenType::CURLY_BRACKET_OPEN, line: l };
  } else if (buf == "}") {
    return { token_type: TokenType::CURLY_BRACKET_CLOSE, line: l };
  } else if (buf == "[") {
    return { token_type: TokenType::SQUARE_BRACKET_OPEN, line: l };
  } else if (buf == "]") {
    return { token_type: TokenType::SQUARE_BRACKET_CLOSE, line: l };
  } else if (buf == ",") {
    return { token_type: TokenType::COMMA, line: l };
  } else if (buf == ";") {
    return { token_type: TokenType::SEMICOLON, line: l };
  } else if (buf == "=") {
    return { token_type: TokenType::EQUAL, line: l };
  } else if (buf == ">") {
    return { token_type: TokenType::BIGGER_THAN, line: l };
  } else if (buf == "<") {
    return { token_type: TokenType::SMALLER_THAN, line: l };
  } else if (buf == "+") {
    return { token_type: TokenType::PLUS, line: l };
  } else if (buf == "-") {
    return { token_type: TokenType::MINUS, line: l };
  } else if (buf == "%") {
    return { token_type: TokenType::MODULO, line: l };
  } else if (buf == "return") {
    return { token_type: TokenType::RETURN, line: l };
  } else if (buf == "if") {
    return { token_type: TokenType::IF, line: l };
  } else if (buf == "while") {
    return { token_type: TokenType::WHILE, line: l };
  } else if (buf == "@ext") {
    return { token_type: TokenType::EXTERNAL_DEFINITION, line: l };
  } else if (buf == "void") {
    return { token_type: TokenType::VOID_T, line: l };
  } else if (buf == "bool") {
    return { token_type: TokenType::I1_T, line: l };
  } else if (buf == "char") {
    return { token_type: TokenType::I8_T, line: l };
  } else if (buf == "short") {
    return { token_type: TokenType::I16_T, line: l };
  } else if (buf == "int") {
    return { token_type: TokenType::I32_T, line: l };
  } else if (buf == "str") {
    return { token_type: TokenType::STR_T, line: l };
  } else if (buf == "void*") {
    return { token_type: TokenType::VOID_PTR, line: l };
  } else if (buf == "~") {
    return { token_type: TokenType::TILDE, line: l };
  } else {
    return { token_type: TokenType::IDENTIFIER, data: buf, line: l };
  }
}

std::vector<proglang::Token> proglang::Tokenizer::getCombinedTokens() {
  std::vector<proglang::Token> t;
  for (long unsigned int i = 0; i < tokens.size(); i++) {
    if (tokens[i].token_type == proglang::TokenType::EXTERNAL_DEFINITION && tokens[i + 1].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 2].token_type == proglang::TokenType::SEMICOLON) {
      std::filesystem::path np = path.parent_path();
      np.append(tokens[i + 1].data.value() + ".proglang");
      std::ifstream ifilestream(np);
      if (!ifilestream) {
        ERROR("File " << np.string() << " does not exist." << std::endl);
        exit(1);
      }
      Tokenizer tok(ifilestream, path);
      ifilestream.close();
      std::vector<proglang::Token> nt = tok.getCombinedTokens();
      t.insert(t.end(), nt.begin(), nt.end());
      i += 3;
    }
    t.push_back(tokens[i]);
  }
  return t;
};
