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
		if (memoKey->startPos <= (input.length() - str.length()) && (input.substr(memoKey->startPos, str.length()) == str)) // ���� �������� ��� �������� ignoreCase, �.�. ���� �� true �� ��������� ��������� ���� ������ ������� true, � ����� ������������ ��������� ���������.
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