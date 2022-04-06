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

/** The First (ordered choice) PEG operator. */
class First : public Clause {
    TypesOfClauses TypeOfClause = TypesOfClauses::First;
    First(vector<Clause*> subClauses) : Clause(subClauses) {
        if (subClauses.size() < 2) {
            cout << "First expects 2 or more subclauses";
            abort();
        }
    }

    void determineWhetherCanMatchZeroChars() {
        for (int subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
            // Up to one subclause of a First clause can match zero characters, and if present,
            // the subclause that can match zero characters must be the last subclause
            if (labeledSubClauses[subClauseIdx]->clause->canMatchZeroChars) {
                canMatchZeroChars = true;
                if (subClauseIdx < labeledSubClauses.size() - 1) {
                    cout <<
                        ("Subclause " + to_string(subClauseIdx)  + " of First"
                            + " can match zero characters, which means subsequent subclauses will never be "
                            + "matched: " + this->toString());
                    abort();
                }
                break;
            }
        }
    }


    Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) {
        for (int subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
            Clause* subClause = labeledSubClauses[subClauseIdx]->clause;
            MemoKey subClauseMemoKey(subClause, memoKey->startPos);
            Match* subClauseMatch = memoTable->lookUpBestMatch(&subClauseMemoKey);
            if (subClauseMatch != nullptr) {
                // Return a match for the first matching subclause
                Match mast(memoKey, /* len = */ subClauseMatch->len,
                /* firstMatchingSubclauseIdx = */ subClauseIdx, vector<Match*>{ subClauseMatch });
                return &mast;
            }
        }
        return nullptr;
    }

    string toString() {
        if (toStringCached.empty()) {
            string buf;
            for (int i = 0; i < labeledSubClauses.size(); i++) {
                if (i > 0) {
                    buf.append(" / ");
                }
                buf.append(labeledSubClauses[i]->toStringWithASTNodeLabel(this));
            }
            toStringCached = buf;
        }
        return toStringCached;
    }
};
