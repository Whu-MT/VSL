#include <cstdio>
#include <cstdlib>
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#ifdef linux    //linux
#include "KaleidoscopeJIT.h"
#else  //windows
#include "../include/KaleidoscopeJIT.h"
#endif

static int emitIR;
static char *inputFileName;
#include "Lexer.h"
#include "AST.h"
#include "Parser.h"

void usage()
{
    printf("usage: VSL inputFile [-r] [-h]\n");
    printf("-r emit IR code to IRcode.ll file\n");
    printf("-h show help information\n");

    exit(EXIT_FAILURE);
}

void getArgs(int argc, char *argv[])
{
    for(int i = 1; i<argc; i++)
    {
        if(argv[i][0] == '-' && argv[i][1] == 'r')
        {
            emitIR = 1;
        }else if (argv[i][0] == '-' && argv[i][1] == 'h')
        {
            usage();
        }else
        {
            inputFileName = argv[i];
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2)
        usage();
    getArgs(argc, argv);
    inputFile = fopen(inputFileName, "r");
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

    TheJIT = llvm::make_unique<KaleidoscopeJIT>();
    InitializeModuleAndPassManager();

    // Run the main "interpreter loop" now.
    getNextToken();
    MainLoop();

    return 0;
}
