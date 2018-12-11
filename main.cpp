#include <cstdio>
#include <cstdlib>
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

void usage()
{
    printf("usage:./VSL file\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if(argc != 2)
        usage();
    inputFile = fopen(argv[1], "r");
    if(!inputFile){
        printf("%s open error!\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    BinopPrecedence['+'] = 10;
    BinopPrecedence['-'] = 10;
    BinopPrecedence['*'] = 40;
    BinopPrecedence['/'] = 40;

    // Make the module, which holds all the code.
    //TheModule = new Module("my cool jit", TheContext);

    TheJIT = llvm::make_unique<KaleidoscopeJIT>();
    InitializeModuleAndPassManager();

    // Run the main "interpreter loop" now.
    getNextToken();
    MainLoop();

    // Print out all of the generated code.
    //TheModule->print(errs(), nullptr);

    return 0;
}
