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
#pragma ones

#include "LabeledClause.hpp"
#include "Clause.hpp"
#include "Match.hpp"
#include "MemoKey.hpp"
#include "MemoTable.hpp"
#include "MetaGrammar.hpp"
#include "TreeUtils.hpp"

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>

using namespace std;

/** A node in the Abstract Syntax Tree (AST). */
class ASTNode {
public:
    string label;
    Clause* nodeType;
    int startPos;
    int len;
    string input;
    vector<ASTNode*> children;

    ASTNode(string label, Clause* nodeType, int startPos, int len, string input) {
        this->label = label;
        this->nodeType = nodeType;
        this->startPos = startPos;
        this->len = len;
        this->input = input;
    }

    /** Recursively create an AST from a parse tree. */
    ASTNode(string label, Match* match, string input) {
        ASTNode(label, match->memoKey->clause, match->memoKey->startPos, match->len, input);
        addNodesWithASTNodeLabelsRecursive(this, match, input);
    }

    /** Recursively convert a match node to an AST node. */
    void addNodesWithASTNodeLabelsRecursive(ASTNode* parentASTNode, Match* parentMatch, string input) {
        // Recurse to descendants
        vector<pair<string,Match*>> subClauseMatchesToUse = parentMatch->getSubClauseMatches();
        for (int subClauseMatchIdx = 0; subClauseMatchIdx < subClauseMatchesToUse.size(); subClauseMatchIdx++) {
            pair<string, Match*> subClauseMatchEnt = subClauseMatchesToUse[subClauseMatchIdx];
            string subClauseASTNodeLabel = subClauseMatchEnt.first;
            Match* subClauseMatch = subClauseMatchEnt.second;
            if (subClauseASTNodeLabel.empty()) {
                // Create an AST node for any labeled sub-clauses
                ASTNode astPer(subClauseASTNodeLabel, subClauseMatch, input);
                parentASTNode->children.push_back(&astPer);
            }
            else {
                // Do not add an AST node for parse tree nodes that are not labeled; however, still need
                // to recurse to their subclause matches
                addNodesWithASTNodeLabelsRecursive(parentASTNode, subClauseMatch, input);
            }
        }
    }

    ASTNode* getOnlyChild() {
        if (children.size() != 1) {
            cout << ("Expected one child, got " + children.size());
            abort();
        }
        return children[0];
    }

    ASTNode* getFirstChild() {
        if (children.size() < 1) {
            cout << "No first child";
            abort();
        }
        return children[0];
    }

    ASTNode* getSecondChild() {
        if (children.size() < 2) {
            cout << "No second child";
            abort();
        }
        return children[1];
    }

    ASTNode* getThirdChild() {
        if (children.size() < 3) {
            cout << "No third child";
            abort();
        }
        return children[2];
    }

    ASTNode* getChild(int i) {
        return children[i];
    }

    string getText() {
        return input.substr(startPos, len);
    }


    string toString() {
        string buf;
        TreeUtils::renderTreeView(this, input, "", true, buf);
        return buf;
    }
};
