#pragma once

#include <set>
#include "LabeledClause.hpp";
#include "Clause.hpp";
#include "RuleRef.hpp";
#include "First.hpp";
#include "Terminal.hpp";
#include "Rule.hpp";

class GrammarUtils
{
private:
    static void findTerminals(Clause* clause, set<Clause*> visited, vector<Clause*> terminalsOut) 
    {
        if (visited.insert(clause).second) 
        {
            if (clause->TypeOfClause == TypesOfClauses::Terminal) 
            {
                terminalsOut.push_back(clause);
            }
            else 
            {
                for (auto labeledSubClause : clause->labeledSubClauses) 
                {
                    auto subClause = labeledSubClause->clause;
                    findTerminals(subClause, visited, terminalsOut);
                }
            }
        }
    }

    static void findReachableClauses(Clause* clause, set<Clause*> visited, vector<Clause*> revTopoOrderOut) 
    {
        if (visited.insert(clause).second) 
        {
            for (auto labeledSubClause : clause->labeledSubClauses) 
            {
                auto subClause = labeledSubClause->clause;
                findReachableClauses(subClause, visited, revTopoOrderOut);
            }
            revTopoOrderOut.push_back(clause);
        }
    }

    static void findCycleHeadClauses(Clause* clause, set<Clause*> discovered, set<Clause*> finished, set<Clause*> cycleHeadClausesOut) 
    {
        if (clause->TypeOfClause == TypesOfClauses::RuleRef)
        {
            /*
            throw new IllegalArgumentException(
                "There should not be any " + RuleRef.class.getSimpleName() + " nodes left in grammar");
                */
        }
        /** Ќе совсем пон€тно что такое RuleRef, почему мы должны пробрасывать исключени€.*/
        discovered.insert(clause);
        for (auto labeledSubClause : clause->labeledSubClauses) 
        {
            auto subClause = labeledSubClause->clause;
            if (discovered.contains(subClause))
            {
                // Reached a cycle
                cycleHeadClausesOut.insert(subClause);
            }
            else if (!finished.contains(subClause))
            {
                findCycleHeadClauses(subClause, discovered, finished, cycleHeadClausesOut);
            }
        }
        discovered.erase(clause);
        finished.insert(clause);
    }

    static int countRuleSelfReferences(Clause* clause, string ruleNameWithoutPrecedence)
    {
        if (clause->TypeOfClause == TypesOfClauses::RuleRef && ((RuleRef*)clause)->refdRuleName == ruleNameWithoutPrecedence)
        {
            return 1;
        }
        else
        {
            auto numSelfRefs = 0;
            for (auto labeledSubClause : clause->labeledSubClauses)
            {
                auto subClause = labeledSubClause->clause;
                numSelfRefs += countRuleSelfReferences(subClause, ruleNameWithoutPrecedence);
            }
            return numSelfRefs;
        }
    }

    static int rewriteSelfReferences(Clause* clause, Rule::Associativity associativity, int numSelfRefsSoFar, int numSelfRefs, string selfRefRuleName, bool isHighestPrec, string currPrecRuleName, string nextHighestPrecRuleName) 
    {
        if (numSelfRefsSoFar < numSelfRefs) 
        {
            for (int i = 0; i < clause->labeledSubClauses.size(); i++)
            {
                auto labeledSubClause = clause->labeledSubClauses[i];
                auto subClause = labeledSubClause->clause;
                if (subClause->TypeOfClause == TypesOfClauses::RuleRef) {
                    if (((RuleRef*)subClause)->refdRuleName == selfRefRuleName) 
                    {
                        if (numSelfRefs >= 2) 
                        {
                            // Change name of self-references to implement precedence climbing:
                            // For leftmost operand of left-recursive rule:
                            // E[i] <- E X E  =>  E[i] = E[i] X E[i+1]
                            // For rightmost operand of right-recursive rule:
                            // E[i] <- E X E  =>  E[i] = E[i+1] X E[i]
                            // For non-associative rule:
                            // E[i] = E E  =>  E[i] = E[i+1] E[i+1]
                            RuleRef X((associativity == Rule::Associativity::LEFT && numSelfRefsSoFar == 0) || (associativity == Rule::Associativity::RIGHT && numSelfRefsSoFar == numSelfRefs - 1)
                                ? currPrecRuleName
                                : nextHighestPrecRuleName);
                            clause->labeledSubClauses[i]->clause = &X;
                        }
                        else /* numSelfRefs == 1 */ 
                        {
                            if (!isHighestPrec) 
                            {
                                // Move subclause (and its AST node label, if any) inside a First clause that
                                // climbs precedence to the next level:
                                // E[i] <- X E Y  =>  E[i] <- X (E[i] / E[(i+1)%N]) Y
                                ((RuleRef*)subClause)->refdRuleName = currPrecRuleName;
                                auto Y = new RuleRef(nextHighestPrecRuleName);
                                vector<Clause*> Z {subClause, Y};
                                First X(Z);
                                clause->labeledSubClauses[i]->clause = &X;

                                /* “ут странный момент, у них конструктор First определЄн от Clause, но они в него суют и RuleRef. ѕочему так можно.*/
                            }
                            else
                            {
                                // Except for highest precedence, just defer back to lowest-prec level:
                                // E[N-1] <- '(' E ')'  =>  E[N-1] <- '(' E[0] ')'        
                                ((RuleRef*)subClause)->refdRuleName = nextHighestPrecRuleName;
                            }
                        }
                        numSelfRefsSoFar++;
                    }
                    // Else don't rewrite the RuleRef, it is not a self-ref
                }
                else
                {
                    numSelfRefsSoFar = rewriteSelfReferences(subClause, associativity, numSelfRefsSoFar,
                        numSelfRefs, selfRefRuleName, isHighestPrec, currPrecRuleName, nextHighestPrecRuleName);
                }
                subClause->toStringCached = "";
            }
        }
        return numSelfRefsSoFar;
    }

    
public:
    static vector<Clause*> findClauseTopoSortOrder(Rule* topLevelRule, vector<Rule*> allRules, vector<Clause*> lowestPrecedenceClauses)
    {
        vector<Clause*> allClausesUnordered;
        set<Clause*> topLevelVisited;

        if (topLevelRule != nullptr)
        {
            allClausesUnordered.push_back(topLevelRule->labeledClause->clause);
            topLevelVisited.insert(topLevelRule->labeledClause->clause);
        }

        for (auto rule : allRules)
        {
            findReachableClauses(rule->labeledClause->clause, topLevelVisited, allClausesUnordered);
        }
        /**“ут тупо занесли в ќл лозесјнордеред все клозы, в которые вообще можно прийти при парсинге грамматики*/
        auto topLevelClauses = allClausesUnordered; // Ќа вс€кий случай спросить на семинаре.
        for (auto clause : allClausesUnordered)
        {
            for (auto labeledSubClause : clause->labeledSubClauses)
            {
                topLevelClauses.erase(remove(topLevelClauses.begin(), topLevelClauses.end(), labeledSubClause->clause));
            }
        }
        /** “ут, в начале занесли все клозы в топлевеледклозес, а потом удалили все, у которых есть родители.*/
        auto dfsRoots = topLevelClauses;

        for (auto x : lowestPrecedenceClauses)
        {
            dfsRoots.push_back(x);
        }

        /** ƒобавл€ем в dfsRoots какие то клозы, непон€тно что такое lowestPrecedence, какой то входной параметр пока что*/

        set<Clause*> cycleDiscovered;
        set<Clause*> cycleFinished;
        set<Clause*> cycleHeadClauses;

        for (auto clause : topLevelClauses)
        {
            findCycleHeadClauses(clause, cycleDiscovered, cycleFinished, cycleHeadClauses);
        }
        /** ƒл€ каждого хэдклоза из грамматики нашли список всех вершин, в которых начинаютс€ циклы*/
        for (auto rule : allRules)
        {
            findCycleHeadClauses(rule->labeledClause->clause, cycleDiscovered, cycleFinished, cycleHeadClauses);
        }
        /** «ачем то делаем то же самое дл€ всех правил.*/
        for (auto x : cycleHeadClauses)
        {
            dfsRoots.push_back(x);
        }
        /**«акидываем все вершины, €вл€ющиес€ началоми циклов в список dfsRoots.*/

        set<Clause*> terminalsVisited;
        vector<Clause*> terminals;
        for (auto rule : allRules)
        {
            findTerminals(rule->labeledClause->clause, terminalsVisited, terminals);
        }
        /** ѕосле этого в terminals будут лежать все терминалы, достижимые в наших правилах.*/
        vector<Clause*> allClauses(terminals);
        set<Clause*> reachableVisited;
        for (auto x : terminals)
        {
            reachableVisited.insert(x);
        }

        for (auto topLevelClause : dfsRoots)
        {
            findReachableClauses(topLevelClause, reachableVisited, allClauses);
        }
        /** ¬ allClauses закидываем все достижимые классы, правда почему то не из топлевелклозов а из dfsRoots, правда наличие visited не даЄт создавать дубликаты.*/

        for (int i = 0; i < allClauses.size(); i++)
        {
            allClauses[i]->clauseIdx = i;
        }
        /** ѕочему то в allClauses классы располагаютс€ в пор€дке увеличени€ индекса.*/
        return allClauses;
    }

    static void checkNoRefCycles(Clause* clause, string selfRefRuleName, set<Clause*> visited) 
    {
        if (visited.insert(clause).second) 
        {
            for (auto labeledSubClause : clause->labeledSubClauses) 
            {
                auto subClause = labeledSubClause->clause;
                checkNoRefCycles(subClause, selfRefRuleName, visited);
            }
        }
        else 
        {
            /*
            throw new IllegalArgumentException(
                "Rules should not contain cycles when they are created: " + selfRefRuleName);
                */
        }
        visited.erase(clause);
    }

    static void handlePrecedence(string ruleNameWithoutPrecedence, vector<Rule*> rules, vector<Clause*> lowestPrecedenceClauses, map<string, string> ruleNameToLowestPrecedenceLevelRuleName) 
    {
        // Rewrite rules
        // 
        // For all but the highest precedence level:
        //
        // E[0] <- E (Op E)+  =>  E[0] <- (E[1] (Op E[1])+) / E[1] 
        // E[0,L] <- E Op E   =>  E[0] <- (E[0] Op E[1]) / E[1] 
        // E[0,R] <- E Op E   =>  E[0] <- (E[1] Op E[0]) / E[1]
        // E[3] <- '-' E      =>  E[3] <- '-' (E[3] / E[4]) / E[4]
        //
        // For highest precedence level, next highest precedence wraps back to lowest precedence level:
        //
        // E[5] <- '(' E ')'  =>  E[5] <- '(' E[0] ')'

        // Check there are no duplicate precedence levels
        unordered_map<int, Rule*> precedenceToRule;
        for (auto rule : rules) 
        {
            
            if (!precedenceToRule.insert({ rule->precedence, rule }).second)
            {
                cout << "Multiple rules with name " + ruleNameWithoutPrecedence
                    + (rule->precedence == -1 ? "" : " and precedence " + to_string(rule->precedence));
                abort();
            }
            // ¬ыше функци€ работает следующим образом, пытаетс€ засунуть в меп пару ключ значение, если такой ключ уже был то возвращает его значение, если его не было то заносит и возвращает null, пока не €сно, нужно нам это или нет.
        }
        // Get rules in ascending order of precedence
        vector<Rule*> precedenceOrder;
        for (auto &x : precedenceToRule)
        {
            precedenceOrder.push_back(x.second);
        }

        // Rename rules to include precedence level
        auto numPrecedenceLevels = rules.size();
        for (int precedenceIdx = 0; precedenceIdx < numPrecedenceLevels; precedenceIdx++) 
        {
            // Since there is more than one precedence level, update rule name to include precedence
            auto rule = precedenceOrder[precedenceIdx];
            rule->ruleName += "[" + to_string(rule->precedence) + "]";
        }

        // Transform grammar rule to handle precence
        for (int precedenceIdx = 0; precedenceIdx < numPrecedenceLevels; precedenceIdx++) {
            auto rule = precedenceOrder[precedenceIdx];

            // Count the number of self-reference operands
            auto numSelfRefs = countRuleSelfReferences(rule->labeledClause->clause, ruleNameWithoutPrecedence);

            auto currPrecRuleName = rule->ruleName;
            auto nextHighestPrecRuleName = precedenceOrder[(precedenceIdx + 1) % numPrecedenceLevels]->ruleName;

            // If a rule has 1+ self-references, need to rewrite rule to handle precedence and associativity
            auto isHighestPrec = precedenceIdx == numPrecedenceLevels - 1;
            if (numSelfRefs >= 1) {
                // Rewrite self-references to higher precedence or left- and right-recursive forms.
                // (the toplevel clause of the rule, rule.labeledClause.clause, can't be a self-reference,
                // since we already checked for that, and IllegalArgumentException would have been thrown.)
                rewriteSelfReferences(rule->labeledClause->clause, rule->associativity, 0, numSelfRefs,
                    ruleNameWithoutPrecedence, isHighestPrec, currPrecRuleName, nextHighestPrecRuleName);
            }

            // Defer to next highest level of precedence if the rule doesn't match, except at the highest level of
            // precedence, which is assumed to be a precedence-breaking pattern (like parentheses), so should not
            // defer back to the lowest precedence level unless the pattern itself matches
            if (!isHighestPrec) {
                // Move rule's toplevel clause (and any AST node label it has) into the first subclause of
                // a First clause that fails over to the next highest precedence level
                auto first = new First(vector<Clause*> { rule->labeledClause->clause, new RuleRef(nextHighestPrecRuleName) });
                // Move any AST node label down into first subclause of new First clause, so that label doesn't
                // apply to the final failover rule reference
                first->labeledSubClauses[0]->astNodeLabel = rule->labeledClause->astNodeLabel;
                rule->labeledClause->astNodeLabel = "";
                // Replace rule clause with new First clause
                rule->labeledClause->clause = first;
            }
        }

        // Map the bare rule name (without precedence suffix) to the lowest precedence level rule name
        auto lowestPrecRule = precedenceOrder[0];
        lowestPrecedenceClauses.push_back(lowestPrecRule->labeledClause->clause);
        ruleNameToLowestPrecedenceLevelRuleName[ruleNameWithoutPrecedence] = lowestPrecRule->ruleName;
    }

    /**
 * Recursively call toString() on the clause tree for this {@link Rule}, so that toString() values are cached
 * before {@link RuleRef} objects are replaced with direct references, and so that shared subclauses are only
 * matched once.
 */
    static Clause* intern(Clause* clause, map<string, Clause*> toStringToClause) {
        // Call toString() on (and intern) subclauses, bottom-up
        for (int i = 0; i < clause->labeledSubClauses.size(); i++) {
            clause->labeledSubClauses[i]->clause = intern(clause->labeledSubClauses[i]->clause, toStringToClause);
        }
        // Call toString after recursing to child nodes
        string toStr = clause->toString();

        // Intern the clause based on the toString value
        auto prevInternedClause = toStringToClause.try_emplace(toStr, clause);

        // Return the previously-interned clause, if present, otherwise the clause, if it was just interned
        return prevInternedClause.second  ? prevInternedClause.first->second : clause;
    }

    // -------------------------------------------------------------------------------------------------------------

    /** Resolve {@link RuleRef} clauses to a reference to the named rule. */
    static void resolveRuleRefs(LabeledClause* labeledClause, map<string, Rule*> ruleNameToRule,
        map<string, string> ruleNameToLowestPrecedenceLevelRuleName, set<Clause*> visited) {
        if (labeledClause->clause->TypeOfClause == TypesOfClauses::RuleRef) {
            // Follow a chain of from name in RuleRef objects until a non-RuleRef is reached
            auto currLabeledClause = labeledClause;
            set<Clause*> visitedClauses;
            while (currLabeledClause->clause->TypeOfClause == TypesOfClauses::RuleRef) {
                if (!visitedClauses.insert(currLabeledClause->clause).second) {
                        cout << "Reached toplevel RuleRef cycle: " + currLabeledClause->clause->toString();
                        abort();
                }
                // Follow a chain of from name in RuleRef objects until a non-RuleRef is reached
                auto refdRuleName = ((RuleRef*)currLabeledClause-> clause)->refdRuleName;

                // Check if the rule is the reference to the lowest precedence rule of a precedence hierarchy
                auto lowestPrecRuleName = ruleNameToLowestPrecedenceLevelRuleName[refdRuleName];

                // Look up Rule based on rule name
                auto refdRule = ruleNameToRule[lowestPrecRuleName.empty() ? refdRuleName : lowestPrecRuleName];
                if (refdRule == nullptr) {
                    cout << "Unknown rule name: " + refdRuleName;
                    abort();
                }
                currLabeledClause = refdRule->labeledClause;
            }

            // Set current clause to a direct reference to the referenced rule
            labeledClause->clause = currLabeledClause->clause;

            // Copy across AST node label, if any
            if (labeledClause->astNodeLabel.empty()) {
                labeledClause->astNodeLabel = currLabeledClause->astNodeLabel;
            }
            // Stop recursing at RuleRef
        }
        else {
            if (visited.insert(labeledClause->clause).second) {
                auto labeledSubClauses = labeledClause->clause->labeledSubClauses;
                for (auto subClauseIdx = 0; subClauseIdx < labeledSubClauses.size(); subClauseIdx++) {
                    auto labeledSubClause = labeledSubClauses[subClauseIdx];
                    // Recurse through subclause tree if subclause was not a RuleRef 
                    resolveRuleRefs(labeledSubClause, ruleNameToRule, ruleNameToLowestPrecedenceLevelRuleName,
                        visited);
                }
            }
        }
    }

};