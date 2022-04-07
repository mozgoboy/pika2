#pragma once
#include "Terminal.hpp"
#include "Start.hpp"
#include "Nothing.hpp"
#include "CharSeq.hpp"
#include "CharSet.hpp"
#include "Rule.hpp"
#include "FollowedBy.hpp"
#include "Seq.hpp"
#include "Clause.hpp"

class ClauseFactory
{
private:
public:
    static Rule* rule(string ruleName, Clause* clause) 
    {
        Rule rule(ruleName, -1, Rule::Associativity::NONE, clause);
        return &rule;
    }

    static Rule* rule(string ruleName, int precedence, Rule::Associativity associativity, Clause* clause)
    {
        Rule rule(ruleName, precedence, associativity, clause);
        return &rule;
    }

    static Clause* seq(vector<Clause*> subClauses) 
    {
        Seq sequence(subClauses);
        return &sequence;
    }

    static Clause* oneOrMore(Clause* subClause)
    {
        
        if (subClause->TypeOfClause == TypesOfClauses::OneOrMore || subClause->TypeOfClause == TypesOfClauses::Nothing || subClause->TypeOfClause == TypesOfClauses::FollowedBy
            || subClause->TypeOfClause == TypesOfClauses::NotFollowedBy || subClause->TypeOfClause == TypesOfClauses::Start)
        {
            return subClause;
        }
        OneOrMore oom(subClause);
        return (Clause*)&oom;
    }

    static Clause* optional(Clause* subClause)
    {
        vector<Clause*> X;
        X.push_back(subClause);
        X.push_back(nothing());
        return first(X);
    }

    static Clause* zeroOrMore(Clause* subClause)
    {
        return optional(oneOrMore(subClause));
    }

    static Clause* first(vector<Clause*> subClauses)
    {
        First X(subClauses);
        return &X;
    }

    static Clause* followedBy(Clause* subClause)
    {
        if (subClause->TypeOfClause == TypesOfClauses::Nothing)
        {
            return subClause;
        }
        else if (subClause->TypeOfClause == TypesOfClauses::FollowedBy || subClause->TypeOfClause == TypesOfClauses::NotFollowedBy
            || subClause->TypeOfClause == TypesOfClauses::Start)
        {
            /*
            throw new IllegalArgumentException(FollowedBy.class.getSimpleName() + "("
                + subClause.getClass().getSimpleName() + ") is nonsensical");
                 Пробрасываем исключение
                 */
        }
        FollowedBy X(subClause);
        return &X;
    }

    static Clause* notFollowedBy(Clause* subClause)
    {
        if (subClause->TypeOfClause == TypesOfClauses::Nothing)
        {
            /*
            throw new IllegalArgumentException(NotFollowedBy.class.getSimpleName() + "("
                + Nothing.class.getSimpleName() + ") will never match anything");
                Пробрасываем исключение
                */
        }
        else if (subClause->TypeOfClause == TypesOfClauses::NotFollowedBy)
        {
            FollowedBy X(subClause->labeledSubClauses[0]->clause);
            return &X;
        }
        else if (subClause->TypeOfClause == TypesOfClauses::FollowedBy || subClause->TypeOfClause == TypesOfClauses::Start)
        {
            /*
            throw new IllegalArgumentException(NotFollowedBy.class.getSimpleName() + "("
                + subClause.getClass().getSimpleName() + ") is nonsensical");
            Пробрасываем исключение
                */
        }
        NotFollowedBy X(subClause);
        return (Clause*) &X;
    }

    static Clause* start()
    {
        Start X;
        return &X;
    }

    static Clause* nothing()
    {
        Nothing X;
        return &X;
    }

    static Clause* str(string str)
    {
        if (str.length() == 1) 
        {
            vector<char> X;
            X.push_back(str[0]);
            return c(X);
        }
        else 
        {
            CharSeq X(str, /* ignoreCase = */ false);
            return &X;
        }
    }

    static CharSet* c(vector<char> chrs)
    {
        return new CharSet(chrs);
    }

    static CharSet* cInStr(string str)
    {
        vector<char> data(str.begin(), str.end());
        return new CharSet(data);
    }

    static CharSet* cRange(char minChar, char maxChar)
    {
        if (maxChar < minChar) 
        {
            /*
            throw new IllegalArgumentException("maxChar < minChar");
            */
        }
        bitset<256> bs;
        bs.set(minChar, maxChar + 1);
        return new CharSet(bs);
    }

    static CharSet* cRange(string charRangeStr)
    {
        bool invert = charRangeStr[0] == '^';
        auto charList = StringUtils::getCharRangeChars(invert ? charRangeStr.substr(1) : charRangeStr); // Пока не совсем понял что эта функция делает
        auto chars = new bitset<256>;
        for (int i = 0; i < charList.size(); i++) {
            auto c = charList[i];
            if (c.length() == 2) 
            {
                // Unescape \^, \-, \], \\
                c = c.substring(1);
            }
            auto c0 = c[0];
            if (i <= charList.size() - 3 && charList[i + 1] == "-") 
            {
                auto cEnd = charList[i + 2];
                if (cEnd.length() == 2) 
                {
                    cEnd = cEnd.substr(1);
                }
                auto cEnd0 = cEnd[0];
                if (cEnd0 < c0) 
                {
                    /*
                    throw new IllegalArgumentException("Char range limits out of order: " + c0 + ", " + cEnd0);
                    */
                }
                chars->set(c0, cEnd0 + 1);
                i += 2;
            }
            else 
            {
                chars->set(c0);
            }
        }
        if (invert)
        {
            CharSet* X = new CharSet(*chars);
            return X->invert();
        }
        else
        {;
            return new CharSet(*chars);
        }
    }

    static CharSet* c(vector<CharSet*> charSets) 
    {
        return new CharSet(charSets);
    }

    static Clause* ast(string astNodeLabel, Clause* clause) 
    {
        return new ASTNodeLabel(astNodeLabel, clause);
    }

    static Clause* ruleRef(string ruleName) 
    {
        return new RuleRef(ruleName);
    }
};