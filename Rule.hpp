#pragma once
#include "Clause.hpp"
#include "LabeledClause.hpp"
#include "ASTNodeLabel.hpp"

class Rule
{
public:
    string ruleName;

    int precedence;

    LabeledClause* labeledClause;

    enum Associativity
    {
        LEFT, RIGHT, NONE
    };

    Associativity associativity;

    Rule(string ruleName, int precedence, Associativity associativity, Clause* clause) 
    {
        this->ruleName = ruleName;
        this->precedence = precedence;
        this->associativity = associativity;

        string astNodeLabel;
        Clause* clauseToUse = clause;
        if (clause->TypeOfClause == TypesOfClauses::ASTNodeLabel) {
            // Transfer ASTNodeLabel.astNodeLabel to astNodeLabel
            astNodeLabel = ((ASTNodeLabel*)clauseToUse)->astNodeLabel;
            // skip over ASTNodeLabel node when adding subClause to subClauses array
            clauseToUse = clauseToUse->labeledSubClauses[0]->clause;
        }
        LabeledClause x(clauseToUse, astNodeLabel);
        this->labeledClause = &x;
    }

    Rule(string ruleName, Clause* clause) {
        Rule(ruleName, -1, NONE, clause);
    }

    string toString() {
        string buf;
        buf.append(ruleName);
        buf.append(" <- ");
        buf.append(labeledClause->toString());
        return buf;
    }
    // Опять же нужно просто для вывода.
};