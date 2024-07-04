#include "macros.h"
#include "parser.h"
#include "tokenizer.h"

#include <fstream>
#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
  if (argc < 3) {
    ERROR("Error: insufficient arguments.\nCorrect usage: compiler <infile> <outfile>" << std::endl);
    return 1;
  }

  std::string ifile = argv[1];
  std::string ofile = argv[2];

  std::ifstream ifilestream(ifile);
  proglang::Tokenizer tokenizer(ifilestream);
  ifilestream.close();

  proglang::Parser parser(tokenizer.getCombinedTokens());

  DEBUG("-----" << std::endl);
  std::string llvm_ir = parser.getLlvmIr();
  DEBUG("-----" << std::endl);
  DEBUG(llvm_ir << std::endl);

  std::ofstream ofilestream(ofile);
  ofilestream << llvm_ir;
  ofilestream.close();

  DEBUG("-----" << std::endl);

  return 0;
}
