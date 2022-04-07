#include "Grammar.hpp";
#include "MetaGrammar.hpp";
#include "MemoTable.hpp";
#include "ParserInfo.hpp";

void tryParsing(Grammar* grammar, string topRuleName, vector<string> syntaxErrCoverageRules,
    string input) 
{
    //MemoTable.counter = 0;
    auto memoTable = grammar->parse(input);
    ParserInfo::printParseResult(topRuleName, memoTable, syntaxErrCoverageRules, false);
    cout << "COUNTER = " + memoTable->counter << endl;
}

void main()
{
    auto grammar = MetaGrammar.parse("Expr <- Expr '+' '1' / Term '+' '1';\n" //
        + "Term <- Term sym:[a-z] / sym:[a-z];\n");
    tryParsing(grammar, "Expr", vector<string>{ "Expr", "Term" }, "aaaaaaaa+1+1+1+1+1");
}