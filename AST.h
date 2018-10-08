#ifndef __AST_H__
#define __AST_H__

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "Lexer.h"

using namespace llvm;

	//IR 部分
	static LLVMContext TheContext;
	static IRBuilder<> Builder(TheContext);
	static Module* TheModule;
	static std::map<std::string, Value *> NamedValues;

	//表达式抽象语法树基类
	class ExprAST {
	public:
		virtual ~ExprAST() = default;
		virtual Value *codegen() = 0;
	};

	std::unique_ptr<ExprAST> LogError(const char *Str);

	//report errors found during code generation
	Value *LogErrorV(const char *Str) {
		LogError(Str);
		return nullptr;
	}

	//数字抽象语法树
	class NumberExprAST : public ExprAST {
		int Val;

	public:
		NumberExprAST(int Val) : Val(Val) {}

		Value * codegen() {
			return ConstantFP::get(TheContext, APFloat((double)Val));
		}
	};

	//变量抽象语法树
	class VariableExprAST : public ExprAST {
		std::string Name;

	public:
		VariableExprAST(const std::string &Name) : Name(Name) {}

		Value * codegen() {
			// Look this variable up in the function.
			Value *V = NamedValues[Name];
			if (!V)
				return LogErrorV("Unknown variable name");
			return V;
		}
	};

	//'+','-','*','/'二元运算表达式抽象语法树
	class BinaryExprAST : public ExprAST {
		char Op;
		std::unique_ptr<ExprAST> LHS, RHS;

	public:
		BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
			std::unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}


		Value * codegen() {
			Value *L = LHS->codegen();
			Value *R = RHS->codegen();
			if (!L || !R)
				return nullptr;

			switch (Op) {
			case '+':
				return Builder.CreateFAdd(L, R, "addtmp");
			case '-':
				return Builder.CreateFSub(L, R, "subtmp");
			case '*':
				return Builder.CreateFMul(L, R, "multmp");
			case '/':
				L = Builder.CreateFDiv(L, R, "divtmp");
				// Convert bool 0/1 to double 0.0 or 1.0
				return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
			default:
				return LogErrorV("invalid binary operator");
			}
		}
	};

	//函数原型抽象语法树--函数名和参数列表
	class PrototypeAST {
		std::string Name;
		std::vector<std::string> Args;

	public:
		PrototypeAST(const std::string &Name, std::vector<std::string> Args)
			: Name(Name), Args(std::move(Args)) {}

		const std::string &getName() const { return Name; }

		/*
		* 待重构：VSL 中不需要 PrototypeAST
		* VSL 中的函数声明必须要带函数体，所以没有必要将 Prototype 分离出来
		*/
		Function * codegen() {
			//不允许函数重定义
			Function *TheFunction = TheModule->getFunction(Name);
			if (TheFunction)
				return (Function*)LogErrorV("Function cannot be redefined.");

			// 函数形参类型为 int
			std::vector<Type*> Integers(Args.size(),
				Type::getInt32Ty(TheContext));
			FunctionType *FT =
				FunctionType::get(Type::getInt32Ty(TheContext), Integers, false);

			// 注册该函数
			Function *F =
				Function::Create(FT, Function::InternalLinkage, Name, TheModule);

			// 为函数参数命名
			unsigned Idx = 0;
			for (auto &Arg : F->args())
				Arg.setName(Args[Idx++]);

			return F;
		}
	};

	//函数抽象语法树
	class FunctionAST {
		std::unique_ptr<PrototypeAST> Proto;
		std::unique_ptr<ExprAST> Body;

	public:
		FunctionAST(std::unique_ptr<PrototypeAST> Proto,
			std::unique_ptr<ExprAST> Body)
			: Proto(std::move(Proto)), Body(std::move(Body)) {}

		/*
		* 待重构：如上
		*/
		Function * codegen() {
			// 调用 Proto 的 codegen()
			Function *TheFunction = Proto->codegen();

			if (!TheFunction)
				return nullptr;

			// Create a new basic block to start insertion into.
			BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
			Builder.SetInsertPoint(BB);

			// Record the function arguments in the NamedValues map.
			NamedValues.clear();
			for (auto &Arg : TheFunction->args())
				NamedValues[Arg.getName()] = &Arg;

			if (Value *RetVal = Body->codegen()) {
				// Finish off the function.
				Builder.CreateRet(RetVal);

				// Validate the generated code, checking for consistency.
				verifyFunction(*TheFunction);

				return TheFunction;
			}

			// Error reading body, remove function.
			TheFunction->eraseFromParent();
			return nullptr;
		}
	};

	//函数调用抽象语法树
	class CallExprAST : public ExprAST {
		std::string Callee;
		std::vector<std::unique_ptr<ExprAST>> Args;
	public:
		CallExprAST(const std::string &Callee,
			std::vector<std::unique_ptr<ExprAST>> Args)
			: Callee(Callee), Args(std::move(Args)) {}

		Value * codegen() {
			// Look up the name in the global module table.
			Function *CalleeF = TheModule->getFunction(Callee);
			if (!CalleeF)
				return LogErrorV("Unknown function referenced");

			// If argument mismatch error.
			if (CalleeF->arg_size() != Args.size())
				return LogErrorV("Incorrect # arguments passed");

			std::vector<Value *> ArgsV;
			for (unsigned i = 0, e = Args.size(); i != e; ++i) {
				ArgsV.push_back(Args[i]->codegen());
				if (!ArgsV.back())
					return nullptr;
			}

			return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
		}
	};

	//程序的抽象语法树
	class ProgramAST {
		std::vector<std::unique_ptr<FunctionAST>> funcs;

	public:
		ProgramAST(std::vector<std::unique_ptr<FunctionAST>> funcs)
			:funcs(std::move(funcs)) {}
	};

	/*statement部分 -- lh*/
	//statement 基类
	class StatAST {
	public:
		virtual ~StatAST() = default;
	};

	//not mine
	class DecAST : public StatAST {};

	//块语句
	class BlockStatAST : public StatAST {
		std::vector<std::unique_ptr<DecAST>> DecList;
		std::vector<std::unique_ptr<StatAST>> StatList;
	};

	//Text
	class TextAST {

	};

	class PrinStatAST : public StatAST {

	};

#endif