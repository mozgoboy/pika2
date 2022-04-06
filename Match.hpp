#pragma once

#include "Clause.hpp";
#include "First.hpp";
#include "OneOrMore.hpp";

class Match
{
private:
public:
	MemoKey* memoKey;
	int len;
	int firstMatchingSubClauseIdx;
	vector<Match*> subClauseMatches;
	static vector<Match*> NO_SUBCLAUSE_MATCHES;

	Match()
	{}

	Match(MemoKey* memoKey, int len, int firstMatchingSubClauseIdx, vector<Match*> subClauseMatches)
	{
		this->memoKey = memoKey;
		this->len = len;
		this->firstMatchingSubClauseIdx = firstMatchingSubClauseIdx;
		this->subClauseMatches = subClauseMatches;
	}

	Match(MemoKey* memoKey, int len, vector<Match*> subClauseMatches) 
	{
		Match(memoKey, len, 0, subClauseMatches);
	}

	Match(MemoKey* memoKey, int len) 
	{
		Match(memoKey, len, 0, NO_SUBCLAUSE_MATCHES);
	}

	Match(MemoKey* memoKey) 
	{
		Match(memoKey, 0);
	}

	vector<pair<string, Match*>> getSubClauseMatches() 
	{
		if (subClauseMatches.size() == 0) {
			// This is a terminals, or an empty placeholder match returned by MemoTable.lookUpBestMatch
			vector<pair<string, Match*>> x;
			return x;
			/*  ак € пон€л у них просто возвращаетс€ пустой лист. */
		}
		if (memoKey->clause->TypeOfClause == TypesOfClauses::OneOrMore) 
		{
			// Flatten right-recursive structure of OneOrMore parse subtree
			vector<pair<string, Match*>> subClauseMatchesToUse;
			for (auto curr = this; curr->subClauseMatches.size() > 0;)
			{
				// Add head of right-recursive list to arraylist, paired with its AST node label, if present
				subClauseMatchesToUse.push_back(make_pair(curr->memoKey->clause->labeledSubClauses[0]->astNodeLabel, curr->subClauseMatches[0]));
				if (curr->subClauseMatches.size() == 1) 
				{
					// The last element of the right-recursive list will have a single element, i.e. (head),
					// rather than two elements, i.e. (head, tail) -- see the OneOrMore.match method
					break;
				}
				// Move to tail of list
				curr = curr->subClauseMatches[1];
			}
			return subClauseMatchesToUse;
		}
		else if (memoKey->clause->TypeOfClause == TypesOfClauses::First) 
		{
			// For First, pair the match with the AST node label from the subclause of idx firstMatchingSubclauseIdx
			vector<pair<string, Match*>> x;
			x.push_back(make_pair(memoKey->clause->labeledSubClauses[firstMatchingSubClauseIdx]->astNodeLabel, subClauseMatches[0]));
			return x;
		}
		else 
		{
			// For other clause types, return labeled subclause matches
			auto numSubClauses = memoKey->clause->labeledSubClauses.size();
			vector<pair<string, Match*>> subClauseMatchesToUse(numSubClauses);
			for (int i = 0; i < numSubClauses; i++) {
				subClauseMatchesToUse.push_back(
					make_pair(memoKey->clause->labeledSubClauses[i]->astNodeLabel, subClauseMatches[i]));
			}
			return subClauseMatchesToUse;
		}
	}



	bool operator==(const Match anothermatch)
	{
		/* ѕотом пропишем здесь сравнение указателей*/
	}

	bool isBetterThan(Match* oldMatch) {
		if (oldMatch == this) 
		{
			return false;
		}
		return this->len > oldMatch->len;
	}

	string toStringWithRuleNames() 
	{
		string buf;
		buf += memoKey->toStringWithRuleNames() + "+" + to_string(len);
		return buf;
	}

	string toString() 
	{
		string buf;
		buf += memoKey->toString() + "+" + to_string(len);
		return buf;
	}
};