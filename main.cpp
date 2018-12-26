#include <cstdio>
#include <cstdlib>
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#ifdef linux    //linux
#include "KaleidoscopeJIT.h"
#else  //windows
#include "../include/KaleidoscopeJIT.h"
#endif

static int emitIR = 0;
static int emitObj = 0; //如果添加了-obj选项，则只生成.o文件，不运行代码
static char *inputFileName;
#include "Lexer.h"
#include "AST.h"
#include "Parser.h"

void usage()
{
    printf("usage: VSL inputFile [-r] [-h]\n");
    printf("-r emit IR code to IRcode.ll file\n");
    printf("-h show help information\n");
    printf("-obj emit obj file of the input file\n");

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
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'o')
        {
            if (strlen(argv[i]) > 3 && 
                (argv[i][2] == 'b' && argv[i][3] == 'j'))
                emitObj = 1;
            else
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

    if(emitObj)
    {
        // Initialize the target registry etc.
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        auto TargetTriple = sys::getDefaultTargetTriple();
        TheModule->setTargetTriple(TargetTriple);

        std::string Error;
        auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

        // Print an error and exit if we couldn't find the requested target.
        // This generally occurs if we've forgotten to initialise the
        // TargetRegistry or we have a bogus target triple.
        if (!Target)
        {
            errs() << Error;
            return 1;
        }

        auto CPU = "generic";
        auto Features = "";

        TargetOptions opt;
        auto RM = Optional<Reloc::Model>();
        auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

        TheModule->setDataLayout(TheTargetMachine->createDataLayout());

        auto Filename = "output.o";
        std::error_code EC;
        raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

        if (EC)
        {
            errs() << "Could not open file: " << EC.message();
            return 1;
        }

        legacy::PassManager pass;
        auto FileType = TargetMachine::CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType))
        {
            errs() << "TheTargetMachine can't emit a file of this type";
            return 1;
        }

        pass.run(*TheModule);
        dest.flush();

        outs() << "Wrote " << Filename << "\n";
    }

    return 0;
}
