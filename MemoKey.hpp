#pragma once

#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <cstdio>
#include "Clause.hpp"

class MemoKey
{
private:
public:
	Clause* clause;
	int startPos;

	MemoKey()
	{}

	MemoKey(Clause* clause, int startPos) 
	{
		this->clause = clause;
		this->startPos = startPos;
	}

	int hashCode() 
	{
		std::hash<Clause*> hut;
		return hut(clause) ^ startPos; // тут побитовая операция исключающего или

	}

	bool equals(MemoKey* mem) 
	{
		if ((mem->clause == this->clause) && (mem->startPos == this->startPos))
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
		return clause->toStringWithRuleNames() + " : " + to_string(startPos);
	}

	string toString() 
	{
		return clause->toString() + " : " + to_string(startPos);
	}
};