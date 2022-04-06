#pragma once
#include "Terminal.hpp"
#include "Start.hpp"
#include "Nothing.hpp"
#include "CharSeq.hpp"
#include "CharSet.hpp"
#include "Rule.hpp"

class ClauseFactory
{
    Rule rule(string ruleName, Clause clause) 
    {
        Rule rule(ruleName, -1, NONE, clause);
        return rule;
    }

    Rule rule(string ruleName, int precedence, Associativity associativity, Clause clause) {
        Rule rule(ruleName, precedence, associativity, clause);
        return rule;
    }

    Clause seq(vector<Clause> subClauses) 
    {
        Seq sequence(subClauses);
        return sequence;
    }

    Clause oneOrMore(Clause subClause) 
    {
        
        if (subClause.TypeOfClause == 1 || subClause.TypeOfClause == 2 || subClause.TypeOfClause == 3
            || subClause.TypeOfClause == 4 || subClause.TypeOfClause == 5) 
        {
            return subClause;
        }
        OneOrMore oom(subClause);
        return oom;
    }

    /*
    public static Clause optional(Clause subClause) 
    {
        return first(subClause, nothing());
    }

    Clause zeroOrMore(Clause subClause) 
    {
        return optional(oneOrMore(subClause));
    }
    С опшиналом пока не ясно.
    */


};