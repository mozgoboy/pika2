#include "Clause.hpp"
#include "LabeledClause.hpp"
#include "MetaGrammar.hpp"

Clause::Clause(vector<Clause*> subClauses)
{
	if (subClauses.size() > 0 && subClauses[0]->TypeOfClause == TypesOfClauses::Nothing) {
		// Nothing can't be the first subclause, since we don't trigger upwards expansion of the DP wavefront
		// by seeding the memo table by matching Nothing at every input position, to keep the memo table small
		cout << "Nothing cannot be the first subclause of any clause";
		abort();
	}
	this->labeledSubClauses.clear();
	for (int i = 0; i < subClauses.size(); i++)
	{
		Clause* subClause = subClauses[i];
		string astNodeLabel;
		if (subClause->TypeOfClause == TypesOfClauses::ASTNodeLabel)
		{
			// Transfer ASTNodeLabel.astNodeLabel to LabeledClause.astNodeLabel field
			astNodeLabel = ((ASTNodeLabel*)subClause)->astNodeLabel;
			// skip over ASTNodeLabel node when adding subClause to subClauses array
			subClause = subClause->labeledSubClauses[0]->clause;
		}
		LabeledClause X(subClause, astNodeLabel);
		this->labeledSubClauses.push_back(&X);
	}
}

string Clause::getRuleNames() {
	string buf;
	for (auto rule : rules)
	{
		buf += rule->ruleName + ", ";
	}
	buf.pop_back();
	buf.pop_back();
	return buf;
}

string Clause::toStringWithRuleNames()
{
	string buf;
	if (toStringWithRuleNameCached.empty()) {
		if (not rules.empty()) {
			// Add rule names
			buf.append(getRuleNames());
			buf.append(" <- ");
			// Add any AST node labels
			bool addedASTNodeLabels = false;
			for (int i = 0; i < rules.size(); i++) {
				Rule* rule = rules[i];
				if (not rule->labeledClause->astNodeLabel.empty()) {
					buf.append(rule->labeledClause->astNodeLabel + ":");
					addedASTNodeLabels = true;
				}
			}
			bool addParens = addedASTNodeLabels && MetaGrammar::needToAddParensAroundASTNodeLabel(this);
			if (addParens) {
				buf.append("(");
			}
			buf.append(toString());
			if (addParens) {
				buf.append(")");
			}
			toStringWithRuleNameCached = buf;
		}
		else {
			toStringWithRuleNameCached = buf;
		}
	}
	return toStringWithRuleNameCached;
}