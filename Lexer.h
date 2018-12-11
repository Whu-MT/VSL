#ifndef __LEXER_H__
#define __LEXER_H__
#include "llvm/ADT/STLExtras.h"
#include <cctype>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>

enum Token
{
	TOK_EOF = -1,
	ERROR = -2,
	VARIABLE = -3,
	INTEGER = -4,
	TEXT = -5,//
	ASSIGN_SYMBOL = -6,
	FUNC = -7,
	PRINT = -8,
	RETURN = -9,
	CONTINUE = -10,
	IF = -11,
	THEN = -12,
	ELSE = -13,
	FI = -14,
	WHILE = -15,
	DO = -16,
	DONE = -17,
	VAR = -18,
};

static std::string IdentifierStr;
static int NumberVal;
static FILE *inputFile;

/*
*返回输入单词类型
*/
static int gettok()
{
	static int LastChar = ' ';

	//过滤空格
	while(isspace(LastChar))
		LastChar = fgetc(inputFile);

	//解析标识符:{lc_letter}({lc_letter}|{digit})*
	if(isalpha(LastChar))
	{
		IdentifierStr = "";
		IdentifierStr += LastChar;
		while (isalnum((LastChar = fgetc(inputFile))))
			IdentifierStr += LastChar;

		if(IdentifierStr == "FUNC")
			return FUNC;
		if(IdentifierStr == "PRINT")
			return PRINT;
		if(IdentifierStr == "RETURN")
			return RETURN;
		if(IdentifierStr == "CONTINUE")
			return CONTINUE;
		if(IdentifierStr == "IF")
			return IF;
		if(IdentifierStr == "THEN")
			return THEN;
		if(IdentifierStr == "ELSE")
			return ELSE;
		if(IdentifierStr == "FI")
			return FI;
		if(IdentifierStr == "WHILE")
			return WHILE;
		if(IdentifierStr == "DO")
			return DO;
		if(IdentifierStr == "DONE")
			return DONE;
		if(IdentifierStr == "VAR")
			return VAR;

		return VARIABLE;	//非预留关键字，而是标识符
	}

	//解析整数:{digit}+
	if(isdigit(LastChar))
	{
		std::string NumStr;
		do{
			NumStr += LastChar;
			LastChar = fgetc(inputFile);
		}while(isdigit(LastChar));

		NumberVal = atoi(NumStr.c_str());

		return INTEGER;
	}

	//解析注释:"//".*
	if(LastChar == '/')
		if ((LastChar = fgetc(inputFile)) == '/')
		{
            do
				LastChar = fgetc(inputFile);
			while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

            //若未到达结尾，返回下一个输入类型
            if(LastChar != EOF)
                return gettok();
        }else{
            return '/';
        }

	//赋值符号
	if (LastChar == ':' && (LastChar = fgetc(inputFile)) == '=')
	{
		LastChar = fgetc(inputFile); //获取下一个字符
		return ASSIGN_SYMBOL;
	}

	//TEXT
	if(LastChar == '\"')
	{
        IdentifierStr = "";
		LastChar = fgetc(inputFile);
		do
		{
            if(LastChar == '\\')
            {
				LastChar = fgetc(inputFile);
				if(LastChar == 'n')
                    LastChar = '\n';
                else if(LastChar == 't')
                    LastChar = '\t';
                else if(LastChar == 'r')
                    LastChar = '\r';
                else
                    IdentifierStr += '\\';
            }
			IdentifierStr += LastChar;
			LastChar = fgetc(inputFile);
		}while(LastChar != '\"');
		LastChar = fgetc(inputFile);
		return TEXT;
	}

	if(LastChar == '\\')
    {
        int tmp;
		LastChar = fgetc(inputFile);
		if(LastChar == 'n')
            tmp = '\n';
        else if(LastChar == 't')
            tmp = '\t';
        else if(LastChar == 'r')
            tmp = '\r';
        else
            return '\\';
		LastChar = fgetc(inputFile);

		return tmp;
    }

	//文档结束标志
	if(LastChar == EOF)
		return TOK_EOF;

	//以上情况均不满足，直接返回当前字符
	int tmpChar = LastChar;
	LastChar = fgetc(inputFile);

	return tmpChar;
}

#endif
