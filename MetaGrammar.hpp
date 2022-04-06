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

class MetaGrammar
{
private:

    // имена для правил

    string GRAMMAR = "GRAMMAR";
    string WSC = "WSC";
    string COMMENT = "COMMENT";
    string RULE = "RULE";
    string CLAUSE = "CLAUSE";
    string IDENT = "IDENT";
    string PREC = "PREC";
    string NUM = "NUM";
    string NAME_CHAR = "NAME_CHAR";
    string CHAR_SET = "CHARSET";
    string HEX = "Hex";
    string CHAR_RANGE = "CHAR_RANGE";
    string CHAR_RANGE_CHAR = "CHAR_RANGE_CHAR";
    string QUOTED_STRING = "QUOTED_STR";
    string ESCAPED_CTRL_CHAR = "ESCAPED_CTRL_CHAR";
    string SINGLE_QUOTED_CHAR = "SINGLE_QUOTED_CHAR";
    string STR_QUOTED_CHAR = "STR_QUOTED_CHAR";
    string NOTHING = "NOTHING";
    string START = "START";

    // имена для вершин дерева разбора

    string RULE_AST = "RuleAST";
    string PREC_AST = "PrecAST";
    string R_ASSOC_AST = "RAssocAST";
    string L_ASSOC_AST = "LAssocAST";
    string IDENT_AST = "IdentAST";
    string LABEL_AST = "LabelAST";
    string LABEL_NAME_AST = "LabelNameAST";
    string LABEL_CLAUSE_AST = "LabelClauseAST";
    string SEQ_AST = "SeqAST";
    string FIRST_AST = "FirstAST";
    string FOLLOWED_BY_AST = "FollowedByAST";
    string NOT_FOLLOWED_BY_AST = "NotFollowedByAST";
    string ONE_OR_MORE_AST = "OneOrMoreAST";
    string ZERO_OR_MORE_AST = "ZeroOrMoreAST";
    string OPTIONAL_AST = "OptionalAST";
    string SINGLE_QUOTED_CHAR_AST = "SingleQuotedCharAST";
    string CHAR_RANGE_AST = "CharRangeAST";
    string QUOTED_STRING_AST = "QuotedStringAST";
    string START_AST = "StartAST";
    string NOTHING_AST = "NothingAST";

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
        switch (astNodelabel) 
        {
        case SEQ_AST:
            clause = seq(parseASTNodes(astNode.children).toArray(new Clause[0]));
            break;
        case FIRST_AST:
            clause = first(parseASTNodes(astNode.children).toArray(new Clause[0]));
            break;
        case ONE_OR_MORE_AST:
            clause = oneOrMore(expectOne(parseASTNodes(astNode.children), astNode));
            break;
        case ZERO_OR_MORE_AST:
            clause = zeroOrMore(expectOne(parseASTNodes(astNode.children), astNode));
            break;
        case OPTIONAL_AST:
            clause = optional(expectOne(parseASTNodes(astNode.children), astNode));
            break;
        case FOLLOWED_BY_AST:
            clause = followedBy(expectOne(parseASTNodes(astNode.children), astNode));
            break;
        case NOT_FOLLOWED_BY_AST:
            clause = notFollowedBy(expectOne(parseASTNodes(astNode.children), astNode));
            break;
        case LABEL_AST:
            clause = ast(astNode.getFirstChild().getText(), parseASTNode(astNode.getSecondChild().getFirstChild()));
            break;
        case IDENT_AST:
            clause = ruleRef(astNode.getText());
            break;
        case QUOTED_STRING_AST:
            clause = str(StringUtils.unescapeString(astNode.getText()));
            break;
        case SINGLE_QUOTED_CHAR_AST:
            clause = c(StringUtils.unescapeChar(astNode.getText()));
            break;
        case START_AST:
            clause = start();
            break;
        case NOTHING_AST:
            clause = nothing();
            break;
        case CHAR_RANGE_AST:
            clause = cRange(astNode.getText());
            break;
        default:
            clause = expectOne(parseASTNodes(astNode.children), astNode);
            break;
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
