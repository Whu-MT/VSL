#include "Lexer.h"
#include "AST.h"
#include "Parser.h"


int main() {
  BinopPrecedence['+'] = 10;
  BinopPrecedence['-'] = 10;
  BinopPrecedence['*'] = 40;
  BinopPrecedence['/'] = 40;

  // Prime the first token.
  fprintf(stderr, "ready> ");
  getNextToken();

  // Make the module, which holds all the code.
  TheModule = new Module("my cool jit", TheContext);

  // Run the main "interpreter loop" now.
  MainLoop();

  // Print out all of the generated code.
  TheModule->print(errs(), nullptr);

  return 0;
}
