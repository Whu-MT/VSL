#ifndef __PARSER_H__
#define __PARSER_H__
#include "Lexer.h"
#include "AST.h"

static int CurTok;
static std::map<char, int> BinopPrecedence;
static int getNextToken() { return CurTok = gettok(); }

static std::unique_ptr<ExprAST> ParseExpression();
std::unique_ptr<ExprAST> LogError(const char *Str);
static std::unique_ptr<ExprAST> ParseNumberExpr();
static std::unique_ptr<ExprAST> ParseParenExpr();
std::unique_ptr<ExprAST> LogError(const char *Str);
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);

//解析如下格式的表达式：
// identifer || identifier(expression list)
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
	std::string IdName = IdentifierStr;

	getNextToken();

	//解析成变量表达式
	if (CurTok != '(') 
		return llvm::make_unique<VariableExprAST>(IdName);

	// 解析成函数调用表达式
	getNextToken();
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (true) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;

			if (CurTok != ',')
				return LogError("Expected ')' or ',' in argument list");
			getNextToken();
		}
	}

	getNextToken();

	return llvm::make_unique<CallExprAST>(IdName, std::move(Args));
}


//解析成 标识符表达式、整数表达式、括号表达式中的一种
static std::unique_ptr<ExprAST> ParsePrimary() {
	switch (CurTok) {
	default:
		return LogError("unknown token when expecting an expression");
	case VARIABLE:
		return ParseIdentifierExpr();
	case INTEGER:
		return ParseNumberExpr();
	case '(':
		return ParseParenExpr();
	}
}

//GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

//解析二元表达式
//参数 ： 
//ExprPrec 左部运算符优先级
//LHS 左部操作数
// 递归得到可以结合的右部，循环得到一个整体二元表达式
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
	std::unique_ptr<ExprAST> LHS) {
	
	while (true) {
		int TokPrec = GetTokPrecedence();

		// 当右部没有运算符或右部运算符优先级小于左部运算符优先级时 退出循环和递归
		if (TokPrec < ExprPrec)
			return LHS;

		// 保存左部运算符
		int BinOp = CurTok;
		getNextToken();

		// 得到右部表达式
		auto RHS = ParsePrimary();
		if (!RHS)
			return nullptr;

		// 如果该右部表达式不与该左部表达式结合 那么递归得到右部表达式
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}

		// 将左右部结合成新的左部
		LHS = llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
			std::move(RHS));
	}
}

// 解析得到表达式
static std::unique_ptr<ExprAST> ParseExpression() {
	auto LHS = ParsePrimary();
	if (!LHS)
		return nullptr;

	return ParseBinOpRHS(0, std::move(LHS));
}

// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
	auto Result = llvm::make_unique<NumberExprAST>(NumberVal);
	//略过数字获取下一个输入
	getNextToken(); 
	return std::move(Result);
}

//prototype ::= VARIABLE '(' parameter_list ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
	if (CurTok != VARIABLE)
		return LogErrorP("Expected function name in prototype");

	std::string FnName = IdentifierStr;
	getNextToken();

	if (CurTok != '(')
		return LogErrorP("Expected '(' in prototype");

	std::vector<std::string> ArgNames;
	getNextToken();
	while (CurTok == VARIABLE)
	{
		ArgNames.push_back(IdentifierStr);
		getNextToken();
		if (CurTok == ',')
			getNextToken();
	}
	if (CurTok != ')')
		return LogErrorP("Expected ')' in prototype");

	// success.
	getNextToken(); // eat ')'.

	return llvm::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

//function ::= FUNC prototype '{' statement '}'
static std::unique_ptr<FunctionAST> ParseFunc()
{
	getNextToken(); // eat FUNC.
	auto Proto = ParsePrototype();
	if (!Proto)
		return nullptr;
	if (CurTok != '{')
	{
		LogErrorP("Expected '{' in function");
		return nullptr;
	}
	getNextToken();

	/*auto E = ParseExpression();
	if (!E)
		return nullptr;*/
	if (CurTok != '}')
	{
		LogErrorP("Expected '}' in function");
		return nullptr;
	}
	getNextToken();

	return llvm::make_unique<FunctionAST>(std::move(Proto), nullptr);
}

//解析括号中的表达式
static std::unique_ptr<ExprAST> ParseParenExpr() {
	// 过滤'('
	getNextToken(); 
	auto V = ParseExpression();
	if (!V)
		return nullptr;

	if (CurTok != ')')
		return LogError("expected ')'");
	// 过滤')'
	getNextToken(); 
	return V;
}

//错误信息打印
std::unique_ptr<ExprAST> LogError(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
	LogError(Str);
	return nullptr;
}

// Top-Level parsing
static void HandleFuncDefinition() {
  if (ParseFunc()) {
    fprintf(stderr, "Parsed a function definition.\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
                                                 std::vector<std::string>());
    return llvm::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

//program ::= definition | expression
static void MainLoop() {
	while (true) {
		fprintf(stderr, "ready> ");
	switch (CurTok) {
		case ';': // ignore top-level semicolons.
			getNextToken();
			break;
		case TOK_EOF:
			return;
		case FUNC:
		 	HandleFuncDefinition();
			break;
		default:
			HandleTopLevelExpression();
			break;
	}
	}
}

#endif