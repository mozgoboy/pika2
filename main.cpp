#include "Grammar.hpp";
#include "MetaGrammar.hpp";
#include "MemoTable.hpp";
#include "ParserInfo.hpp";

void tryParsing(Grammar* grammar, string topRuleName, vector<string> syntaxErrCoverageRules,
    string input) 
{
    auto memoTable = grammar->parse(input);
    memoTable->counter = 0;
    ParserInfo::printParseResult(topRuleName, memoTable, syntaxErrCoverageRules, false);
    cout << "COUNTER = " + memoTable->counter << endl;
}

void main()
{
    static string s = "Expr <- Expr '+' '1' / Term '+' '1';\n";
    s.append("Term <- Term sym:[a-z] / sym:[a-z];\n");
    auto grammar = MetaGrammar::parse(s);
    tryParsing(grammar, "Expr", vector<string>{ "Expr", "Term" }, "aaaaaaaa+1+1+1+1+1");
}