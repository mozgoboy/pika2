#pragma once

#include "ClauseFactory.hpp"
#include "Clause.hpp"
#include "Grammar.hpp"
#include "First.hpp"
#include "FollowedBy.hpp"
#include "NotFollowedBy.hpp"
#include "OneOrMore.hpp"
#include "Seq.hpp"
#include "Terminal.hpp"
#include "Rule.hpp"
#include "ParserInfo.hpp"
#include "StringUtils.hpp"
#include "ASTNode.hpp"
#include "ASTNodeLabel.hpp"
#include "RuleRef.hpp"
#include "str_switch.hpp"

// имена дл€ правил

#define GRAMMAR  "GRAMMAR"
#define WSC  "WSC"
#define COMMENT  "COMMENT"
#define RULE  "RULE"
#define CLAUSE  "CLAUSE"
#define IDENT  "IDENT"
#define PREC  "PREC"
#define NUM  "NUM"
#define NAME_CHAR  "NAME_CHAR"
#define CHAR_SET  "CHARSET"
#define HEX  "Hex"
#define CHAR_RANGE  "CHAR_RANGE"
#define CHAR_RANGE_CHAR  "CHAR_RANGE_CHAR"
#define QUOTED_STRING  "QUOTED_STR"
#define ESCAPED_CTRL_CHAR  "ESCAPED_CTRL_CHAR"
#define SINGLE_QUOTED_CHAR  "SINGLE_QUOTED_CHAR"
#define STR_QUOTED_CHAR  "STR_QUOTED_CHAR"
#define NOTHING  "NOTHING"
#define START  "START"


// имена дл€ вершин дерева разбора

#define RULE_AST  "RuleAST"
#define PREC_AST  "PrecAST"
#define R_ASSOC_AST  "RAssocAST"
#define L_ASSOC_AST  "LAssocAST"
#define IDENT_AST  "IdentAST"
#define LABEL_AST  "LabelAST"
#define LABEL_NAME_AST  "LabelNameAST"
#define LABEL_CLAUSE_AST  "LabelClauseAST"
#define SEQ_AST   "SeqAST"
#define FIRST_AST  "FirstAST"
#define FOLLOWED_BY_AST  "FollowedByAST"
#define NOT_FOLLOWED_BY_AST  "NotFollowedByAST"
#define ONE_OR_MORE_AST  "OneOrMoreAST"
#define ZERO_OR_MORE_AST  "ZeroOrMoreAST"
#define OPTIONAL_AST  "OptionalAST"
#define SINGLE_QUOTED_CHAR_AST  "SingleQuotedCharAST"
#define CHAR_RANGE_AST  "CharRangeAST"
#define QUOTED_STRING_AST  "QuotedStringAST"
#define START_AST  "StartAST"
#define NOTHING_AST  "NothingAST"

class MetaGrammar
{
private:



    map<Clause, int> clauseTypeToPrecedence;
    clauseTypeToPrecedence[Terminal] = 7;
    clauseTypeToPrecedence[RuleRef] = 7;
    clauseTypeToPrecedence[OneOrMore] = 6;
    clauseTypeToPrecedence[NotFollowedBy] = 5;
    clauseTypeToPrecedence[FollowedBy] = 5;
    clauseTypeToPrecedence[ASTNodeLabel] = 3;
    clauseTypeToPrecedence[Seq] = 2;
    clauseTypeToPrecedence[First] = 1;

    /*
    * 1) ¬се эти классы - дочерние дл€ клоза, не совсем пон€тно как это описать
    * 2)  ак-то указать что этот umap не должен измен€тьс€
    */

    Clause* expectOne(vector<Clause*> clauses, ASTNode* astNode) 
    {
        if (clauses.size() != 1) {
            cout << "Expected one subclause, got " << clauses.size() << ": " << astNode->toString();
        }
        return clauses[0];
    }

    vector<Clause*> parseASTNodes(vector<ASTNode*> astNodes) 
    {
        vector<Clause*> clauses;
        for (auto astNode : astNodes) 
        {
            clauses.push_back(parseASTNode(astNode));
        }
        return clauses;
    }

    Clause* parseASTNode(ASTNode* astNode) 
    {
        Clause* clause;
        vector<char> s;
        SWITCH (astNode->label) 
        {
        CASE ( SEQ_AST ) :
            clause = ClauseFactory::seq(parseASTNodes(astNode->children));
            break;
        CASE (FIRST_AST ):
            clause = ClauseFactory::first(parseASTNodes(astNode->children));
            break;
        CASE ( ONE_OR_MORE_AST ):
            clause = ClauseFactory::oneOrMore(expectOne(parseASTNodes(astNode->children), astNode));
            break;
        CASE (ZERO_OR_MORE_AST):
            clause = ClauseFactory::zeroOrMore(expectOne(parseASTNodes(astNode->children), astNode));
            break;
        CASE (OPTIONAL_AST):
            clause = ClauseFactory::optional(expectOne(parseASTNodes(astNode->children), astNode));
            break;
        CASE (FOLLOWED_BY_AST):
            clause = ClauseFactory::followedBy(expectOne(parseASTNodes(astNode->children), astNode));
            break;
        CASE (NOT_FOLLOWED_BY_AST):
            clause = ClauseFactory::notFollowedBy(expectOne(parseASTNodes(astNode->children), astNode));
            break;
        CASE (LABEL_AST):
            clause = ast(astNode->getFirstChild()->getText(), parseASTNode(astNode->getSecondChild()->getFirstChild()));
            break;
        CASE (IDENT_AST):
            clause = ruleRef(astNode->getText());
            break;
        CASE (QUOTED_STRING_AST):
            clause = ClauseFactory::str(StringUtils::unescapeString(astNode->getText()));
            break;
        CASE(SINGLE_QUOTED_CHAR_AST) :
            
            s.push_back(StringUtils::unescapeChar(astNode->getText()));
            clause = ClauseFactory::c(s);
            break;
        CASE (START_AST):
            clause = ClauseFactory::start();
            break;
        CASE (NOTHING_AST):
            clause = ClauseFactory::nothing();
            break;
        CASE (CHAR_RANGE_AST):
            clause = ClauseFactory::cRange(astNode->getText());
            break;
        default:
            clause = expectOne(parseASTNodes(astNode->children), astNode);
            break;
        }
        return clause;
    }
    // “ут вызываетс€ много конструкторов из других файлов, хот€ со свитчем до конца не €сно.

    Rule parseRule(ASTNode* ruleNode) 
    {
        string ruleName = ruleNode->getFirstChild()->getText();
        auto hasPrecedence = ruleNode->children.size() > 2;
        Rule::Associativity associativity;
        if (ruleNode->children.size() < 4)
        {
            associativity = Rule::Associativity::NONE;
        }
        else
        {
            if (ruleNode->getThirdChild()->label == L_ASSOC_AST)
            {
                associativity = Rule::Associativity::LEFT;
            }
            else
            {
                if (ruleNode->getThirdChild()->label == R_ASSOC_AST)
                {
                    associativity = Rule::Associativity::RIGHT;
                }
                else
                {
                    associativity = Rule::Associativity::NONE;
                }
            }
        }
        int precedence;
        if (hasPrecedence)
        {
            precedence = stoi(ruleNode->getSecondChild()->getText());
        }
        else
        {
            precedence = -1;
        }
        /*
        if (hasPrecedence && precedence < 0) {
            throw new IllegalArgumentException("Precedence needs to be zero or positive (rule " + ruleName
                + " has precedence level " + precedence + ")");
        }*/
        auto astNode = ruleNode->getChild(ruleNode->children.size() - 1);
        Clause* clause = parseASTNode(astNode);
        return Rule(ruleName, precedence, associativity, clause);
    }

public:
    vector<Rule> NecessaryRules = {
        Rule(GRAMMAR, ClauseFactory::seq(ClauseFactory::start(), ruleRef(WSC), ClauseFactory::oneOrMore(ruleRef(RULE)))),
        Rule(RULE, ast(RULE_AST, ClauseFactory::seq(ruleRef(IDENT), ruleRef(WSC), ClauseFactory::optional(ruleRef(PREC)), ClauseFactory::str("<-"), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC), ClauseFactory::c(vector<char> x = {';'}), ruleRef(WSC)))),
        Rule(CLAUSE, 8, /* associativity = */ Rule::Associativity::NONE, ClauseFactory::seq(ClauseFactory::c('('), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC), c(')'))),
        Rule(CLAUSE, 7, /* associativity = */ Rule::Associativity::NONE, ClauseFactory::first(ruleRef(IDENT), ruleRef(QUOTED_STRING), ruleRef(CHAR_SET), ruleRef(NOTHING), ruleRef(START))),
        Rule(CLAUSE, 6, /* associativity = */ Rule::Associativity::NONE, ClauseFactory::first(ClauseFactory::seq(ast(ONE_OR_MORE_AST, ruleRef(CLAUSE)), ruleRef(WSC), c('+')), ClauseFactory::seq(ast(ZERO_OR_MORE_AST, ruleRef(CLAUSE)), ruleRef(WSC), ClauseFactory::c('*')))),
        Rule(CLAUSE, 5, /* associativity = */ Rule::Associativity::NONE, ClauseFactory::first(ClauseFactory::seq(ClauseFactory::c('&'), Ast(FOLLOWED_BY_AST, ruleRef(CLAUSE))), ClauseFactory::seq(c('!'), ast(NOT_FOLLOWED_BY_AST, ruleRef(CLAUSE))))),
        Rule(CLAUSE, 4, /* associativity = */ Rule::Associativity::NONE, ClauseFactory::seq(ast(OPTIONAL_AST, ruleRef(CLAUSE)), ruleRef(WSC), c('?'))),
        Rule(CLAUSE, 3, /* associativity = */ Rule::Associativity::NONE, ast(LABEL_AST, ClauseFactory::seq(ast(LABEL_NAME_AST, ruleRef(IDENT)), ruleRef(WSC), ClauseFactory::c(':'), ruleRef(WSC), ast(LABEL_CLAUSE_AST, ruleRef(CLAUSE)), ruleRef(WSC)))),
        Rule(CLAUSE, 2, /* associativity = */ Rule::Associativity::NONE, ast(SEQ_AST, ClauseFactory::seq(ruleRef(CLAUSE), ruleRef(WSC), ClauseFactory::oneOrMore(ClauseFactory::seq(ruleRef(CLAUSE), ruleRef(WSC)))))),
        Rule(CLAUSE, 1, /* associativity = */ Rule::Associativity::NONE, ast(FIRST_AST, ClauseFactory::seq(ruleRef(CLAUSE), ruleRef(WSC), ClauseFactory::oneOrMore(ClauseFactory::seq(c('/'), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC)))))),
        Rule(WSC, ClauseFactory::zeroOrMore(ClauseFactory::first(ClauseFactory::c(' ', '\n', '\r', '\t'), ruleRef(COMMENT)))),
        Rule(COMMENT, ClauseFactory::seq(c('#'), zeroOrMore(ClauseFactory::c('\n').invert()))),
        Rule(IDENT, ast(IDENT_AST, seq(ruleRef(NAME_CHAR), zeroOrMore(First(ruleRef(NAME_CHAR), ClauseFactory::cRange('0', '9')))))),
        Rule(NUM, ClauseFactory::oneOrMore(ClauseFactory::cRange('0', '9'))),
        Rule(NAME_CHAR, ClauseFactory::c(ClauseFactory::cRange('a', 'z'), ClauseFactory::cRange('A', 'Z'), ClauseFactory::c('_', '-'))),
        Rule(PREC, ClauseFactory::seq(ClauseFactory::c('['), ruleRef(WSC), ast(PREC_AST, ruleRef(NUM)), ruleRef(WSC), ClauseFactory::optional(ClauseFactory::seq(c(','), ruleRef(WSC), ClauseFactory::first(ast(R_ASSOC_AST, ClauseFactory::first(ClauseFactory::c('r'), ClauseFactory::c('R'))), ast(L_ASSOC_AST, ClauseFactory::first(ClauseFactory::c('l'), ClauseFactory::c('L')))), ruleRef(WSC))), ClauseFactory::c(']'), ruleRef(WSC))),
        Rule(CHAR_SET, ClauseFactory::first(ClauseFactory::seq(ClauseFactory::c('\''), ast(SINGLE_QUOTED_CHAR_AST, ruleRef(SINGLE_QUOTED_CHAR)), ClauseFactory::c('\'')), ClauseFactory::seq(ClauseFactory::c('['), ast(CHAR_RANGE_AST, ClauseFactory::seq(ClauseFactory::optional(ClauseFactory::c('^')), ClauseFactory::oneOrMore(ClauseFactory::first(ruleRef(CHAR_RANGE), ruleRef(CHAR_RANGE_CHAR))))), ClauseFactory::c(']')))),
        Rule(SINGLE_QUOTED_CHAR, ClauseFactory::first(ruleRef(ESCAPED_CTRL_CHAR), ClauseFactory::c('\'').invert())),
        Rule(CHAR_RANGE, ClauseFactory::seq(ruleRef(CHAR_RANGE_CHAR), ClauseFactory::c('-'), ruleRef(CHAR_RANGE_CHAR))),
        Rule(CHAR_RANGE_CHAR, ClauseFactory::first(ClauseFactory::c('\\', ']').invert(), ruleRef(ESCAPED_CTRL_CHAR), ClauseFactory::str("\\-"), ClauseFactory::str("\\\\"), ClauseFactory::str("\\]"), ClauseFactory::str("\\^"))),
        Rule(QUOTED_STRING, ClauseFactory::seq(ClauseFactory::c('"'), ast(QUOTED_STRING_AST, ClauseFactory::zeroOrMore(ruleRef(STR_QUOTED_CHAR))), ClauseFactory::c('"'))),
        Rule(STR_QUOTED_CHAR, ClauseFactory::first(ruleRef(ESCAPED_CTRL_CHAR), ClauseFactory::c('"', '\\').invert())),
        Rule(HEX, ClauseFactory::c(ClauseFactory::cRange('0', '9'), ClauseFactory::cRange('a', 'f'), ClauseFactory::cRange('A', 'F'))),
        Rule(ESCAPED_CTRL_CHAR, ClauseFactory::first(ClauseFactory::str("\\t"), ClauseFactory::str("\\b"), ClauseFactory::str("\\n"), ClauseFactory::str("\\r"), ClauseFactory::str("\\f"), ClauseFactory::str("\\'"), ClauseFactory::str("\\\""), ClauseFactory::str("\\\\"), ClauseFactory::seq(ClauseFactory::str("\\u"), ruleRef(HEX), ruleRef(HEX), ruleRef(HEX), ruleRef(HEX)))),
        Rule(NOTHING, ast(NOTHING_AST, ClauseFactory::seq(ClauseFactory::c('('), ruleRef(WSC), ClauseFactory::c(')')))),
        Rule(START, ast(START_AST, ClauseFactory::c('^')))
    };
    Grammar grammar(NecessaryRules);

    static bool needToAddParensAroundSubClause(Clause* parentClause, Clause* subClause)
    {
        int clausePrec;
        if (parentClause->TypeOfClause == TypesOfClauses::Terminal)
        {
            clausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            clausePrec = clauseTypeToPrecedence[parentClause->getClass()];
        }

        int subClausePrec;
        if (subClause->TypeOfClause == TypesOfClauses::Terminal)
        {
            subClausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            subClausePrec = clauseTypeToPrecedence[subClause->getClass()];
        }

        // √еткласс возвращает класса потомка родительского класса, не совсем €сно как это в плюсах работает.

        return ((parentClause->TypeOfClause == TypesOfClauses::First && subClause->TypeOfClause == TypesOfClauses::Seq)
            || (subClausePrec <= clausePrec));
    }

    static bool needToAddParensAroundASTNodeLabel(Clause* subClause)
    {
        int astNodeLabelPrec = clauseTypeToPrecedence[ASTNodeLabel];
        int subClausePrec;
        if (subClause.TypeOfClause == 0) // 0 это просто значение инта дл€ терминала, можно любое другое число забить
        {
            subClausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            subClausePrec = clauseTypeToPrecedence[subClause.getClass()];
        }
        return subClausePrec < astNodeLabelPrec;
    }

    Grammar parse(string input) 
    {
        auto memoTable = grammar->parse(input);

        /*
        auto syntaxErrors = memoTable.getSyntaxErrors(GRAMMAR, RULE,
            CLAUSE + "[" + clauseTypeToPrecedence.get(First.class) + "]");
        if (!syntaxErrors.isEmpty()) {
            ParserInfo.printSyntaxErrors(syntaxErrors);
        }

        «десь расписан вывод грамматических ошибок, нам пока не нужен

        */
        auto topLevelRule = grammar.getRule(GRAMMAR);
        auto topLevelRuleASTNodeLabel = topLevelRule.labeledClause.astNodeLabel;
        if (topLevelRuleASTNodeLabel == nullptr) 
        {
            topLevelRuleASTNodeLabel = "<root>";
        }
        auto topLevelMatches = grammar.getNonOverlappingMatches(GRAMMAR, memoTable);
        /*
        if (topLevelMatches.isEmpty()) {
            throw new IllegalArgumentException("Toplevel rule \"" + GRAMMAR + "\" did not match");
        }
        else if (topLevelMatches.size() > 1) {
            System.out.println("\nMultiple toplevel matches:");
            for (var topLevelMatch : topLevelMatches) {
                var topLevelASTNode = new ASTNode(topLevelRuleASTNodeLabel, topLevelMatch, input);
                System.out.println(topLevelASTNode);
            }
            throw new IllegalArgumentException("Stopping");
        }

        ѕробрасывание ошибок тоже пока не учитываем
        */
        auto topLevelMatch = topLevelMatches.get(0);

        // TreeUtils.printTreeView(topLevelMatch, input);

        auto topLevelASTNode = new ASTNode(topLevelRuleASTNodeLabel, topLevelMatch, input);

        // System.out.println(topLevelASTNode);

        vector<Rule> rules;
        for (ASTNode astNode : topLevelASTNode->children) 
        {
            /*
            if (!astNode.label.equals(RULE_AST)) {
                throw new IllegalArgumentException("Wrong node type");
            }
            */
            Rule rule = parseRule(astNode);
            rules.push_back(rule);
        }
        return Grammar(rules);
    }
};
