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

#include "ASTNode.hpp"
#include "MetaGrammar.hpp"
#include "Match.hpp"
#include <algorithm>
using namespace std;

/** Tree utilities. */
class TreeUtils {
public:
    /** Render the AST rooted at an {@link ASTNode} into a StringBuffer. */
    static void renderTreeView(ASTNode* astNode, string input, string indentStr, bool isLastChild, string buf) {
        int inpLen = 80;
        string inp = input.substr(astNode->startPos,
            min(int(input.length()) - astNode->startPos, min(astNode->len, inpLen)));
        if (inp.length() == inpLen) {
            inp += "...";
        }
        inp = StringUtils::escapeString(inp);

        // Uncomment for double-spaced rows
        // buf.append(indentStr + "│\n");

        buf.append(indentStr + (isLastChild ? "└─" : "├─") + astNode->label + " : " + to_string(astNode->startPos) + "+"
            + to_string(astNode->len) + " : \"" + inp + "\"\n");
        if (! astNode->children.empty()) {
            for (int i = 0; i < astNode->children.size(); i++) {
                renderTreeView(astNode->children[i], input, indentStr + (isLastChild ? "  " : "│ "),
                    i == astNode->children.size() - 1, buf);
            }
        }
    }

    /** Render a parse tree rooted at a {@link Match} node into a StringBuffer. */
    static void renderTreeView(Match* match, string astNodeLabel, string input, string indentStr,
        bool isLastChild,string buf) {
        int inpLen = 80;
        string inp = input.substr(match->memoKey->startPos,
            min(int(input.length()) - match->memoKey->startPos,  min(match->len, inpLen)));
        if (inp.length() == inpLen) {
            inp += "...";
        }
        inp = StringUtils::escapeString(inp);

        // Uncomment for double-spaced rows
        // buf.append(indentStr + "│\n");

        bool astNodeLabelNeedsParens = MetaGrammar::needToAddParensAroundASTNodeLabel(match->memoKey->clause);
        buf.append(indentStr);
        buf.append(isLastChild ? "└─" : "├─");
        string ruleNames = match->memoKey->clause->getRuleNames();
        if (!ruleNames.empty()) {
            buf.append(ruleNames + " <- ");
        }
        if (! astNodeLabel.empty()) {
            buf.append(astNodeLabel);
            buf.append(":");
            if (astNodeLabelNeedsParens) {
                buf.append("(");
            }
        }
        string toStr = match->memoKey->clause->toString();
        buf.append(toStr);
        if ( !astNodeLabel.empty() && astNodeLabelNeedsParens) {
            buf.append(")");
        }
        buf.append(" : ");
        buf.append(to_string(match->memoKey->startPos));
        buf.append("+");
        buf.append(to_string(match->len));
        buf.append(" : \"");
        buf.append(inp);
        buf.append("\"\n");

        // Recurse to descendants
        vector<pair<string,Match*>> subClauseMatchesToUse = match->getSubClauseMatches();
        for (int subClauseMatchIdx = 0; subClauseMatchIdx < subClauseMatchesToUse.size(); subClauseMatchIdx++) {
            pair<string,Match*> subClauseMatchEnt = subClauseMatchesToUse[subClauseMatchIdx];
            string subClauseASTNodeLabel = subClauseMatchEnt.first;
            Match* subClauseMatch = subClauseMatchEnt.second;
            renderTreeView(subClauseMatch, subClauseASTNodeLabel, input, indentStr + (isLastChild ? "  " : "│ "),
                subClauseMatchIdx == subClauseMatchesToUse.size() - 1, buf);
        }
    }

    /** Print the parse tree rooted at a {@link Match} node to stdout. */
    static void printTreeView(Match* match, string input) {
        string buf;
        renderTreeView(match, "", input, "", true, buf);
        cout << buf;
    }
};
