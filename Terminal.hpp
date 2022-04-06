#pragma once
#include "Clause.hpp"

class Terminal : public Clause
{
public:
	TypesOfClauses TypeOfClause = TypesOfClauses::Terminal;
	Terminal() : Clause(vector<Clause*> {} )
	{}
	//выше немного бред написан, нужно переработать родительский конструктор от клоза, так как почему то он имеет конструктор с вектором
};