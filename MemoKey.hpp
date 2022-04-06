#pragma once

#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <cstdio>
#include "Clause.hpp";

class MemoKey
{
private:
public:
	Clause* clause;
	int startPos;

	MemoKey()
	{}

	MemoKey(Clause clause, int startPos) 
	{
		this->clause = clause;
		this->startPos = startPos;
	}

	int hashCode() 
	{
		return clause.hashCode() ^ startPos; // тут побитовая операция исключающего или

	}

	bool equals(Memokey mem) 
	{
		if ((mem.clause == this->clause) && (mem.startPos == this->startPos))
		{
			return true;
		}
		else 
		{
			return false;
		}
	}

	string toStringWithRuleNames() 
	{
		return clause.toStringWithRuleNames() + " : " + startPos;
	}

	string toString() 
	{
		return clause.toString() + " : " + startPos;
	}
};