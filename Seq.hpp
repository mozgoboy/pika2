//
// This file is part of the pika parser reference implementation:
//
//     https://github.com/lukehutch/pikaparser
//
// The pika parsing algorithm is described in the following paper: 
//
//     Pika parsing: reformulating packrat parsing as a dynamic programming algorithm solves the left recursion
//     and error recovery problems. Luke A. D. Hutchison, May 2020.
//     https://arxiv.org/abs/2005.06444
//
// This software is provided under the MIT license:
//
// Copyright 2020 Luke A. D. Hutchison
//  
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
#pragma once

#include "Clause.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unordered_set>

/** The Seq (sequence) PEG operator. */
class Seq : public Clause {
    TypesOfClauses TypeOfClause = TypesOfClauses::Seq;
    Seq(vector<Clause*> subClauses) : Clause(subClauses) {
        if (subClauses.size() < 2) {
            cout << "Seq expects 2 or more subclauses";
            abort();
        }
    }

    /** Если меньше двух то не последовательность, у нас последовательность, мы переиграли */

    void determineWhetherCanMatchZeroChars() {
        // For Seq, all subclauses must be able to match zero characters for the whole clause to
        // be able to match zero characters
        bool canMatchZeroChars = true;
        for (int subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
            if (!labeledSubClauses[subClauseIdx]->clause->canMatchZeroChars) {
                canMatchZeroChars = false;
                break;
            }
        }
    }

    /** Снова мэтчзирочарс которого у нас пока нет, т.к. не ясно для чего, ставит тру если у всех детей тру */
    // Надо дописать
    void addAsSeedParentClause() {
        // All sub-clauses up to and including the first clause that matches one or more characters
        // needs to seed its parent clause if there is a subclause match
        set<Clause*> added;
        for (int subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
            Clause* subClause = labeledSubClauses[subClauseIdx]->clause;
            // Don't duplicate seed parent clauses in the subclause
            if (added.insert(subClause).second) {
                subClause->seedParentClauses.push_back(this);
            }
            if (!subClause->canMatchZeroChars) {
                // Don't need to any subsequent subclauses to seed this parent clause
                break;
            }
        }
    }

    /** как будто просто снова дописываем родителя всем детям у которых он пустой */


    Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) {
        vector<Match*> subClauseMatches;
        int currStartPos = memoKey->startPos;
        for (int subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
            Clause* subClause = labeledSubClauses[subClauseIdx]->clause;
            MemoKey subClauseMemoKey(subClause, currStartPos);
            Match* subClauseMatch = memoTable->lookUpBestMatch(subClauseMemoKey);
            if (subClauseMatch == nullptr) {
                // Fail after first subclause fails to match
                return nullptr;
            }
            if (subClauseMatches.empty()) {
                subClauseMatches.resize(labeledSubClauses.size(), nullptr);
            }
            subClauseMatches[subClauseIdx] = subClauseMatch;
            currStartPos += subClauseMatch->len;
        }
        // All subclauses matched, so the Seq clause matches
        Match mast(memoKey, /* len = */ currStartPos - memoKey->startPos, subClauseMatches);
        return &mast;
    }
    /** Пока не уверены, наверное сравнение с имеющимися элементами в мемо чтобы не разбирать. Но это не точно. */

    string toString() {
        if (toStringCached.empty()) {
            string buf;
            for (int i = 0; i < labeledSubClauses.size(); i++) {
                if (i > 0) {
                    buf.append(" ");
                }
                buf.append(labeledSubClauses[i]->toStringWithASTNodeLabel(this));
            }
            toStringCached = buf;
        }
        return toStringCached;
    }
};

/** Снова штука просто для вывода */