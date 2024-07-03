#include "parser.h"

proglang::ParserElement::ParserElement(proglang::ParserElement* _parent, proglang::ParserElementType _type, std::vector<std::string> _data) {
  parent = _parent;
  type = _type;
  data = _data;
}

proglang::ParserElement::~ParserElement() { }

void proglang::ParserElement::printTree(std::string off) {
  if (type == ParserElementType::ROOT) {
    std::cout << off << "ROOT (";
  } else if (type == ParserElementType::FUNCTION) {
    std::cout << off << "FUNCTION (";
  } else if (type == ParserElementType::EXTERNAL_FUNCTION) {
    std::cout << off << "EXTERNAL_FUNCTION (";
  } else if (type == ParserElementType::FUNCTION_CALL) {
    std::cout << off << "FUNCTION_CALL (";
  } else if (type == ParserElementType::VARIABLE_DECLARATION) {
    std::cout << off << "VARIABLE_DECLARATION (";
  } else if (type == ParserElementType::VARIABLE_DECLARATION_EMPTY) {
    std::cout << off << "VARIABLE_DECLARATION_EMPTY (";
  } else if (type == ParserElementType::VARIABLE_ASSIGNMENT) {
    std::cout << off << "VARIABLE_ASSIGNMENT (";
  } else if (type == ParserElementType::VARIABLE_ASSIGNMENT_TO_FUNCTION_CALL) {
    std::cout << off << "VARIABLE_ASSIGNMENT_TO_FUNCTION_CALL (";
  } else if (type == ParserElementType::VARIABLE_ASSIGNMENT_TO_INDEX) {
    std::cout << off << "VARIABLE_ASSIGNMENT_TO_INDEX (";
  } else if (type == ParserElementType::VARIABLE_INDEX_ASSIGNMENT) {
    std::cout << off << "VARIABLE_INDEX_ASSIGNMENT (";
  } else if (type == ParserElementType::VARIABLE_INCREMENT) {
    std::cout << off << "VARIABLE_INCREMENT (";
  } else if (type == ParserElementType::VARIABLE_DECREMENT) {
    std::cout << off << "VARIABLE_DECREMENT (";
  } else if (type == ParserElementType::IF) {
    std::cout << off << "IF (";
  } else if (type == ParserElementType::IF_SINGLE_BOOL) {
    std::cout << off << "IF_SINGLE_BOOL (";
  } else if (type == ParserElementType::WHILE) {
    std::cout << off << "WHILE (";
  } else if (type == ParserElementType::WHILE_SINGLE_BOOL) {
    std::cout << off << "WHILE_SINGLE_BOOL (";
  } else if (type == ParserElementType::RETURN) {
    std::cout << off << "RETURN (";
  } else {
    std::cout << off << "<UNKNOWN> (";
  }
  for (std::string s : data) {
    std::cout << s << " ";
  }
  std::cout << ")" << std::endl;
  for (proglang::ParserElement e : children) {
    e.printTree(off + "  ");
  }
}

std::string proglang::ParserElement::buildLlvmIr(std::vector<std::string> _data, LlvmIrExtendedParserData& extData) {
  std::string d;
  std::vector<std::string> ed;
  if (type == proglang::ParserElementType::FUNCTION) {
    extData.var_decls = "";
    extData.var_inits = "";
    d += "define dso_local " + data[0] + " @" + data[1] + "() " + extData.registerAttrs("noinline nounwind optnone sspstrong uwtable") + " {\n[var_decls][var_inits]";
    if (data[1] == "main") {
      extData.snum_count++;
      extData.var_decls += "%1 = alloca i32, align 4\n";
      extData.var_inits += "store i32 0, ptr %1, align 4\n";
    }
  } else if (type == proglang::ParserElementType::EXTERNAL_FUNCTION) {
    extData.ext_defs.push_back("declare " + data[0] + " @" + data[1] + "(");
    std::string func_def = data[0] + " (";
    for (long unsigned int i = 2; i < data.size(); i++) {
      extData.ext_defs.back() += data[i];
      func_def += data[i];
      if (data[i] != "...") {
        extData.ext_defs.back() += " noundef";
      }
      if (i < data.size() - 1) {
        extData.ext_defs.back() += ", ";
        func_def += ", ";
      }
    }
    extData.ext_defs.back() += ") " + extData.registerAttrs("");
    func_def += ") @" + data[1];
    extData.func_signs[data[1]] = func_def;
  } else if (type == proglang::ParserElementType::FUNCTION_CALL) {
    if (extData.func_signs[data[0]].substr(0, 4) == "void") {
      d += "[temp0]call " + extData.func_signs[data[0]] + "(";
    } else {
      d += "[temp0]%[temp1] = call " + extData.func_signs[data[0]] + "(";
    }
    std::string temp0;
    for (long unsigned int i = 1; i < data.size(); i++) {
      if (data[i].substr(0, 4) == "str.") {
        int s;
        d += "ptr noundef " + extData.registerStringConst(data[i].substr(4, data[i].size() - 1), s);
      } else if (data[i].substr(0, 4) == "any.") {
        std::string ap;
        temp0 += extData.obtainVariableAccess(data[i].substr(4, data[i].size() - 1), _data[1], ap);
        d += extData.var_types[data[i].substr(4, data[i].size() - 1)] + " " + ap;
      } else {
        std::cerr << "This is actually a compiler bug." << std::endl;
        exit(1);
      }
      if (i < data.size() - 1) {
        d += ", ";
      }
    }
    if (extData.func_signs[data[0]].substr(0, 4) != "void") {
      extData.snum_count++;
      d = std::regex_replace(d, std::regex("\\[temp1]"), std::to_string(extData.snum_count));
    }
    d = std::regex_replace(d, std::regex("\\[temp0]"), temp0);
    d += ")\n";
  } else if (type == proglang::ParserElementType::VARIABLE_ASSIGNMENT_TO_FUNCTION_CALL) {
    d += "[temp0]%[temp1] = call " + extData.func_signs[data[1]] + "(";
    std::string temp0;
    for (long unsigned int i = 2; i < data.size(); i++) {
      if (data[i].substr(0, 4) == "str.") {
        int s;
        d += "ptr noundef " + extData.registerStringConst(data[i].substr(4, data[i].size() - 1), s);
      } else if (data[i].substr(0, 4) == "any.") {
        std::string ap;
        temp0 += extData.obtainVariableAccess(data[i].substr(4, data[i].size() - 1), _data[1], ap);
        d += extData.var_types[data[i].substr(4, data[i].size() - 1)] + " " + ap;
      } else {
        std::cerr << "This is actually a compiler bug." << std::endl;
        exit(1);
      }
      if (i < data.size() - 1) {
        d += ", ";
      }
    }
    extData.snum_count++;
    d = std::regex_replace(d, std::regex("\\[temp0]"), temp0);
    d = std::regex_replace(d, std::regex("\\[temp1]"), std::to_string(extData.snum_count));
    d += ")\n";
    d += extData.storeVariableValue(data[0], _data[1], "%" + std::to_string(extData.snum_count));
  } else if (type == proglang::ParserElementType::VARIABLE_DECLARATION) {
    if (data[2].substr(0, 4) == "str.") {
      int s;
      std::string ap = extData.registerStringConst(data[2].substr(4, data[2].size() - 1), s);
      extData.arr_sizes[data[3]] = "[" + std::to_string(s) + " x i8]";
      extData.arr_size_nums[data[3]] = std::to_string(s);
      extData.var_decls += "%.v." + data[3] + " = alloca " + extData.arr_sizes[data[3]] + ", align 1\n";
      extData.var_inits += "call void @llvm.memcpy.p0.p0.i32(ptr align 1 %.v." + data[3] + ", ptr align 1 " + ap + ", i32 " + std::to_string(s) + ", i1 false)\n";
    } else {
      extData.var_decls += "%.v." + data[3] + " = alloca " + data[0] + ", align " + data[1] + "\n";
      extData.var_inits += "store " + data[0] + " ";
      extData.var_inits += data[2].substr(4, data[2].size() - 1);
      extData.var_inits += ", ptr %.v." + data[3] + ", align " + data[1] + "\n";
    }
    extData.var_types[data[3]] = data[0];
    extData.var_sizes[data[3]] = data[1];
  } else if (type == proglang::ParserElementType::VARIABLE_DECLARATION_EMPTY) {
    if (data[3] == "str.") {
      std::string ap = extData.registerEmptyGlobalStringVar(_data[1] + "." + data[2], data[4]);
      extData.arr_sizes[data[2]] = "[" + data[4] + " x i8]";
      extData.arr_size_nums[data[2]] = data[4];
      extData.var_decls += "%.v." + data[2] + " = alloca " + extData.arr_sizes[data[2]] + ", align 1\n";
      extData.var_inits += "call void @llvm.memcpy.p0.p0.i32(ptr align 1 %.v." + data[2] + ", ptr align 1 " + ap + ", i32 " + data[4] + ", i1 false)\n";
    } else {
      extData.var_decls += "%.v." + data[2] + " = alloca " + data[0] + ", align " + data[1] + "\n";
    }
    extData.var_types[data[2]] = data[0];
    extData.var_sizes[data[2]] = data[1];
  } else if (type == proglang::ParserElementType::VARIABLE_ASSIGNMENT) {
    if (data[1].substr(0, 4) == "str.") {
      int l;
      d += extData.storeVariableValue(data[0], _data[1], extData.registerStringConst(data[1].substr(4, data[1].size() - 1), l));
    } else {
      d += extData.storeVariableValue(data[0], _data[1], data[1].substr(4, data[1].size() - 1));
    }
  } else if (type == proglang::ParserElementType::VARIABLE_INDEX_ASSIGNMENT) {
    std::string ap0;
    std::string ap1 = data[2];
    std::string ap2 = data[1];
    if (ap1.find_first_not_of("0123456789") != std::string::npos) {
      d += extData.obtainVariableAccess(ap1, _data[1], ap1);
    }
    if (ap2.find_first_not_of("0123456789") != std::string::npos) {
      d += extData.obtainVariableAccess(ap2, _data[1], ap2);
    }
    d += extData.obtainVariableAccess(data[0], _data[1], ap0);
    extData.snum_count++;
    d += "%" + std::to_string(extData.snum_count) + " = getelementptr inbounds " + extData.arr_sizes[data[0]] + ", ptr " + ap0 + ", i32 0, i32 " + ap1 + "\nstore i8 " + ap2 + ", ptr %" + std::to_string(extData.snum_count) + ", align 1\n";
  } else if (type == proglang::ParserElementType::VARIABLE_ASSIGNMENT_TO_INDEX) {
    std::string ap0;
    std::string ap1 = data[2];
    if (ap1.find_first_not_of("0123456789") != std::string::npos) {
      d += extData.obtainVariableAccess(ap1, _data[1], ap1);
    }
    d += extData.obtainVariableAccess(data[1], _data[1], ap0);
    extData.snum_count++;
    // TODO 31
    d += "%" + std::to_string(extData.snum_count) + " = getelementptr inbounds i8, ptr " + ap0 + ", i32 " + ap1 + "\n";
    extData.snum_count++;
    d += "%" + std::to_string(extData.snum_count) + " = load " + extData.var_types[data[0]] + ", ptr %" + std::to_string(extData.snum_count - 1) + ", align " + extData.var_sizes[data[0]] + "\n";
    d += extData.storeVariableValue(data[0], _data[1], "%" + std::to_string(extData.snum_count));
  } else if (type == proglang::ParserElementType::VARIABLE_INCREMENT) {
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    extData.snum_count++;
    d += "%" + std::to_string(extData.snum_count) + " = add nsw " + extData.var_types[data[0]] + " " + ap + ", " + data[1] + "\n";
    d += extData.storeVariableValue(data[0], _data[1], "%" + std::to_string(extData.snum_count));
  } else if (type == proglang::ParserElementType::VARIABLE_DECREMENT) {
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    extData.snum_count++;
    d += "%" + std::to_string(extData.snum_count) + " = sub nsw " + extData.var_types[data[0]] + " " + ap + ", " + data[1] + "\n";
    d += extData.storeVariableValue(data[0], _data[1], "%" + std::to_string(extData.snum_count));
  } else if (type == proglang::ParserElementType::VARIABLE_MODULO) {
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    extData.snum_count++;
    d += "%" + std::to_string(extData.snum_count) + " = srem " + extData.var_types[data[0]] + " " + ap + ", " + data[1] + "\n";
    d += extData.storeVariableValue(data[0], _data[1], "%" + std::to_string(extData.snum_count));
  } else if (type == proglang::ParserElementType::IF) {
    extData.label_count += 2;
    d += extData.compare(data[0], data[1], _data[1], data[2]);
    d += "br i1 %" + std::to_string(extData.snum_count) + ", label %.l" + std::to_string(extData.label_count - 1) + ", label %.l" + std::to_string(extData.label_count) + "\n.l" + std::to_string(extData.label_count - 1) + ":\n";
    ed.push_back(".l" + std::to_string(extData.label_count));
  } else if (type == proglang::ParserElementType::IF_SINGLE_BOOL) {
    extData.label_count += 2;
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    d += "br i1 " + ap + ", label %.l" + std::to_string(extData.label_count - 1) + ", label %.l" + std::to_string(extData.label_count) + "\n.l" + std::to_string(extData.label_count - 1) + ":\n";
    ed.push_back(".l" + std::to_string(extData.label_count));
  } else if (type == proglang::ParserElementType::WHILE) {
    extData.label_count += 2;
    d += extData.compare(data[0], data[1], _data[1], data[2]);
    d += "br i1 %" + std::to_string(extData.snum_count) + ", label %.l" + std::to_string(extData.label_count - 1) + ", label %.l" + std::to_string(extData.label_count) + "\n.l" + std::to_string(extData.label_count - 1) + ":\n";
    ed.push_back(".l" + std::to_string(extData.label_count));
    ed.push_back(".l" + std::to_string(extData.label_count - 1));
  } else if (type == proglang::ParserElementType::WHILE_SINGLE_BOOL) {
    extData.label_count += 2;
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    d += "br i1 " + ap + ", label %.l" + std::to_string(extData.label_count - 1) + ", label %.l" + std::to_string(extData.label_count) + "\n.l" + std::to_string(extData.label_count - 1) + ":\n";
    ed.push_back(".l" + std::to_string(extData.label_count));
    ed.push_back(".l" + std::to_string(extData.label_count - 1));
  } else if (type == proglang::ParserElementType::RETURN) {
    if (data[0].find_first_not_of("0123456789") == std::string::npos) {
      extData.snum_count++;
      d += "ret " + _data[0] + " " + data[0] + "\n";
    } else {
      std::string ap;
      d += extData.obtainVariableAccess(data[0], _data[1], ap);
      extData.snum_count++;
      d += "ret " + _data[0] + " " + ap + "\n";
    }
  }
  for (proglang::ParserElement e : children) {
    if (type == proglang::ParserElementType::FUNCTION) {
      d += e.buildLlvmIr({ data[0], data[1] }, extData);
    } else {
      d += e.buildLlvmIr(_data, extData);
    }
  }
  if (type == proglang::ParserElementType::FUNCTION) {
    d += "}\n";
    d = std::regex_replace(d, std::regex("\\[var_decls]"), extData.var_decls);
    d = std::regex_replace(d, std::regex("\\[var_inits]"), extData.var_inits);
  } else if (type == proglang::ParserElementType::IF || type == proglang::ParserElementType::IF_SINGLE_BOOL) {
    d += "br label %" + ed[0] + "\n" + ed[0] + ":\n";
  } else if (type == proglang::ParserElementType::WHILE) {
    d += extData.compare(data[0], data[1], _data[1], data[2]);
    d += "br i1 %" + std::to_string(extData.snum_count) + ", label %" + ed[1] + ", label %" + ed[0] + "\n" + ed[0] + ":\n";
  } else if (type == proglang::ParserElementType::WHILE_SINGLE_BOOL) {
    std::string ap;
    d += extData.obtainVariableAccess(data[0], _data[1], ap);
    d += "br i1 " + ap + ", label %" + ed[1] + ", label %" + ed[0] + "\n" + ed[0] + ":\n";
  }
  return d;
}

proglang::ParserElement& proglang::ParserElement::appendChild(proglang::ParserElement e) {
  children.push_back(e);
  return children.back();
}

proglang::ParserElementType proglang::ParserElement::getType() {
  return type;
}

proglang::ParserElement* proglang::ParserElement::getParent() {
  return parent;
}

proglang::Parser::Parser(std::vector<Token> tokens) {
  for (Token t : tokens) {
    std::cout << t.toString() << std::endl;
  }
  std::cout << "-----" << std::endl;
  bool success = true;
  proglang::ParserElement* current = &root;
  // TODO 2
  for (long unsigned int i = 0; i < tokens.size(); i++) {
    if (tokens[i].isType() && tokens[i + 1].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 2].token_type == proglang::TokenType::BRACKET_OPEN) {
      current = &current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::FUNCTION, { tokens[i].typeToPrimitive(), tokens[i + 1].data.value() }));
      i += 2;
      if (tokens[i + 1].token_type != proglang::TokenType::BRACKET_CLOSE) {
        std::cerr << "Expected BRACKET_CLOSE in function definition in line " << tokens[i].line << ", got \"" << tokens[i + 1].toString() << "\" instead." << std::endl;
        success = false;
      }
      i++;
      if (tokens[i + 1].token_type != proglang::TokenType::CURLY_BRACKET_OPEN) {
        std::cerr << "Expected CURLY_BRACKET_OPEN in function definition in line " << tokens[i].line << ", got \"" << tokens[i + 1].toString() << "\" instead." << std::endl;
        success = false;
      }
      i++;
    } else if (tokens[i].isType() && tokens[i + 1].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 2].token_type == proglang::TokenType::EQUAL && tokens[i + 4].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_DECLARATION, { tokens[i].typeToPrimitive(), std::to_string(tokens[i].primitiveSize()), tokens[i].typeToValidator() + tokens[i + 3].data.value(), tokens[i + 1].data.value() }));
      i += 4;
    } else if (tokens[i].isType() && tokens[i + 1].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 2].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_DECLARATION_EMPTY, { tokens[i].typeToPrimitive(), std::to_string(tokens[i].primitiveSize()), tokens[i + 1].data.value(), tokens[i].typeToValidator(), "16" }));
      i += 2;
    } else if (tokens[i].token_type == proglang::TokenType::STR_T && tokens[i + 1].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 2].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 3].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_DECLARATION_EMPTY, { tokens[i].typeToPrimitive(), std::to_string(tokens[i].primitiveSize()), tokens[i + 1].data.value(), tokens[i].typeToValidator(), tokens[i + 2].data.value() }));
      i += 3;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::EQUAL && tokens[i + 2].data.has_value() && tokens[i + 3].token_type == proglang::TokenType::SEMICOLON) {
      if (tokens[i + 2].token_type == proglang::TokenType::STRING_LITERAL) {
        current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_ASSIGNMENT, { tokens[i].data.value(), "str." + tokens[i + 2].data.value() }));
      } else {
        current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_ASSIGNMENT, { tokens[i].data.value(), "any." + tokens[i + 2].data.value() }));
      }
      i += 3;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::EQUAL && tokens[i + 2].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 3].token_type == proglang::TokenType::SQUARE_BRACKET_OPEN && tokens[i + 4].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 5].token_type == proglang::TokenType::SQUARE_BRACKET_CLOSE && tokens[i + 6].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_ASSIGNMENT_TO_INDEX, { tokens[i].data.value(), tokens[i + 2].data.value(), tokens[i + 4].data.value() }));
      i += 6;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::SQUARE_BRACKET_OPEN && tokens[i + 2].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 3].token_type == proglang::TokenType::SQUARE_BRACKET_CLOSE && tokens[i + 4].token_type == proglang::TokenType::EQUAL && tokens[i + 5].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 6].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_INDEX_ASSIGNMENT, { tokens[i].data.value(), tokens[i + 5].data.value(), tokens[i + 2].data.value() }));
      i += 6;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::PLUS && tokens[i + 2].token_type == proglang::TokenType::PLUS && tokens[i + 3].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_INCREMENT, { tokens[i].data.value(), "1" }));
      i += 3;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::PLUS && tokens[i + 2].token_type == proglang::TokenType::EQUAL && tokens[i + 3].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 4].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_INCREMENT, { tokens[i].data.value(), tokens[i + 3].data.value() }));
      i += 4;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::MINUS && tokens[i + 2].token_type == proglang::TokenType::MINUS && tokens[i + 3].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_DECREMENT, { tokens[i].data.value(), "1" }));
      i += 3;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::MINUS && tokens[i + 2].token_type == proglang::TokenType::EQUAL && tokens[i + 3].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 4].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_DECREMENT, { tokens[i].data.value(), tokens[i + 3].data.value() }));
      i += 4;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::MODULO && tokens[i + 2].token_type == proglang::TokenType::EQUAL && tokens[i + 3].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 4].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_MODULO, { tokens[i].data.value(), tokens[i + 3].data.value() }));
      i += 4;
    } else if (tokens[i].token_type == proglang::TokenType::IF && tokens[i + 1].token_type == proglang::TokenType::BRACKET_OPEN) {
      if (tokens[i + 2].token_type != proglang::TokenType::IDENTIFIER || ((!tokens[i + 3].isComparingOperator() || tokens[i + 4].token_type != proglang::TokenType::IDENTIFIER) && tokens[i + 3].token_type != proglang::TokenType::BRACKET_CLOSE)) {
        std::cerr << "Unexpected stuff in if statement in line " << tokens[i + 2].line << "." << std::endl;
        success = false;
      }
      if (tokens[i + 3].token_type == proglang::TokenType::BRACKET_CLOSE) {
        i += 4;
        current = &current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::IF_SINGLE_BOOL, { tokens[i - 2].data.value() }));
      } else {
        i += 6;
        current = &current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::IF, { tokens[i - 4].data.value(), tokens[i - 2].data.value(), tokens[i - 3].comparingOperatorToPrimitive() }));
      }
      if (tokens[i].token_type != proglang::TokenType::CURLY_BRACKET_OPEN) {
        std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in if statement in line " << tokens[i - 1].line << ". Expected CURLY_BRACKET_OPEN instead." << std::endl;
        success = false;
      }
    } else if (tokens[i].token_type == proglang::TokenType::WHILE && tokens[i + 1].token_type == proglang::TokenType::BRACKET_OPEN) {
      if (tokens[i + 2].token_type != proglang::TokenType::IDENTIFIER || ((!tokens[i + 3].isComparingOperator() || tokens[i + 4].token_type != proglang::TokenType::IDENTIFIER) && tokens[i + 3].token_type != proglang::TokenType::BRACKET_CLOSE)) {
        std::cerr << "Unexpected stuff in while statement in line " << tokens[i + 2].line << "." << std::endl;
        success = false;
      }
      if (tokens[i + 3].token_type == proglang::TokenType::BRACKET_CLOSE) {
        i += 4;
        current = &current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::WHILE_SINGLE_BOOL, { tokens[i - 2].data.value() }));
      } else {
        i += 6;
        current = &current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::WHILE, { tokens[i - 4].data.value(), tokens[i - 2].data.value(), tokens[i - 3].comparingOperatorToPrimitive() }));
      }
      if (tokens[i].token_type != proglang::TokenType::CURLY_BRACKET_OPEN) {
        std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in if statement in line " << tokens[i - 1].line << ". Expected CURLY_BRACKET_OPEN instead." << std::endl;
        success = false;
      }
    } else if (tokens[i].token_type == proglang::TokenType::EXTERNAL_DEFINITION && tokens[i + 1].isType() && tokens[i + 2].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 3].token_type == proglang::TokenType::BRACKET_OPEN) {
      std::vector<std::string> data = { tokens[i + 1].typeToPrimitive(), tokens[i + 2].data.value() };
      i += 3;
      while (tokens[i].token_type != proglang::TokenType::BRACKET_CLOSE) {
        i++;
        if ((tokens[i].isType() || tokens[i].token_type == proglang::TokenType::TILDE) && (tokens[i + 1].token_type == proglang::TokenType::COMMA || tokens[i + 1].token_type == proglang::TokenType::BRACKET_CLOSE)) {
          if (tokens[i].isType()) {
            data.push_back(tokens[i].typeToPrimitive());
          } else {
            data.push_back("...");
          }
        } else if(tokens[i].token_type == proglang::TokenType::BRACKET_CLOSE) {
          continue;
        } else {
          std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in external function definition in line " << tokens[i].line << "." << std::endl;
          success = false;
        }
        i++;
      }
      i++;
      if (tokens[i].token_type != proglang::TokenType::SEMICOLON) {
        std::cerr << "Expected SEMICOLON after external function definition in line " << tokens[i - 1].line << ", got " << tokens[i].toString() << " instead." << std::endl;
        success = false;
      }
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::EXTERNAL_FUNCTION, data));
    } else if (tokens[i].token_type == proglang::TokenType::RETURN && tokens[i + 2].token_type == proglang::TokenType::SEMICOLON) {
      current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::RETURN, { tokens[i + 1].data.value() }));
      i += 2;
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::BRACKET_OPEN) {
      std::vector<std::string> funcdata = { tokens[i].data.value() };
      i += 2;
      while (tokens[i - 1].token_type != proglang::TokenType::BRACKET_CLOSE) {
        if (tokens[i].token_type == proglang::TokenType::STRING_LITERAL) {
          funcdata.push_back("str." + tokens[i].data.value());
        } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER) {
          funcdata.push_back("any." + tokens[i].data.value());
        } else if (tokens[i].token_type == proglang::TokenType::BRACKET_CLOSE) {
          i++;
          break;
        } else {
          std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in function call in line " << tokens[i].line << "." << std::endl;
          success = false;
        }
        i += 2;
      }
      if (tokens[i].token_type != proglang::TokenType::SEMICOLON) {
        std::cerr << "Missing SEMICOLON after function call in line " << tokens[i].line << std::endl;
        success = false;
      } else {
        current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::FUNCTION_CALL, funcdata));
      }
    } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 1].token_type == proglang::TokenType::EQUAL && tokens[i + 2].token_type == proglang::TokenType::IDENTIFIER && tokens[i + 3].token_type == proglang::TokenType::BRACKET_OPEN) {
      std::vector<std::string> funcdata = { tokens[i].data.value(), tokens[i + 2].data.value() };
      i += 4;
      while (tokens[i - 1].token_type != proglang::TokenType::BRACKET_CLOSE) {
        if (tokens[i].token_type == proglang::TokenType::STRING_LITERAL) {
          funcdata.push_back("str." + tokens[i].data.value());
        } else if (tokens[i].token_type == proglang::TokenType::IDENTIFIER) {
          funcdata.push_back("any." + tokens[i].data.value());
        } else if (tokens[i].token_type == proglang::TokenType::BRACKET_CLOSE) {
          i++;
          break;
        } else {
          std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in function call in line " << tokens[i].line << "." << std::endl;
          success = false;
        }
        i += 2;
      }
      if (tokens[i].token_type != proglang::TokenType::SEMICOLON) {
        std::cerr << "Missing SEMICOLON after function call in line " << tokens[i].line << std::endl;
        success = false;
      } else {
        current -> appendChild(proglang::ParserElement(current, proglang::ParserElementType::VARIABLE_ASSIGNMENT_TO_FUNCTION_CALL, funcdata));
      }
    } else if (tokens[i].token_type == proglang::TokenType::CURLY_BRACKET_CLOSE) {
      current = current -> getParent();
    } else {
      std::cerr << "Unexpected \"" << tokens[i].toString() << "\" in line " << tokens[i].line << "." << std::endl;
      success = false;
    }
  }
  std::cout << "-----" << std::endl;
  root.printTree("| ");
  if (!success) {
    exit(1);
  }
}

proglang::Parser::~Parser() { }

std::string proglang::Parser::getLlvmIr() {
  proglang::LlvmIrExtendedParserData extData;
  std::string d;
  std::string d_ = root.buildLlvmIr({ }, extData) + "\n";
  for (std::string a : extData.str_consts) {
    d += a + "\n";
  }
  d += "\n" + d_;
  for (std::string a : extData.ext_defs) {
    d += a + "\n";
  }
  d += "\n";
  for (std::string a : extData.attrs) {
    d += a + "\n";
  }
  return d;
}
