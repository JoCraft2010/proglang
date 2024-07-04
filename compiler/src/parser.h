#pragma once

#include "macros.h"
#include "tokenizer.h"

#include <algorithm>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <string>
#include <vector>

namespace proglang {

  enum class ParserElementType {
    ROOT,
    FUNCTION,
    EXTERNAL_FUNCTION,
    FUNCTION_CALL,
    VARIABLE_DECLARATION,
    VARIABLE_DECLARATION_EMPTY,
    VARIABLE_ASSIGNMENT,
    VARIABLE_ASSIGNMENT_TO_FUNCTION_CALL,
    VARIABLE_ASSIGNMENT_TO_INDEX,
    VARIABLE_INDEX_ASSIGNMENT,
    VARIABLE_INCREMENT,
    VARIABLE_DECREMENT,
    VARIABLE_MODULO,
    IF,
    IF_SINGLE_BOOL,
    WHILE,
    WHILE_SINGLE_BOOL,
    RETURN
  };

  struct LlvmIrExtendedParserData {
    std::vector<std::string> strs;
    std::vector<std::string> str_consts;
    std::vector<std::string> ext_defs = { "declare void @llvm.memcpy.p0.p0.i32(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i32, i1 immarg) #0" };
    std::unordered_map<std::string, std::string> func_signs;
    std::string universal_attrs = " \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"tune-cpu\"=\"generic\"";
    std::vector<std::string> attrs = { "attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }" };
    std::vector<std::string> registered_attrs = { "nocallback nofree nounwind willreturn memory(argmem: readwrite)" };
    std::vector<std::string> global_vars;
    std::string var_decls;
    std::string var_inits;
    std::unordered_map<std::string, std::string> arr_sizes;
    std::unordered_map<std::string, std::string> arr_size_nums;
    std::unordered_map<std::string, std::string> var_types;
    std::unordered_map<std::string, std::string> var_sizes;
    int snum_count = 0;
    int label_count = 0;

    std::string registerAttrs(std::string _attrs) {
      std::vector<std::string>::iterator loc = std::find(registered_attrs.begin(), registered_attrs.end(), _attrs);
      if (loc == registered_attrs.end()) {
        registered_attrs.push_back(_attrs);
        attrs.push_back("attributes #" + std::to_string(attrs.size()) + " = { " + _attrs + universal_attrs + " }");
        return "#" + std::to_string(attrs.size() - 1);
      } else {
        return "#" + std::to_string(std::distance(registered_attrs.begin(), loc));
      }
    }
    std::string registerStringConst(std::string str, int& len) {
      std::string name = "@.str." + std::to_string(str_consts.size());
      strs.push_back(str);
      len = str.size() + 1;
      std::string nstr;
      bool mod = false;
      for (char c : str) {
        // TODO 24
        if (mod) {
          if (c == 'n') {
            nstr += "\\0a";
          } else if (c == '0') {
            nstr += "\\00";
          } else if (c == '\\') {
            nstr += "\\\\";
          } else {
            nstr += "<?>";
            len += 2;
          }
          mod = false;
        } else if (c == '\\') {
          mod = true;
          len--;
        } else {
          nstr.push_back(c);
        }
      }
      str_consts.push_back(name + " = private unnamed_addr constant [" + std::to_string(len) + " x i8] c\"" + nstr + "\\00\", align 1");
      arr_sizes[std::to_string(str_consts.size() - 1)] = "[" + std::to_string(len) + " x i8]";
      arr_size_nums[std::to_string(str_consts.size() - 1)] = std::to_string(len);
      return name;
    }
    std::string registerEmptyGlobalStringVar(std::string name, std::string size) {
      str_consts.push_back("@.str." + name + " = dso_local global [" + size + " x i8] zeroinitializer, align 16");
      return "@.str." + name;
    }
    std::string storeVariableValue(std::string name, std::string from, std::string val) {
      if (val.substr(0, 5) == "@.str") {
        return "call void @llvm.memcpy.p0.p0.i32(ptr align 1 " + obtainVariableAccessPointer(name, from) + ", ptr align 1 " + val + ", i32 " + arr_size_nums[name] + ", i1 false)\n";
      } else {
        return "store " + var_types[name] + " " + val + ", ptr " + obtainVariableAccessPointer(name, from) + ", align " + var_sizes[name] + "\n";
      }
    }
    std::string obtainVariableAccess(std::string name, std::string from, std::string& accessPoint) {
      if (name.find_first_not_of("0123456789") == std::string::npos) {
        accessPoint = name;
        return "";
      }
      if (var_types[name] == "ptr") {
        accessPoint = "%.v." + name;
        return "";
      }
      if (std::find(global_vars.begin(), global_vars.end(), ".str." + from + "." + name) != global_vars.end()) {
        accessPoint = "@.str." + from + "." + name;
        return "";
      }
      snum_count++;
      accessPoint = "%" + std::to_string(snum_count);
      return "%" + std::to_string(snum_count) + " = load " + var_types[name] + ", ptr %.v." + name + ", align " + var_sizes[name] + "\n";
    }
    std::string obtainVariableAccessPointer(std::string name, std::string from) {
      if (std::find(global_vars.begin(), global_vars.end(), ".str." + from + "." + name) != global_vars.end()) {
        return "@.str." + from + "." + name;
      }
      return "%.v." + name;
    }
    std::string compare(std::string name0, std::string name1, std::string from, std::string op) {
      std::string d, ap0, ap1;
      d += obtainVariableAccess(name0, from, ap0);
      d += obtainVariableAccess(name1, from, ap1);
      snum_count++;
      d += "%" + std::to_string(snum_count) + " = icmp " + op + " " + var_types[name0] + " " + ap0 + ", " + ap1 + "\n";
      return d;
    }
  };

  class ParserElement {
  public:
    ParserElement(ParserElement*, ParserElementType, std::vector<std::string>);
    ~ParserElement();

    void printTree(std::string);
    std::string buildLlvmIr(std::vector<std::string>, LlvmIrExtendedParserData&);

    ParserElement& appendChild(ParserElement);
    ParserElementType getType();
    ParserElement* getParent();
  private:
    ParserElement* parent;
    ParserElementType type;
    std::vector<std::string> data;
    std::vector<ParserElement> children;
  };

  class Parser {
  public:
    Parser(std::vector<Token>);
    ~Parser();

    std::string getLlvmIr();
  private:
    ParserElement root{ nullptr, ParserElementType::ROOT, { } };
  };

}
