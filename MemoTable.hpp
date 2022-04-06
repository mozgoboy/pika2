#pragma once

#include "Clause.hpp";
#include "NotFollowedBy.hpp";
#include "Grammar.hpp";
#include "IntervalUnion.hpp";
#include <queue>;

class MemoTable
{
private:
	unordered_map<MemoKey, Match> memoTable;

public:
    Grammar grammar;
    string input;
    int counter = 0;

    int numMatchObjectsCreated;
    int numMatchObjectsMemoized;
    /* У них вместо инта используется AtomicInteger, типа он лучше подходит для параллельных вычислений, нужно ли нам это, пока не ясно. */

    MemoTable(Grammar grammar, string input) 
    {
        this->grammar = grammar;
        this->input = input;
    }

    Match lookUpBestMatch(MemoKey memoKey)
    {
        auto bestMatch = memoTable[memoKey];

        if (bestMatch != nullptr)
        {
            return bestMatch;
        }
        else if (memoKey.clause->TypeOfClause == TypesofClauses::NotFollowedBy)
        {
            return memoKey.clause.match(*this, memoKey, input);
        }
        else if (memoKey.clause.canMatchZeroChars) 
        {
            return Match(memoKey);
        }
        return nullptr;
    }
    void addMatch(MemoKey memoKey, Match newMatch, priority_queue<Clause> priorityQueue)
    {
        counter++;
        auto matchUpdated = false;
        if (newMatch != nullptr)
        {
            numMatchObjectsCreated++;
            auto oldMatch = memoTable[memoKey];
            if ((oldMatch == nullptr || newMatch.isBetterThan(oldMatch)))
            {
                memoTable[memoKey] = newMatch;
                matchUpdated = true;

                numMatchObjectsMemoized++;
                /*
                if (Grammar.DEBUG) {
                    System.out.println("Setting new best match: " + newMatch.toStringWithRuleNames());
                }
                С выводом ошибок пока не совсем ясно, идейно в функции просто перезаписываем значение если оно лучше подходит.
                */
            }
        }
        for (int i = 0, ii = memoKey.clause.seedParentClauses.size(); i < ii; i++) 
        {
            auto seedParentClause = memoKey.clause.seedParentClauses[i];
            // If there was a valid match, or if there was no match but the parent clause can match
            // zero characters, schedule the parent clause for matching. (This is part of the strategy
            // for minimizing the number of zero-length matches that are memoized.)
            if (matchUpdated || seedParentClause.canMatchZeroChars) 
            {
                priorityQueue.push(seedParentClause);
                /*
                if (Grammar.DEBUG) {
                    System.out.println(
                        "    Following seed parent clause: " + seedParentClause.toStringWithRuleNames());
                }
                */
            }
            /*
            if (Grammar.DEBUG) {
                System.out.println(newMatch != null ? "Matched: " + newMatch.toStringWithRuleNames()
                    : "Failed to match: " + memoKey.toStringWithRuleNames());
            }
            */
        }
    }

    /*
    Здесь и дальше я ничего не понимаю в структурах, вообще ничего не ясно.

    unordered_map<Clause, unordered_map<int, Match>> getAllNavigableMatches()
    {
        auto clauseMap = new HashMap<Clause, NavigableMap<Integer, Match>>();
        memoTable.values().stream().forEach(match -> {
            var startPosMap = clauseMap.get(match.memoKey.clause);
            if (startPosMap == null) {
                startPosMap = new TreeMap<>();
                clauseMap.put(match.memoKey.clause, startPosMap);
            }
            startPosMap.put(match.memoKey.startPos, match);
        });
        return clauseMap;
    }
    */
};