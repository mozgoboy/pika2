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
#include "MemoKey.hpp";
#include "MemoTable.hpp";

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>

/**
 * Placeholder clause for referring to another rule by name. These clauses are replaced with direct references to
 * the clauses they reference by name before parsing begins, in order to optimize parsing.
 */
class RuleRef : public Clause {
public:
    string refdRuleName;

    RuleRef(string refdRuleName) : Clause(vector<Clause*> {}) {
        this->refdRuleName = refdRuleName;
    }

    void determineWhetherCanMatchZeroChars() {
    }

    Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) {
        cout << "RuleRef node should not be in final grammar";
        abort();
    }


    string toString() {
        if (toStringCached.empty()) {
            toStringCached = refdRuleName;
        }
        return toStringCached;
    }
};
