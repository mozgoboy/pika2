#pragma once

#include "ASTNodeLabel.hpp"
#include "Seq.hpp"
#include "Nothing.hpp"

#include "Rule.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"
#include <vector>
#include <string>
#include <stdlib.h>
#include <set>
#include <algorithm>




using namespace std;

enum class TypesOfClauses {Clause , First , FollowedBy , NotFollowedBy , OneOrMore , 
	Seq , CharSeq , CharSet , Nothing , Start , Terminal , ASTNodeLabel , RuleRef  };

class LabeledClause;

class Clause
{
private:
public:
	vector<LabeledClause*> labeledSubClauses;
	vector<Rule*> rules;
	vector<Clause*> seedParentClauses;
	bool canMatchZeroChars; //По идее удалим, пока не понятно зачем нужно
	int clauseIdx;
	string toStringCached;
	string toStringWithRuleNameCached;
	TypesOfClauses TypeOfClause = TypesOfClauses::Clause;

	Clause()
	{}

	Clause(vector<Clause*> subClauses);

	void registerRule(Rule* rule)
	{
		rules.push_back(rule);
	}

	void unregisterRule(Rule* rule)
	{
		rules.erase(remove(rules.begin(), rules.end(), rule), rules.end());
	}

	void addAsSeedParentClause()
	{
		set<Clause*> added;
		for (auto labeledSubClause : labeledSubClauses)
		{
			// Don't duplicate seed parent clauses in the subclause
			if (added.insert(labeledSubClause->clause).second) {
				labeledSubClause->clause->seedParentClauses.push_back(this);
			}
		}
	}

	virtual void determineWhetherCanMatchZeroChars() = 0;

	virtual Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) = 0;
	


	/** Функции сверху - как макеты, описаны персонально для каждего класса */
	/** Get the names of rules that this clause is the root clause of. */


	string getRuleNames();

	string toString() {
		cout << "toString() needs to be overridden in subclasses";
		abort();
	}

	/** Get the clause as a string, with rule names prepended if the clause is the toplevel clause of a rule. */
	string toStringWithRuleNames();
};

/** Перегоняем каким то образом класс в строку для вывода. */