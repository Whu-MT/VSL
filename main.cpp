#include "Lexer.h"
#include "AST.h"
#include "Parser.h"


int main() {
  BinopPrecedence['+'] = 10;
  BinopPrecedence['-'] = 10;
  BinopPrecedence['*'] = 40;
  BinopPrecedence['/'] = 40;

  fprintf(stderr, "ready> ");
  getNextToken();

  MainLoop();

  return 0;
}
