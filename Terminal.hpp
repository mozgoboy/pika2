#pragma once
#include "Clause.hpp"

class Terminal : public Clause
{
public:
	TypesOfClauses TypeOfClause = TypesOfClauses::Terminal;
	Terminal() : Clause(vector<Clause*> {} )
	{}
	//���� ������� ���� �������, ����� ������������ ������������ ����������� �� �����, ��� ��� ������ �� �� ����� ����������� � ��������
};