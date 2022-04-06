#pragma once
#include "Terminal.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"

class Start : public Terminal
{
public:
	TypesOfClauses TypeOfClause = TypesOfClauses::Start;
	string START_STR = "^";

	Start() : Terminal()
	{}

	void determineWhetherCanMatchZeroChars() 
	{
		canMatchZeroChars = true;
	}

	Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) 
	{
		if (memoKey->startPos == 0) 
		{
			Match mast(memoKey);
			return &mast;
		}
		return nullptr;
	}
	//Функция выше пока не ясно как работает

	string toString() 
	{
		if (toStringCached == "")
		{
			toStringCached = START_STR;
		}
		return toStringCached;
	}
	//Функция выше просто вывод, мб потом уберём
};