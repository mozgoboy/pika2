#pragma once

#include "Match.hpp"
#include "Clause.hpp"
#include "NotFollowedBy.hpp"
#include "Grammar.hpp"
#include "IntervalUnion.hpp"
#include <queue>

class MemoTable
{
private:
	unordered_map<MemoKey*, Match*> memoTable;

public:
    Grammar* grammar;
    string input;
    int counter = 0;

    int numMatchObjectsCreated;
    int numMatchObjectsMemoized;
    /* У них вместо инта используется AtomicInteger, типа он лучше подходит для параллельных вычислений, нужно ли нам это, пока не ясно. */

    MemoTable(Grammar* grammar, string input) 
    {
        this->grammar = grammar;
        this->input = input;
    }

    Match* lookUpBestMatch(MemoKey* memoKey)
    {
        auto bestMatch = memoTable[memoKey];

        if (bestMatch != nullptr)
        {
            return bestMatch;
        }
        else if (memoKey->clause->TypeOfClause == TypesOfClauses::NotFollowedBy)
        {
            return memoKey->clause->match(this, memoKey, input);
        }
        else if (memoKey->clause->canMatchZeroChars) 
        {
            return new Match(memoKey);
        }
        return nullptr;
    }
    void addMatch(MemoKey* memoKey, Match* newMatch, priority_queue< Clause*, vector<Clause*>, cmp >  priorityQueue)
    {
        counter++;
        auto matchUpdated = false;
        if (newMatch != nullptr)
        {
            numMatchObjectsCreated++;
            auto oldMatch = memoTable[memoKey];
            if ((oldMatch == nullptr) || newMatch->isBetterThan(oldMatch))
            {
                memoTable[memoKey] = newMatch;
                matchUpdated = true;

                numMatchObjectsMemoized++;
                
                if (grammar->DEBUG) {
                    cout << "Setting new best match: " + newMatch->toStringWithRuleNames();
                }
                //С выводом ошибок пока не совсем ясно, идейно в функции просто перезаписываем значение если оно лучше подходит.
                
            }
        }
        for (int i = 0, ii = memoKey->clause->seedParentClauses.size(); i < ii; i++) 
        {
            auto seedParentClause = memoKey->clause->seedParentClauses[i];
            // If there was a valid match, or if there was no match but the parent clause can match
            // zero characters, schedule the parent clause for matching. (This is part of the strategy
            // for minimizing the number of zero-length matches that are memoized.)
            if (matchUpdated || seedParentClause->canMatchZeroChars) 
            {
                priorityQueue.push(seedParentClause);
                
                if (grammar->DEBUG) {
                    cout <<
                        "    Following seed parent clause: " + seedParentClause->toStringWithRuleNames();
                }
                
            }
            
            if (grammar->DEBUG) {
                cout << (newMatch != nullptr) ? "Matched: " + newMatch->toStringWithRuleNames()
                    : "Failed to match: " + memoKey->toStringWithRuleNames();
            }
            
        }
    }


    /** Get all {@link Match} entries, indexed by clause then start position. */
    map<Clause*, map<int, Match*>> getAllNavigableMatches() {
        map<Clause*, map<int, Match*>> clauseMap;
     
        for (auto item : memoTable)
        {
            auto match = item.second;
            auto startPosMap = clauseMap[match->memoKey->clause];
            if (startPosMap.empty()) {
                clauseMap[match->memoKey->clause] =  startPosMap;
            }
                startPosMap[match->memoKey->startPos] = match;
                
        }
        return clauseMap;
    }

    /** Get all nonoverlapping {@link Match} entries, indexed by clause then start position. */
    map<Clause*, map<int, Match*>> getAllNonOverlappingMatches() {
        map<Clause*, map<int, Match*>> nonOverlappingClauseMap;
        for (auto ent : getAllNavigableMatches()) {
            auto clause = ent.first;
            auto startPosMap = ent.second;
            int prevEndPos = 0;
            map<int,Match*> nonOverlappingStartPosMap;
            for (auto startPosEnt : startPosMap) {
                auto startPos = startPosEnt.first;
                if (startPos >= prevEndPos) {
                    auto match = startPosEnt.second;
                    nonOverlappingStartPosMap[startPos] = match;
                    auto endPos = startPos + match->len;
                    prevEndPos = endPos;
                }
            }
            nonOverlappingClauseMap[clause] = nonOverlappingStartPosMap;
        }
        return nonOverlappingClauseMap;
    }

    /** Get all {@link Match} entries for the given clause, indexed by start position. */
    map<int, Match*> getNavigableMatches(Clause* clause) {
        map<int,Match*> treeMap;

        for (auto ent : memoTable)
        {
            if (ent.first->clause == clause) {
                treeMap[ent.first->startPos] = ent.second;
            }
        }
        return treeMap;
    }

    /** Get the {@link Match} entries for all matches of this clause. */
    vector<Match*> getAllMatches(Clause* clause) {
        vector<Match*> matches;
        for (auto ent : getNavigableMatches(clause))
        {
            matches.push_back(ent.second);
        }
        return matches;
    }

    /**
     * Get the {@link Match} entries for all nonoverlapping matches of this clause, obtained by greedily matching
     * from the beginning of the string, then looking for the next match after the end of the current match.
     */
    vector<Match*> getNonOverlappingMatches(Clause* clause) {
        auto matches = getAllMatches(clause);
        vector<Match*>* nonoverlappingMatches = new vector<Match*>();
        for (int i = 0; i < matches.size(); i++) {
            auto match = matches[i];
            auto startPos = match->memoKey->startPos;
            auto endPos = startPos + match->len;
            nonoverlappingMatches->push_back(match);
            while (i < matches.size() - 1 && matches[i + 1]->memoKey->startPos < endPos) {
                i++;
            }
        }
        return *nonoverlappingMatches;
    }

    /**
     * Get any syntax errors in the parse, as a map from start position to a tuple, (end position, span of input
     * string between start position and end position).
     */
    map<int, pair<int, string>> getSyntaxErrors(vector<string> syntaxCoverageRuleNames) {
        // Find the range of characters spanned by matches for each of the coverageRuleNames
        auto parsedRanges = new IntervalUnion();
        for (auto coverageRuleName : syntaxCoverageRuleNames) {
            auto rule = grammar->getRule(coverageRuleName);
            for (auto match : getNonOverlappingMatches(rule->labeledClause->clause)) {
                parsedRanges->addRange(match->memoKey->startPos, match->memoKey->startPos + match->len);
            }
        }
        // Find the inverse of the parsed ranges -- these are the syntax errors
        auto unparsedRanges = parsedRanges->invert(0, input.length())->getNonOverlappingRanges();
        // Extract the input string span for each unparsed range
        map<int, pair<int, string>> syntaxErrorSpans;

        for (auto ent : unparsedRanges)
        {
            
               pair<int, string> x(ent.second, input.substr(ent.first, ent.second - ent.first));
               syntaxErrorSpans[ent.first] = x;
        }
        return syntaxErrorSpans;
    }
};