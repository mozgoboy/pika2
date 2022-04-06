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

class OneOrMore : public Clause {
public:
    TypesOfClauses TypeOfClause = TypesOfClauses::OneOrMore;
    OneOrMore(Clause* subClause) : Clause(vector<Clause*> { subClause }) {
    }

    void determineWhetherCanMatchZeroChars() {
        if (labeledSubClauses[0]->clause->canMatchZeroChars) {
            canMatchZeroChars = true;
        }
    }

    Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) {
        Clause* labeledSubClause = labeledSubClauses[0]->clause;
        MemoKey subClauseMemoKey(labeledSubClause, memoKey->startPos);
        Match* subClauseMatch = memoTable->lookUpBestMatch(&subClauseMemoKey);
        if (subClauseMatch == nullptr) {
            // Zero matches at memoKey.startPos
            return nullptr;
        }

        // Perform right-recursive match of the same OneOrMore clause, so that the memo table doesn't
        // fill up with O(M^2) entries in the number of subclause matches M.
        // If there are two or more matches, tailMatch will be non-null.
        MemoKey tailMatchMemoKey(this, memoKey->startPos + subClauseMatch->len);
        Match* tailMatch = memoTable->lookUpBestMatch(&tailMatchMemoKey);
        Match* mast1 = new Match(memoKey, /* len = */ subClauseMatch->len, //
            vector<Match*>{ subClauseMatch });
        Match* mast2 = new Match(memoKey, /* len = */ subClauseMatch->len + tailMatch->len, //
            vector<Match*>{ subClauseMatch, tailMatch });
        // Return a new (right-recursive) match
        return tailMatch == nullptr //
                // There is only one match => match has only one subclause
            ? mast1
            // There are two or more matches => match has two subclauses (head, tail)
            : mast2;
    }
    string toString() {
        if (toStringCached.empty()) {
            toStringCached = labeledSubClauses[0]->toStringWithASTNodeLabel(this) + "+";
        }
        return toStringCached;
    }
};
