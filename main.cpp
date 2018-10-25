#include "Lexer.h"
#include "AST.h"
#include "Parser.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#ifdef linux    //linux
#include "KaleidoscopeJIT.h"
#else  //windows
#include "../include/KaleidoscopeJIT.h"
#endif


int main() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  BinopPrecedence['+'] = 10;
  BinopPrecedence['-'] = 10;
  BinopPrecedence['*'] = 40;
  BinopPrecedence['/'] = 40;

  // Prime the first token.
  fprintf(stderr, "ready> ");
  getNextToken();

  // Make the module, which holds all the code.
  //TheModule = new Module("my cool jit", TheContext);

  TheJIT = llvm::make_unique<KaleidoscopeJIT>();
  InitializeModuleAndPassManager();

  // Run the main "interpreter loop" now.
  MainLoop();

  // Print out all of the generated code.
  TheModule->print(errs(), nullptr);

  return 0;
}
