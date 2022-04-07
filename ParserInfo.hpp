#pragma once

#include "ASTNode.hpp";
#include "Clause.hpp";
#include "Seq.hpp";
#include "Terminal.hpp";
#include "Grammar.hpp";
#include "Match.hpp";
#include "MemoKey.hpp";
#include "MemoTable.hpp";

#define max(x,y) (((int)((x)<(y)) * (y)) + ((int)((y)<=(x)) * (x)))

class ParserInfo
{
private:
    int findCycleDepth(Match* match,
        map<int, map<int, map<int, Match*>>> cycleDepthToMatches)
    {
        auto cycleDepth = 0;
        for (auto subClauseMatchEnt : match->getSubClauseMatches())
        {
            auto subClauseMatch = subClauseMatchEnt.second;
            auto subClauseIsInDifferentCycle = //
                match->memoKey->clause->clauseIdx <= subClauseMatch->memoKey->clause->clauseIdx;
            auto subClauseMatchDepth = findCycleDepth(subClauseMatch, cycleDepthToMatches);
            cycleDepth = max(cycleDepth, subClauseIsInDifferentCycle ? subClauseMatchDepth + 1 : subClauseMatchDepth);
        }
        /*
        auto matchesForDepth = cycleDepthToMatches.computeIfAbsent(cycleDepth,
            k -> new TreeMap<>(Collections.reverseOrder()));

        var matchesForClauseIdx = matchesForDepth.computeIfAbsent(match.memoKey.clause.clauseIdx,
            k -> new TreeMap<>());

        matchesForClauseIdx.put(match.memoKey.startPos, match);
        */
        // Выше че то снова сложно через treemap
        return cycleDepth;
    }

public:
    void printClauses(Grammar* grammar)
    {
        for (int i = grammar->allClauses.size() - 1; i >= 0; --i)
        {
            auto clause = grammar->allClauses[i];
            cout << i << " : " << clause->toStringWithRuleNames();
        }
    }

    void printMemoTable(MemoTable* memoTable)
    {
        vector<string> buf;
        int marginWidth = 0;
        for (int i = 0; i < memoTable->grammar->allClauses.size(); i++)
        {
            string s = memoTable->grammar->allClauses.size() - 1 - i + " : ";
            buf[i].append(s);
            Clause* clause = memoTable->grammar->allClauses[memoTable->grammar->allClauses.size() - 1 - i];
            if (clause->TypeOfClause == TypesOfClauses::Terminal)
            {
                buf[i].append("[terminal] ");
            }
            if (clause->canMatchZeroChars)
            {
                buf[i].append("[canMatchZeroChars] ");
            }
            buf[i].append(clause->toStringWithRuleNames());
            marginWidth = max(marginWidth, buf[i].length() + 2);
        }
        int tableWidth = marginWidth + memoTable->input.length() + 1;
        for (int i = 0; i < memoTable->grammar->allClauses.size(); i++)
        {
            while (buf[i].length() < marginWidth) {
                buf[i].append(" ");
            }
            while (buf[i].length() < tableWidth) {
                buf[i].append(" - ");
            }
        }
        auto nonOverlappingMatches = memoTable->getAllNonOverlappingMatches();
        for (auto clauseIdx = memoTable->grammar->allClauses.size() - 1; clauseIdx >= 0; --clauseIdx)
        {
            auto row = memoTable->grammar->allClauses.size() - 1 - clauseIdx;
            auto clause = memoTable->grammar->allClauses[clauseIdx];
            auto matchesForClause = nonOverlappingMatches[clause];
            if (!matchesForClause.empty())
            {
                for (auto matchEnt : matchesForClause)
                {
                    auto match = matchEnt.second;
                    auto matchStartPos = match->memoKey->startPos;
                    auto matchEndPos = matchStartPos + match->len;
                    if (matchStartPos <= memoTable->input.length())
                    {
                        buf[row][marginWidth + matchStartPos] = '#';
                        for (int j = matchStartPos + 1; j < matchEndPos; j++)
                        {
                            if (j <= memoTable->input.length()) {
                                buf[row][marginWidth + j] = '=';
                            }
                        }
                    }
                }
            }
            cout << buf[row];
        }
        for (int j = 0; j < marginWidth; j++) {
            cout << " ";
        }
        for (int i = 0; i < memoTable->input.length(); i++) {
            cout << i % 10;
        }
        cout << endl;

        for (int i = 0; i < marginWidth; i++) {
            cout << " ";
        }
        cout << StringUtils::replaceNonASCII(memoTable->input);
    }

    void printParseTreeInMemoTableForm(MemoTable* memoTable)
    {
        if (memoTable->grammar->allClauses.size() == 0)
        {
            //throw new IllegalArgumentException("Grammar is empty");
            // пробрасываем исключение
        }

        map<int, map<int, map<int, Match*>>> cycleDepthToMatches; // я не понимаю что делает reverseOrder,  по идее у нас не так упорядочено буит.
        auto inputSpanned = new IntervalUnion();

        auto nonOverlappingMatches = memoTable->getAllNonOverlappingMatches();
        auto maxCycleDepth = 0;
        for (auto clauseIdx = memoTable->grammar->allClauses.size() - 1; clauseIdx >= 0; --clauseIdx)
        {
            auto clause = memoTable->grammar->allClauses[clauseIdx];
            auto matchesForClause = nonOverlappingMatches[clause];
            if (!matchesForClause.empty())
            {
                for (auto matchEnt : matchesForClause)
                {
                    auto match = matchEnt.second;
                    auto matchStartPos = match->memoKey->startPos;
                    auto matchEndPos = matchStartPos + match->len;
                    // Only add parse tree to chart if it doesn't overlap with input spanned by a higher-level match
                    if (!inputSpanned->rangeOverlaps(matchStartPos, matchEndPos)) {
                        // Pack matches into the lowest cycle they will fit into
                        auto cycleDepth = findCycleDepth(match, cycleDepthToMatches);
                        maxCycleDepth = max(maxCycleDepth, cycleDepth);
                        // Add the range spanned by this match
                        inputSpanned->addRange(matchStartPos, matchEndPos);
                    }
                }
            }
        }
        vector<map<int, Match*>> matchesForRow;
        vector<Clause*> clauseForRow;
        for (auto matchesForDepth : cycleDepthToMatches)
        {
            for (auto matchesForClauseIdxEnt : matchesForDepth.second)
            {
                clauseForRow.push_back(memoTable->grammar->allClauses[matchesForClauseIdxEnt.first]);
                matchesForRow.push_back(matchesForClauseIdxEnt.second);
            }
        }

        vector<string> rowLabel;
        auto rowLabelMaxWidth = 0;
        for (auto i = 0; i < clauseForRow.size(); i++)
        {
            Clause* clause = clauseForRow[i];
            if (clause->TypeOfClause == TypesOfClauses::Terminal)
            {
                rowLabel[i].append("[terminal] ");
            }
            if (clause->canMatchZeroChars)
            {
                rowLabel[i].append("[canMatchZeroChars] ");
            }
            rowLabel[i].append(clause->toStringWithRuleNames());
            rowLabel[i].append("  ");
            rowLabelMaxWidth = max(rowLabelMaxWidth, rowLabel[i].length());
        }
        for (auto i = 0; i < clauseForRow.size(); i++) {
            auto clause = clauseForRow[i];
            auto clauseIdx = clause->clauseIdx;
            // Right-justify the row label
            string label = rowLabel[i];
            rowLabel[i].resize(0);
            for (int j = 0, jj = rowLabelMaxWidth - label.length(); j < jj; j++) {
                rowLabel[i].append(" ");
            }
            rowLabel[i] += clauseIdx + " : ";
            rowLabel[i].append(label);
        }
        string emptyRowLabel;
        for (int i = 0, ii = rowLabelMaxWidth + 6; i < ii; i++)
        {
            emptyRowLabel.append(" ");
        }
        string edgeMarkers;
        edgeMarkers.append(" ");
        for (int i = 1, ii = memoTable->input.length() * 2; i < ii; i++)
        {
            edgeMarkers.append("\u2591");
        }
        edgeMarkers.append("   ");

        for (auto row = 0; row < clauseForRow.size(); row++)
        {
            auto matches = matchesForRow[row];

            string rowTreeChars;
            rowTreeChars.append(edgeMarkers);
            vector<int> zeroLenMatchIdxs;
            for (auto ent : matches)
            {
                auto match = ent.second;
                auto startIdx = match->memoKey->startPos;
                auto endIdx = startIdx + match->len;

                if (startIdx == endIdx)
                {
                    // Zero-length match
                    zeroLenMatchIdxs.push_back(startIdx);
                }
                else
                {
                    // Match consumes 1 or more characters
                    for (auto i = startIdx; i <= endIdx; i++)
                    {
                        char chrLeft = rowTreeChars[i * 2];
                        rowTreeChars[i * 2] =
                            i == startIdx
                            ? (chrLeft == '│' ? '├' : chrLeft == '┤' ? '┼' : chrLeft == '┐' ? '┬' : '┌')
                            : i == endIdx ? (chrLeft == '│' ? '┤' : '┐') : '─';
                        if (i < endIdx)
                        {
                            rowTreeChars[i * 2 + 1] = '─';
                        }
                    }
                }
            }
            cout << emptyRowLabel;
            cout << rowTreeChars << endl;

            for (auto ent : matches)
            {
                auto match = ent.second;
                auto startIdx = match->memoKey->startPos;
                auto endIdx = startIdx + match->len;
                edgeMarkers[startIdx * 2] = '│';
                edgeMarkers[endIdx * 2] = '│';
                for (int i = startIdx * 2 + 1, ii = endIdx * 2; i < ii; i++)
                {
                    auto c = edgeMarkers[i];
                    if (c == '░' || c == '│')
                    {
                        edgeMarkers[i] = ' ';
                    }
                }
            }
            rowTreeChars.resize(0);
            rowTreeChars.append(edgeMarkers);
            for (auto ent : matches)
            {
                auto match = ent.second;
                auto startIdx = match->memoKey->startPos;
                auto endIdx = startIdx + match->len;
                for (int i = startIdx; i < endIdx; i++)
                {
                    rowTreeChars[i * 2 + 1] = StringUtils::replaceNonASCII(memoTable->input[i]);
                }
            }
            for (auto zeroLenMatchIdx : zeroLenMatchIdxs)
            {
                rowTreeChars[zeroLenMatchIdx * 2] = '▮';
            }
            cout << rowLabel[row];
            cout << rowTreeChars << endl;
        }

        // Print input index digits
        for (int j = 0; j < rowLabelMaxWidth + 6; j++)
        {
            cout << ' ';
        }
        cout << ' ';
        for (int i = 0; i < memoTable->input.length(); i++)
        {
            cout << i % 10;
            cout << ' ';
        }
        cout << endl;

        // Print input string
        for (int i = 0; i < rowLabelMaxWidth + 6; i++)
        {
            cout << ' ';
        }
        cout << ' ';
        for (int i = 0; i < memoTable->input.length(); i++)
        {
            cout << StringUtils::replaceNonASCII(memoTable->input[i]);
            cout << ' ';
        }
        cout << endl;
    }

    void printSyntaxErrors(map<int, pair<int, string>> syntaxErrors)
    {
        if (!syntaxErrors.empty())
        {
            cout << endl << "SYNTAX ERRORS:" << endl;
            for (auto ent : syntaxErrors)
            {
                auto startPos = ent.first;
                auto endPos = ent.second.first;
                auto syntaxErrStr = ent.second.second;
                // TODO: show line numbers
                string s = " : ";
                cout << startPos + "+" + (endPos - startPos) + s + StringUtils::replaceNonASCII(syntaxErrStr) << endl;
            }
        }
    }

    void printMatches(Clause* clause, MemoTable* memoTable, bool showAllMatches)
    {
        auto matches = memoTable->getAllMatches(clause);
        if (!matches.empty())
        {
            cout << endl << "====================================" << endl << endl << "Matches for "
                + clause->toStringWithRuleNames() + " :" << endl;
            // Get toplevel AST node label(s), if present
            string astNodeLabel = "";
            if (!clause->rules.empty())
            {
                for (auto rule : clause->rules)
                {
                    if (rule->labeledClause->astNodeLabel != "")
                    {
                        if (astNodeLabel != "")
                        {
                            astNodeLabel += ":";
                        }
                        astNodeLabel += rule->labeledClause->astNodeLabel;
                    }
                }
            }
            auto prevEndPos = -1;
            for (int j = 0; j < matches.size(); j++)
            {
                auto match = matches[j];
                // Indent matches that overlap with previous longest match
                auto overlapsPrevMatch = match->memoKey->startPos < prevEndPos;
                if (!overlapsPrevMatch || showAllMatches)
                {
                    auto indent = overlapsPrevMatch ? "    " : "";
                    string buf;
                    TreeUtils::renderTreeView(match, astNodeLabel.empty() ? "" : astNodeLabel, memoTable->input,
                        indent, true, buf);
                    cout << buf;
                }
                int newEndPos = match->memoKey->startPos + match->len;
                if (newEndPos > prevEndPos)
                {
                    prevEndPos = newEndPos;
                }
            }
        }
        else
        {
            cout << endl <<
                "====================================" << endl << endl << "No matches for " + clause->toStringWithRuleNames() << endl;
        }
    }

    void printMatchesAndSubClauseMatches(Clause* clause, MemoTable* memoTable)
    {
        printMatches(clause, memoTable, true);
        for (int i = 0; i < clause->labeledSubClauses.size(); i++)
        {
            printMatches(clause->labeledSubClauses[i]->clause, memoTable, true);
        }
    }

    void printMatchesAndPartialMatches(Seq* seqClause, MemoTable* memoTable)
    {
        auto numSubClauses = seqClause->labeledSubClauses.size();
        for (auto subClause0Match : memoTable->getAllMatches(seqClause->labeledSubClauses[0]->clause))
        {
            vector<Match*> subClauseMatches;
            subClauseMatches.push_back(subClause0Match);
            auto currStartPos = subClause0Match->memoKey->startPos + subClause0Match->len;
            for (auto i = 1; i < numSubClauses; i++)
            {
                MemoKey* x = new MemoKey(seqClause->labeledSubClauses[i]->clause, currStartPos);
                auto subClauseIMatch = memoTable->lookUpBestMatch(x);
                if (subClauseIMatch == nullptr)
                {
                    break;
                }
                subClauseMatches.push_back(subClauseIMatch);
            }
            cout << endl << "====================================" << endl << endl << "Matched "
                + (subClauseMatches.size() == numSubClauses ? "all subclauses"
                    : subClauseMatches.size() + " out of " + numSubClauses + string(" subclauses"))
                + " of clause (" + seqClause->toString() + ") at start pos " + to_string(subClause0Match->memoKey->startPos) << endl << endl;

            for (int i = 0; i < subClauseMatches.size(); i++)
            {
                auto subClauseMatch = subClauseMatches[i];
                string buf;
                TreeUtils::renderTreeView(subClauseMatch, seqClause->labeledSubClauses[i]->astNodeLabel,
                    memoTable->input, "", true, buf);
                cout << buf << endl;
            }
        }
    }

    void printAST(string astNodeLabel, Clause* clause, MemoTable* memoTable)
    {
        auto matches = memoTable->getNonOverlappingMatches(clause);
        for (int i = 0; i < matches.size(); i++)
        {
            auto match = matches[i];
            auto ast = new ASTNode(astNodeLabel, match, memoTable->input);
            cout << ast->toString() << endl;
        }
    }

    void printParseResult(string topLevelRuleName, MemoTable* memoTable,
        vector<string> syntaxCoverageRuleNames, bool showAllMatches)
    {
        cout << endl << "Clauses:" << endl;
        printClauses(memoTable->grammar);

        cout << endl << "Memo Table:" << endl;
        printMemoTable(memoTable);

        // Print memo table
        cout << endl << "Match tree for rule " + topLevelRuleName + ":" << endl;
        printParseTreeInMemoTableForm(memoTable);

        // Print all matches for each clause
        for (auto i = memoTable->grammar->allClauses.size() - 1; i >= 0; --i)
        {
            auto clause = memoTable->grammar->allClauses[i];
            printMatches(clause, memoTable, showAllMatches);
        }

        auto rule = memoTable->grammar->ruleNameWithPrecedenceToRule[topLevelRuleName];
        if (rule != nullptr)
        {
            cout << endl <<
                "====================================" << endl << endl << "AST for rule \"" + topLevelRuleName + "\":" << endl << endl;
            auto ruleClause = rule->labeledClause->clause;
            printAST(topLevelRuleName, ruleClause, memoTable);
        }
        else
        {
            cout << endl << "Rule \"" + topLevelRuleName + "\" does not exist" << endl;
        }

        auto syntaxErrors = memoTable->getSyntaxErrors(syntaxCoverageRuleNames);
        if (!syntaxErrors.empty())
        {
            printSyntaxErrors(syntaxErrors);
        }

        cout << endl << "Num match objects created: " + memoTable->numMatchObjectsCreated << endl;
        cout << "Num match objects memoized:  " + memoTable->numMatchObjectsMemoized << endl;
    }
};