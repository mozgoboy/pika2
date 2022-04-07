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


using namespace ClauseFactory;
// имена для правил

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


// имена для вершин дерева разбора

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
    * 1) Все эти классы - дочерние для клоза, не совсем понятно как это описать
    * 2) Как-то указать что этот umap не должен изменяться
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
        if (astNode->label == SEQ_AST)
        {
            clause = seq(parseASTNodes(astNode->children).toArray(new Clause[0]));
        }
        else if (astNode->label == SEQ_AST) {
            clause = first(parseASTNodes(astNode->children).toArray(new Clause[0]));
        }
        else if (astNode->label == ONE_OR_MORE_AST) {
            clause = oneOrMore(expectOne(parseASTNodes(astNode->children), astNode));
        }
        else if (astNode->label == ZERO_OR_MORE_AST) {
            clause = zeroOrMore(expectOne(parseASTNodes(astNode.children), astNode));
        } else if (astNode->label == OPTIONAL_AST) {
            clause = optional(expectOne(parseASTNodes(astNode.children), astNode));
        }
        else if (astNode->label == FOLLOWED_BY_AST) {
            clause = followedBy(expectOne(parseASTNodes(astNode.children), astNode));
        }
        else if (astNode->label == NOT_FOLLOWED_BY_AST) {
            clause = notFollowedBy(expectOne(parseASTNodes(astNode.children), astNode));
        }
        else if (astNode->label == LABEL_AST ) {
            clause = ast(astNode.getFirstChild().getText(), parseASTNode(astNode.getSecondChild().getFirstChild()));
        }
        else if (astNode->label == IDENT_AST ) {
            clause = ruleRef(astNode.getText());
        }
        else if (astNode->label == QUOTED_STRING_AST ) {
            clause = str(StringUtils::unescapeString(astNode.getText()));
        }
        else if (astNode->label == SINGLE_QUOTED_CHAR_AST) {
            clause = c(StringUtils::unescapeChar(astNode.getText()));
        }
        else if (astNode->label == START_AST) {
            clause = start();
        }
        else if (astNode->label == NOTHING_AST) {
            clause = nothing();
        }
        else if (astNode->label == CHAR_RANGE_AST)
            clause = cRange(astNode.getText());
        else {
            clause = expectOne(parseASTNodes(astNode.children), astNode);
        }
        return clause;
    }
    // Тут вызывается много конструкторов из других файлов, хотя со свитчем до конца не ясно.

    Rule parseRule(ASTNode ruleNode) {
        string ruleName = ruleNode.getFirstChild().getText();
        auto hasPrecedence = ruleNode.children.size() > 2;
        Rule::Associativity associativity;
        if (ruleNode.children.size() < 4)
        {
            associativity = NONE;
        }
        else
        {
            if (ruleNode.getThirdChild().label.equals(L_ASSOC_AST))
            {
                associativity = LEFT;
            }
            else
            {
                if (ruleNode.getThirdChild().label.equals(R_ASSOC_AST))
                {
                    associativity = RIGHT;
                }
                else
                {
                    associativity = NONE;
                }
            }
        }
        int precedence;
        if (hasPrecedence)
        {
            precedence = int(ruleNode.getSecondChild().getText());
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
        auto astNode = ruleNode.getChild(ruleNode.children.size() - 1);
        Clause clause = parseASTNode(astNode);
        return Rule(ruleName, precedence, associativity, clause);
    }

public:
    vector<Rule> NecessaryRules = {
        Rule(GRAMMAR, seq(start(), ruleRef(WSC), oneOrMore(ruleRef(RULE)))),
        Rule(RULE, ast(RULE_AST, Seq(ruleRef(IDENT), ruleRef(WSC), optional(ruleRef(PREC)), str("<-"), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC), c(';'), ruleRef(WSC)))),
        Rule(CLAUSE, 8, /* associativity = */ Associativity.NONE, seq(c('('), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC), c(')'))),
        Rule(CLAUSE, 7, /* associativity = */ Associativity.NONE, first(ruleRef(IDENT), ruleRef(QUOTED_STRING), ruleRef(CHAR_SET), ruleRef(NOTHING), ruleRef(START))),
        Rule(CLAUSE, 6, /* associativity = */ Associativity.NONE, first(Seq(ast(ONE_OR_MORE_AST, ruleRef(CLAUSE)), ruleRef(WSC), c('+')), Seq(ast(ZERO_OR_MORE_AST, ruleRef(CLAUSE)), ruleRef(WSC), c('*')))),
        Rule(CLAUSE, 5, /* associativity = */ Associativity.NONE, first(Seq(c('&'), Ast(FOLLOWED_BY_AST, ruleRef(CLAUSE))), Seq(c('!'), ast(NOT_FOLLOWED_BY_AST, ruleRef(CLAUSE))))),
        Rule(CLAUSE, 4, /* associativity = */ Associativity.NONE, Seq(ast(OPTIONAL_AST, ruleRef(CLAUSE)), ruleRef(WSC), c('?'))),
        Rule(CLAUSE, 3, /* associativity = */ Associativity.NONE, ast(LABEL_AST, seq(ast(LABEL_NAME_AST, ruleRef(IDENT)), ruleRef(WSC), c(':'), ruleRef(WSC), ast(LABEL_CLAUSE_AST, ruleRef(CLAUSE)), ruleRef(WSC)))),
        Rule(CLAUSE, 2, /* associativity = */ Associativity.NONE, ast(SEQ_AST, seq(ruleRef(CLAUSE), ruleRef(WSC), oneOrMore(seq(ruleRef(CLAUSE), ruleRef(WSC)))))),
        Rule(CLAUSE, 1, /* associativity = */ Associativity.NONE, ast(FIRST_AST, seq(ruleRef(CLAUSE), ruleRef(WSC), oneOrMore(seq(c('/'), ruleRef(WSC), ruleRef(CLAUSE), ruleRef(WSC)))))),
        Rule(WSC, zeroOrMore(fFirst(c(' ', '\n', '\r', '\t'), ruleRef(COMMENT)))),
        Rule(COMMENT, seq(c('#'), zeroOrMore(c('\n').invert()))),
        Rule(IDENT, ast(IDENT_AST, seq(ruleRef(NAME_CHAR), zeroOrMore(First(ruleRef(NAME_CHAR), cRange('0', '9')))))),
        Rule(NUM, oneOrMore(cRange('0', '9'))),
        Rule(NAME_CHAR, c(cRange('a', 'z'), cRange('A', 'Z'), c('_', '-'))),
        Rule(PREC, seq(c('['), ruleRef(WSC), ast(PREC_AST, ruleRef(NUM)), ruleRef(WSC), optional(seq(c(','), ruleRef(WSC), First(ast(R_ASSOC_AST, First(c('r'), c('R'))), ast(L_ASSOC_AST, First(c('l'), c('L')))), ruleRef(WSC))), c(']'), ruleRef(WSC))),
        Rule(CHAR_SET, first(seq(c('\''), ast(SINGLE_QUOTED_CHAR_AST, ruleRef(SINGLE_QUOTED_CHAR)), c('\'')), seq(c('['), ast(CHAR_RANGE_AST, seq(optional(c('^')), oneOrMore(First(ruleRef(CHAR_RANGE), ruleRef(CHAR_RANGE_CHAR))))), c(']')))),
        Rule(SINGLE_QUOTED_CHAR, first(ruleRef(ESCAPED_CTRL_CHAR), c('\'').invert())),
        Rule(CHAR_RANGE, seq(ruleRef(CHAR_RANGE_CHAR), c('-'), ruleRef(CHAR_RANGE_CHAR))),
        Rule(CHAR_RANGE_CHAR, First(c('\\', ']').invert(), ruleRef(ESCAPED_CTRL_CHAR), str("\\-"), str("\\\\"), str("\\]"), str("\\^"))),
        Rule(QUOTED_STRING, seq(c('"'), ast(QUOTED_STRING_AST, zeroOrMore(ruleRef(STR_QUOTED_CHAR))), c('"'))),
        Rule(STR_QUOTED_CHAR, first(ruleRef(ESCAPED_CTRL_CHAR), c('"', '\\').invert())),
        Rule(HEX, c(cRange('0', '9'), cRange('a', 'f'), cRange('A', 'F'))),
        Rule(ESCAPED_CTRL_CHAR, First(str("\\t"), str("\\b"), str("\\n"), str("\\r"), str("\\f"), str("\\'"), str("\\\""), str("\\\\"), seq(str("\\u"), ruleRef(HEX), ruleRef(HEX), ruleRef(HEX), ruleRef(HEX)))),
        Rule(NOTHING, ast(NOTHING_AST, seq(c('('), ruleRef(WSC), c(')')))),
        Rule(START, ast(START_AST, c('^')))
    };
    Grammar grammar(NecessaryRules);

    static bool needToAddParensAroundSubClause(Clause* parentClause, Clause* subClause)
    {
        int clausePrec;
        if (parentClause.TypeOfClause == 0) // 0 это просто значение инта для терминала, можно любое другое число забить
        {
            clausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            clausePrec = clauseTypeToPrecedence[parentClause.getClass()];
        }

        int subClausePrec;
        if (subClause.TypeOfClause == 0) // 0 это просто значение инта для терминала, можно любое другое число забить
        {
            subClausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            subClausePrec = clauseTypeToPrecedence[subClause.getClass()];
        }

        // Геткласс возвращает класса потомка родительского класса, не совсем ясно как это в плюсах работает.

        return ((parentClause.TypeOfClause == 1 && subClause.TypeOfClause == 2) // 1 для first и 2 для seq
            || (subClausePrec <= clausePrec));
    }

    static bool needToAddParensAroundASTNodeLabel(Clause* subClause)
    {
        int astNodeLabelPrec = clauseTypeToPrecedence.[ASTNodeLabel];
        int subClausePrec;
        if (subClause.TypeOfClause == 0) // 0 это просто значение инта для терминала, можно любое другое число забить
        {
            subClausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            subClausePrec = clauseTypeToPrecedence[subClause.getClass()];
        }
        return subClausePrec < astNodeLabelPrec;
    }

    Grammar parse(string input) {
        auto memoTable = grammar.parse(input);

        /*
        auto syntaxErrors = memoTable.getSyntaxErrors(GRAMMAR, RULE,
            CLAUSE + "[" + clauseTypeToPrecedence.get(First.class) + "]");
        if (!syntaxErrors.isEmpty()) {
            ParserInfo.printSyntaxErrors(syntaxErrors);
        }

        Здесь расписан вывод грамматических ошибок, нам пока не нужен

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

        Пробрасывание ошибок тоже пока не учитываем
        */
        auto topLevelMatch = topLevelMatches.get(0);

        // TreeUtils.printTreeView(topLevelMatch, input);

        auto topLevelASTNode = new ASTNode(topLevelRuleASTNodeLabel, topLevelMatch, input);

        // System.out.println(topLevelASTNode);

        vector<Rule> rules;
        for (ASTNode astNode : topLevelASTNode.children) 
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
