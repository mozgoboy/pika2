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
#include "MetaGrammar.hpp"

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>

using namespace std;

/** A container for grouping a subclause together with its AST node label. */
class LabeledClause {
public:
    Clause* clause;
    string astNodeLabel;

    LabeledClause(Clause* clause, string astNodeLabel) {
        this->clause = clause;
        this->astNodeLabel = astNodeLabel;
    }

    /** Call {@link #toString()}, prepending any AST node label. */
    string toStringWithASTNodeLabel(Clause* parentClause) {
        bool addParens;
        addParens = ((parentClause != nullptr) && (MetaGrammar::needToAddParensAroundSubClause(parentClause, clause)));
        if (astNodeLabel.empty() && !addParens) {
            // Fast path
            return clause->toString();
        }
        string buf;
        if (not astNodeLabel.empty()) {
            buf.append(astNodeLabel);
            buf.append(":");
            addParens = addParens || MetaGrammar::needToAddParensAroundASTNodeLabel(clause);
        }
        if (addParens) {
            buf.append("(");
        }
        buf.append(clause->toString());
        if (addParens) {
            buf.append(")");
        }
        return buf;
    }

    string toString() {
        return toStringWithASTNodeLabel(nullptr);
    }
};
