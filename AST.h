#ifndef __AST_H__
#define __AST_H__
namespace{
	//表达式抽象语法树基类
	class ExprAST {
	public:
  		virtual ~ExprAST() = default;
	};

	//数字抽象语法树
	class NumberExprAST : public ExprAST {
  		int Val;

	public:
  		NumberExprAST(int Val) : Val(Val) {}
	};

	//变量抽象语法树
	class VariableExprAST : public ExprAST {
	  std::string Name;

	public:
	  VariableExprAST(const std::string &Name) : Name(Name) {}
	};

	//'+','-','*','/'二元运算表达式抽象语法树
	class BinaryExprAST : public ExprAST {
	  char Op;
	  std::unique_ptr<ExprAST> LHS, RHS;

	public:
	  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
	                std::unique_ptr<ExprAST> RHS)
	      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
	};

	//函数原型抽象语法树--函数名和参数列表
	class PrototypeAST {
	  std::string Name;
	  std::vector<std::string> Args;

	public:
	  PrototypeAST(const std::string &Name, std::vector<std::string> Args)
	      : Name(Name), Args(std::move(Args)) {}

	  const std::string &getName() const { return Name; }
	};

	//函数抽象语法树
	class FunctionAST {
	  std::unique_ptr<PrototypeAST> Proto;
	  std::unique_ptr<ExprAST> Body;

	public:
	  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
	              std::unique_ptr<ExprAST> Body)
	      : Proto(std::move(Proto)), Body(std::move(Body)) {}
	};

	//函数调用抽象语法树
	class CallExprAST : public ExprAST{
		std::string Callee;
		std::vector<std::unique_ptr<ExprAST>> Args;
	public:
		CallExprAST(const std::string &Callee,
			std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
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
}

#endif