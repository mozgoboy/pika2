#pragma once
#include "Terminal.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"
#include "StringUtils.hpp"


class CharSeq : public Terminal
{
	
public:
	string str;
	bool ignoreCase;
	TypesOfClauses TypeOfClause = TypesOfClauses::CharSeq;
	CharSeq(string str, bool ignoreCase) : Terminal()
	{
		this->str = str;
		this->ignoreCase = ignoreCase;
	}

	void determineWhetherCanMatchZeroChars() 
	{}

	Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) 
	{
		if (memoKey->startPos <= (input.length() - str.length()) && (input.substr(memoKey->startPos, str.length()) == str)) // надо спросить про параметр ignoreCase, т.к. если он true то последнее сравнение тоже должно вернуть true, а иначе возвращается результат сравнения.
		{
			Match*  mast = new Match(memoKey, /* len = */ str.length());
			return mast;
		}
		return nullptr;
	}

	string toString() {
		if (toStringCached.empty()) {
			toStringCached = '"' + StringUtils::escapeString(str) + '"';
		}
		return toStringCached;
	}
};