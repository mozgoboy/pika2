#pragma once
#include "LabeledClause.hpp"
#include "ASTNodeLabel.hpp"
#include "Seq.hpp"
#include "Nothing.hpp"
#include "MetaGrammar.hpp"
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

enum class TypesOfClauses {Clause , First , FollowedBy , NotFollowedBy , OneOrMore , Seq , CharSeq , CharSet , Nothing , Start , Terminal , ASTNodeLabel , RuleRef  };



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

	Clause(vector<Clause*> subClauses)
	{
		if (subClauses.size() > 0 &&  subClauses[0]->TypeOfClause == TypesOfClauses::Nothing) {
			// Nothing can't be the first subclause, since we don't trigger upwards expansion of the DP wavefront
			// by seeding the memo table by matching Nothing at every input position, to keep the memo table small
			cout <<   "Nothing cannot be the first subclause of any clause";
			abort();
		}
		this->labeledSubClauses.clear();
		for (int i = 0; i < subClauses.size(); i++)
		{
			Clause* subClause = subClauses[i];
			string astNodeLabel;
			if (subClause->TypeOfClause == TypesOfClauses::ASTNodeLabel )
			{
				// Transfer ASTNodeLabel.astNodeLabel to LabeledClause.astNodeLabel field
				astNodeLabel = ((ASTNodeLabel*)subClause)->astNodeLabel;
				// skip over ASTNodeLabel node when adding subClause to subClauses array
				subClause = subClause->labeledSubClauses[0]->clause;
			}
			LabeledClause X(subClause, astNodeLabel);
			this->labeledSubClauses.push_back(&X);
		}
	}

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


	string getRuleNames() {
		string buf;
		for (auto rule : rules)
		{
			buf += rule->ruleName + ", ";
		}
		buf.pop_back();
		buf.pop_back();
		return buf;
	}
	/** Тупа вывод */

	string toString() {
		cout << "toString() needs to be overridden in subclasses";
		abort();
	}

	/** Get the clause as a string, with rule names prepended if the clause is the toplevel clause of a rule. */
	string toStringWithRuleNames() {
		string buf;
		if (toStringWithRuleNameCached.empty()) {
			if (not rules.empty()) {
				// Add rule names
				buf.append(getRuleNames());
				buf.append(" <- ");
				// Add any AST node labels
				bool addedASTNodeLabels = false;
				for (int i = 0; i < rules.size(); i++) {
					Rule* rule = rules[i];
					if (not rule->labeledClause->astNodeLabel.empty()) {
						buf.append(rule->labeledClause->astNodeLabel + ":");
						addedASTNodeLabels = true;
					}
				}
				bool addParens = addedASTNodeLabels && MetaGrammar::needToAddParensAroundASTNodeLabel(this);
				if (addParens) {
					buf.append("(");
				}
				buf.append(toString());
				if (addParens) {
					buf.append(")");
				}
				toStringWithRuleNameCached = buf;
			}
			else {
				toStringWithRuleNameCached = buf;
			}
		}
		return toStringWithRuleNameCached;
	}
};

/** Перегоняем каким то образом класс в строку для вывода. */