#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include "Rule.hpp"
#include "Terminal.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"
#include "StringUtils.hpp"
#include "GrammarUtils.hpp"
#include "RuleRef.hpp"
#include "Clause.hpp"
#include "Nothing.hpp"
#include <ranges>
class Grammar
{
private:

public:
	vector<Rule*> allRules;
	unordered_map<string, Rule*> ruleNameWithPrecedenceToRule;
	vector<Clause*> allClauses;
	bool DEBUG = false;

	Grammar(vector<Rule*> rules)
	{
		Rule* topLevelRule = rules[0];
		unordered_map<string, vector<Rule*>> ruleNameToRules;
		for (auto rule : rules)
		{
			if (rule->ruleName.empty()) {
				cout << "All rules must be named";
				abort();
			}
			if (rule->labeledClause->clause->TypeOfClause == TypesOfClauses::RuleRef
				&& ((RuleRef*)rule->labeledClause->clause)->refdRuleName == rule->ruleName) {
				// Make sure rule doesn't refer only to itself
				cout <<	"Rule cannot refer to only itself: " + rule->ruleName + "[" + to_string(rule->precedence) + "]";
			}
			vector<Rule*> rulesWithName = ruleNameToRules[rule->ruleName];
			if (rulesWithName.empty()) {
				ruleNameToRules[rule->ruleName] = rulesWithName;
			}
			
			// тут есть моментик что rulesWithName достаём из анордеред мапа если существует, а если нет то создаём 

			rulesWithName.push_back(rule);
			GrammarUtils::checkNoRefCycles(rule->labeledClause->clause, rule->ruleName, new unordered_set<Clause>());
		}

		allRules = rules;
		vector<Clause> lowestPrecedenceClauses;
		unordered_map<string,string> ruleNameToLowestPrecedenceLevelRuleName;
		
		for (auto ent : ruleNameToRules) 
		{
			vector<Rule*> rulesWithName = ent.second;
			if (rulesWithName.size() > 1) 
			{
				string ruleName = ent.first;
				GrammarUtils::handlePrecedence(ruleName, rulesWithName, lowestPrecedenceClauses,
					ruleNameToLowestPrecedenceLevelRuleName);
			}
		}
		unordered_map<string, Rule*> ruleNameWithPrecedenceToRule;
		// Закоменченное выше вызывает метод entrySet, который вовращает что-то типа набора пар ключ значение, я если честно не понимаю как оно должно прописываться.
		for (auto rule : allRules) {
			// The handlePrecedence call above added the precedence to the rule name as a suffix
			if (!  (ruleNameWithPrecedenceToRule.insert_or_assign(rule->ruleName, rule).second) ) {
				// Should not happen
				cout << "Duplicate rule name " + rule->ruleName;
			}
		}

		for (auto rule : allRules) 
		{
			rule->labeledClause->clause->registerRule(rule);
		}

		unordered_map<string, Clause*> toStringToClause;
		for (auto rule : allRules) 
		{
			rule->labeledClause->clause = GrammarUtils::intern(rule->labeledClause->clause, toStringToClause);
		}

		set<Clause*> clausesVisitedResolveRuleRefs;
		for (auto rule : allRules) 
		{
			GrammarUtils::resolveRuleRefs(rule->labeledClause, ruleNameWithPrecedenceToRule, ruleNameToLowestPrecedenceLevelRuleName, clausesVisitedResolveRuleRefs);
		}

		allClauses = GrammarUtils::findClauseTopoSortOrder(topLevelRule, allRules, lowestPrecedenceClauses);

		for (Clause* clause : allClauses) 
		{
			clause->determineWhetherCanMatchZeroChars();
		}

		for (Clause* clause : allClauses) 
		{
			clause->addAsSeedParentClause();
		}
	}

	MemoTable* parse(string input) {
		priority_queue< Clause*, vector<Clause*>, cmp > priorityQueue;

		MemoTable memoTable(this, input);

		vector<Clause*> terminals;
		for (auto clause : allClauses) {
			if ((clause->TypeOfClause == TypesOfClauses::CharSeq ||
				clause->TypeOfClause == TypesOfClauses::CharSet ||
				clause->TypeOfClause == TypesOfClauses::Start) && clause->TypeOfClause != TypesOfClauses::Nothing)
				terminals.push_back(clause);
		}

		// То что сверху пока тоже не очень ясно

		// Main parsing loop
		for (int startPos = input.length() - 1; startPos >= 0; --startPos) {
			if (DEBUG) {
				cout << "=============== POSITION: " << startPos << " CHARACTER:["
					+ StringUtils::escapeQuotedChar(input[startPos]) << "] ===============";
			}
			for (Clause* terminal : terminals)
			{
				priorityQueue.push(terminal);
			}
			//priorityQueue.addAll(terminals);
			while (!priorityQueue.empty()) {
				// Remove a clause from the priority queue (ordered from terminals to toplevel clauses)
				auto clause = priorityQueue.top();
				priorityQueue.pop();
				MemoKey memoKey(clause, startPos);
				Match* match = clause->match(&memoTable, &memoKey, input);
				memoTable.addMatch(memoKey, match, priorityQueue);
			}
		}
		return &memoTable;
	}


	Rule* getRule(string ruleNameWithPrecedence) 
	{
		Rule* rule = ruleNameWithPrecedenceToRule[ruleNameWithPrecedence];
		return rule;
	}

	vector<Match*> getNonOverlappingMatches(string ruleName, MemoTable* memoTable) 
	{
		return memoTable->getNonOverlappingMatches(getRule(ruleName)->labeledClause->clause);
	}
	
	/*
	NavigableMap<Integer, Match> getNavigableMatches(String ruleName, MemoTable memoTable) 
	{
		return memoTable.getNavigableMatches(getRule(ruleName).labeledClause.clause);
	}
	*/

	//Пока не ясно с аналогом на плюсах у навигаблмэп.
};


struct cmp
{
		bool operator()(const Clause*& c1, const Clause*& c2)    // pass by a const reference
		{
			return c1->clauseIdx > c2->clauseIdx;
		}
};