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



    map<Clause*, int> clauseTypeToPrecedence;
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

    static Clause* expectOne(vector<Clause*> clauses, ASTNode* astNode) 
    {
        if (clauses.size() != 1) {
            cout << "Expected one subclause, got " << clauses.size() << ": " << astNode->toString();
        }
        return clauses[0];
    }

    static vector<Clause*> parseASTNodes(vector<ASTNode*> astNodes) 
    {
        vector<Clause*> clauses;
        for (auto astNode : astNodes) 
        {
            clauses.push_back(parseASTNode(astNode));
        }
        return clauses;
    }

    static Clause* parseASTNode(ASTNode* astNode) 
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
            clause = ClauseFactory::ast(astNode->getFirstChild()->getText(), parseASTNode(astNode->getSecondChild()->getFirstChild()));
            break;
        CASE (IDENT_AST):
            clause = ClauseFactory::ruleRef(astNode->getText());
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

    static Rule* parseRule(ASTNode* ruleNode) 
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
        return new Rule(ruleName, precedence, associativity, clause);
    }

public:
    vector<Rule*> NecessaryRules = {
        new Rule(GRAMMAR, 
            ClauseFactory::seq(vector<Clause*> 
            { 
                ClauseFactory::start(), 
                ClauseFactory::ruleRef(WSC), 
                ClauseFactory::oneOrMore(ClauseFactory::ruleRef(RULE))
            }
            )
        ),
        new Rule(RULE, 
            ClauseFactory::ast(RULE_AST, 
                ClauseFactory::seq(vector<Clause*> 
                {
                    ClauseFactory::ruleRef(IDENT), 
                    ClauseFactory::ruleRef(WSC), 
                    ClauseFactory::optional(ClauseFactory::ruleRef(PREC)), 
                    ClauseFactory::str("<-"), 
                    ClauseFactory::ruleRef(WSC), 
                    ClauseFactory::ruleRef(CLAUSE), 
                    ClauseFactory::ruleRef(WSC), 
                    ClauseFactory::c(vector<char> {';'}), 
                    ClauseFactory::ruleRef(WSC)
                })
            )
        ),
        new Rule(CLAUSE,
            8,
            /* associativity = */ Rule::Associativity::NONE, 
            ClauseFactory::seq(vector<Clause*> 
            {
                (Clause*)ClauseFactory::c(vector<char> {'('}), 
                ClauseFactory::ruleRef(WSC), 
                ClauseFactory::ruleRef(CLAUSE), 
                ClauseFactory::ruleRef(WSC), 
                (Clause*)ClauseFactory::c(vector<char> {')'}) 
            })
        ),
        new Rule(CLAUSE,
            7,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::first(vector<Clause*> 
            {
                ClauseFactory::ruleRef(IDENT),
                ClauseFactory::ruleRef(QUOTED_STRING),
                ClauseFactory::ruleRef(CHAR_SET),
                ClauseFactory::ruleRef(NOTHING),
                ClauseFactory::ruleRef(START)
            })
        ),
        new Rule(CLAUSE,
            6,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::first(ClauseFactory::seq(ast(ONE_OR_MORE_AST,
                ClauseFactory::ruleRef(CLAUSE)),
                ClauseFactory::ruleRef(WSC), c('+')
            ), ClauseFactory::seq(ClauseFactory::ast(ZERO_OR_MORE_AST,
                    ClauseFactory::ruleRef(CLAUSE)
                ), ClauseFactory::ruleRef(WSC),
                ClauseFactory::c('*')))
        ),
        new Rule(CLAUSE,
            5,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::first(ClauseFactory::seq(ClauseFactory::c('&'),
                ClauseFactory::ast(FOLLOWED_BY_AST,
                    ClauseFactory::ruleRef(CLAUSE))
            ), ClauseFactory::seq(c('!'),
                ClauseFactory::ast(NOT_FOLLOWED_BY_AST,
                    ClauseFactory::ruleRef(CLAUSE)))
            )
        ),
        new Rule(CLAUSE,
            4,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::seq(ClauseFactory::ast(OPTIONAL_AST,
                ClauseFactory::ruleRef(CLAUSE)
            ), ClauseFactory::ruleRef(WSC),
            ClauseFactory::c('?'))
        ),
        new Rule(CLAUSE,
            3,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::ast(LABEL_AST,
                ClauseFactory::seq(ClauseFactory::ast(LABEL_NAME_AST,
                    ClauseFactory::ruleRef(IDENT)
                ), ClauseFactory::ruleRef(WSC),
                ClauseFactory::c(':'), 
                ClauseFactory::ruleRef(WSC),
                ClauseFactory::ast(LABEL_CLAUSE_AST,
                    ClauseFactory::ruleRef(CLAUSE)
                ), ClauseFactory::ruleRef(WSC))
            )
        ),
        new Rule(CLAUSE,
            2,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::ast(SEQ_AST,
                ClauseFactory::seq(ClauseFactory::ruleRef(CLAUSE),
                    ClauseFactory::ruleRef(WSC),
                    ClauseFactory::oneOrMore(ClauseFactory::seq(ClauseFactory::ruleRef(CLAUSE),
                        ClauseFactory::ruleRef(WSC)))
                )
            )
        ),
        new Rule(CLAUSE,
            1,
            /* associativity = */ Rule::Associativity::NONE,
            ClauseFactory::ast(FIRST_AST,
                ClauseFactory::seq(ClauseFactory::ruleRef(CLAUSE),
                    ClauseFactory::ruleRef(WSC),
                    ClauseFactory::oneOrMore(ClauseFactory::seq(c('/'),
                        ClauseFactory::ruleRef(WSC),
                        ClauseFactory::ruleRef(CLAUSE),
                        ClauseFactory::ruleRef(WSC))
                    )
                )
            )
        ),
        new Rule(WSC,
            ClauseFactory::zeroOrMore(ClauseFactory::first(ClauseFactory::c(' ', '\n', '\r', '\t'),
                ClauseFactory::ruleRef(COMMENT))
            )
        ),
        new Rule(COMMENT,
            ClauseFactory::seq(ClauseFactory::c('#'),
                ClauseFactory::zeroOrMore(ClauseFactory::c('\n').invert())
            )
        ),
        new Rule(IDENT,
            ClauseFactory::ast(IDENT_AST,
                ClauseFactory::seq(ClauseFactory::ruleRef(NAME_CHAR),
                    ClauseFactory::zeroOrMore(ClauseFactory::first(ClauseFactory::ruleRef(NAME_CHAR),
                        ClauseFactory::cRange('0', '9'))
                    )
                )
            )
        ),
        new Rule(NUM,
            ClauseFactory::oneOrMore(ClauseFactory::cRange('0', '9')
            )
        ),
        new Rule(NAME_CHAR,
            ClauseFactory::c(ClauseFactory::cRange('a', 'z'),
                ClauseFactory::cRange('A', 'Z'),
                ClauseFactory::c('_', '-')
            )
        ),
        new Rule(PREC,
            ClauseFactory::seq(ClauseFactory::c('['),
                ClauseFactory::ruleRef(WSC),
                ClauseFactory::ast(PREC_AST,
                    ClauseFactory::ruleRef(NUM)
                ),
                ClauseFactory::ruleRef(WSC),
                ClauseFactory::optional(ClauseFactory::seq(c(','),
                    ClauseFactory::ruleRef(WSC),
                    ClauseFactory::first(ClauseFactory::ast(R_ASSOC_AST,
                        ClauseFactory::first(ClauseFactory::c('r'),
                            ClauseFactory::c('R')
                        )
                    ),
                    ClauseFactory::ast(L_ASSOC_AST,
                        ClauseFactory::first(ClauseFactory::c('l'),
                            ClauseFactory::c('L'))
                    )
                    ),
                    ClauseFactory::ruleRef(WSC)
                )),
                ClauseFactory::c(']'),
                ClauseFactory::ruleRef(WSC)
            )
        ),
        new Rule(CHAR_SET,
            ClauseFactory::first(ClauseFactory::seq(ClauseFactory::c('\''),
                ClauseFactory::ast(SINGLE_QUOTED_CHAR_AST,
                    ClauseFactory::ruleRef(SINGLE_QUOTED_CHAR)
                ),
                ClauseFactory::c('\'')),
                ClauseFactory::seq(ClauseFactory::c('['),
                    ClauseFactory::ast(CHAR_RANGE_AST,
                        ClauseFactory::seq(ClauseFactory::optional(ClauseFactory::c('^')),
                            ClauseFactory::oneOrMore(ClauseFactory::first(ClauseFactory::ruleRef(CHAR_RANGE),
                                ClauseFactory::ruleRef(CHAR_RANGE_CHAR))
                            )
                        )
                    ),
                    ClauseFactory::c(']')
                )
            )
        ),
        new Rule(SINGLE_QUOTED_CHAR,
            ClauseFactory::first(ClauseFactory::ruleRef(ESCAPED_CTRL_CHAR),
                ClauseFactory::c('\'').invert()
            )
        ),
        new Rule(CHAR_RANGE,
            ClauseFactory::seq(ClauseFactory::ruleRef(CHAR_RANGE_CHAR),
                ClauseFactory::c('-'),
                ClauseFactory::ruleRef(CHAR_RANGE_CHAR)
            )
        ),
        new Rule(CHAR_RANGE_CHAR,
            ClauseFactory::first(ClauseFactory::c('\\', ']').invert(),
                ClauseFactory::ruleRef(ESCAPED_CTRL_CHAR),
                ClauseFactory::str("\\-"),
                ClauseFactory::str("\\\\"),
                ClauseFactory::str("\\]"),
                ClauseFactory::str("\\^")
            )
        ),
        new Rule(QUOTED_STRING,
            ClauseFactory::seq(ClauseFactory::c('"'),
                ClauseFactory::ast(QUOTED_STRING_AST,
                    ClauseFactory::zeroOrMore(ClauseFactory::ruleRef(STR_QUOTED_CHAR))
                ),
                ClauseFactory::c('"')
            )
        ),
        new Rule(STR_QUOTED_CHAR,
            ClauseFactory::first(ClauseFactory::ruleRef(ESCAPED_CTRL_CHAR),
                ClauseFactory::c('"', '\\').invert()
            )
        ),
        new Rule(HEX,
            ClauseFactory::c(ClauseFactory::cRange('0', '9'),
                ClauseFactory::cRange('a', 'f'),
                ClauseFactory::cRange('A', 'F')
            )
        ),
        new Rule(ESCAPED_CTRL_CHAR,
            ClauseFactory::first(ClauseFactory::str("\\t"),
                ClauseFactory::str("\\b"),
                ClauseFactory::str("\\n"),
                ClauseFactory::str("\\r"),
                ClauseFactory::str("\\f"),
                ClauseFactory::str("\\'"),
                ClauseFactory::str("\\\""),
                ClauseFactory::str("\\\\"),
                ClauseFactory::seq(ClauseFactory::str("\\u"), 
                    ClauseFactory::ruleRef(HEX),
                    ClauseFactory::ruleRef(HEX),
                    ClauseFactory::ruleRef(HEX),
                    ClauseFactory::ruleRef(HEX)
                )
            )
        ),
        new Rule(NOTHING,
            ClauseFactory::ast(NOTHING_AST,
                ClauseFactory::seq(ClauseFactory::c('('),
                    ClauseFactory::ruleRef(WSC),
                    ClauseFactory::c(')'))
            )
        ),
        new Rule(START,
            ClauseFactory::ast(START_AST,
                ClauseFactory::c('^')
            )
        )
    };
    static Grammar* grammar = new Grammar(NecessaryRules);

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
        if (subClause->TypeOfClause == TypesOfClauses::Terminal)
        {
            subClausePrec = clauseTypeToPrecedence[Terminal];
        }
        else
        {
            subClausePrec = clauseTypeToPrecedence[subClause.getClass()];
        }
        return subClausePrec < astNodeLabelPrec;
    }

    static Grammar* parse(string input) 
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
        auto topLevelRule = grammar->getRule(GRAMMAR);
        auto topLevelRuleASTNodeLabel = topLevelRule->labeledClause->astNodeLabel;
        if (topLevelRuleASTNodeLabel.empty()) 
        {
            topLevelRuleASTNodeLabel = "<root>";
        }
        auto topLevelMatches = grammar->getNonOverlappingMatches(GRAMMAR, memoTable);
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
        auto topLevelMatch = topLevelMatches[0];

        // TreeUtils.printTreeView(topLevelMatch, input);

        auto topLevelASTNode = new ASTNode(topLevelRuleASTNodeLabel, topLevelMatch, input);

        // System.out.println(topLevelASTNode);

        vector<Rule*> rules;
        for (ASTNode* astNode : topLevelASTNode->children) 
        {
            /*
            if (!astNode.label.equals(RULE_AST)) {
                throw new IllegalArgumentException("Wrong node type");
            }
            */
            Rule* rule = parseRule(astNode);
            rules.push_back(rule);
        }
        return new Grammar(rules);
    }
};
