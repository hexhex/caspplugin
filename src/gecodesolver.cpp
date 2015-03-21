#include <string>
#include <map>

#include <dlvhex2/Error.h>

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "casp/gecodesolver.h"

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

using namespace Gecode;
using namespace std;

GecodeSolver::GecodeSolver(vector<string> sumData, int domainMinValue,int domainMaxValue, string globalConstraintName, string globalConstraintValue,
		boost::shared_ptr<SimpleParser> simpleParser) :
		_simpleParser(simpleParser) {

	_sumData = sumData;

	_minValue = domainMinValue;
	_maxValue = domainMaxValue;

	if (globalConstraintName != "") {
		ParseTree* tree;
		_simpleParser->makeTree(globalConstraintValue, &tree);
		LinExpr globalConstraintExpression = makeExpression(tree);

		if (globalConstraintName == "maximize")
			_costVariable = expr(*this, globalConstraintExpression);
		if (globalConstraintName == "minimize")
			_costVariable = expr(*this, -globalConstraintExpression);

		_simpleParser->deleteTree(tree);
	}
}

void GecodeSolver::propagate(vector<string> expressions) {
	BOOST_FOREACH(string expression, expressions) {
		propagate(expression);
	}
}

Gecode::LinRel makeMainRelation(const Gecode::LinExpr& leftExpression, const Gecode::LinExpr& rightExpression, const string& value) {
	if (value == "==")
		return LinRel(leftExpression == rightExpression);
	else if (value == "!=")
		return LinRel(leftExpression != rightExpression);
	else if (value == ">")
		return LinRel(leftExpression > rightExpression);
	else if (value == "<")
		return LinRel(leftExpression < rightExpression);
	else if (value == ">=")
		return LinRel(leftExpression >= rightExpression);
	else if (value == "<=")
		return LinRel(leftExpression <= rightExpression);
	else
		throw dlvhex::PluginError("Unknown operator: " + value);
}

void GecodeSolver::propagate(string expression) {
	// If the expression is not specified, ignore it
	// This could be the case for learning
	if (expression == "")
		return;

	ParseTree* tree;
	_simpleParser->makeTree(expression, &tree);

	const LinExpr& leftExpression = makeExpression(tree->left);
	const LinExpr& rightExpression = makeExpression(tree->right);

	const LinRel& mainRelation = makeMainRelation(leftExpression, rightExpression, tree->value);

	rel(*this, mainRelation);

	_simpleParser->deleteTree(tree);
}

Gecode::LinExpr GecodeSolver::makeExpression(ParseTree* tree) {
	if (tree->type == OPERATOR) {
		LinExpr leftExpression = makeExpression(tree->left);
		LinExpr rightExpression = makeExpression(tree->right);

		string op = tree->value;

		if (op == "+")
			return leftExpression + rightExpression;
		else if (op == "-")
			return leftExpression - rightExpression;
		else if (op == "*")
			return leftExpression * rightExpression;
		else if (op == "/")
			return leftExpression / rightExpression;
		else
			throw "Invalid operator";
	} else if (tree->type == NUMBER) {
		int value = atoi(tree->value.c_str());

//		Gecode::IntVar res(*this, value, value);
//		return res;

		return expr(*this, value);
	} else if (tree->type == CONSTRAINT_VARIABLE) {
		string variableName = tree->value;

		if (boost::starts_with(variableName, "sum_")) {
			int index1 = variableName.find('_', 0);
			int index2 = variableName.find('_', index1 + 1);
			string predicate = variableName.substr(index1 + 1, index2 - index1 - 1);
			int position = atoi(variableName.substr(index2 + 1, variableName.length() - index2).c_str());

			LinExpr result = expr(*this, 0);
			for (int i = 0; i < _sumData.size(); i++) {
				string s = _sumData[i];

				boost::char_separator<char> sep(",() ", "", boost::drop_empty_tokens);

				boost::tokenizer<boost::char_separator<char> > tokens(s, sep);

				int ind = 0;
				for (boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
					string token = *it;

					if (ind == 0 && token != predicate)
						break;
					else if (ind == position) {
						result = result + expr(*this, atoi(token.c_str()));
						break;
					}
					ind++;
				}
			}
			return result;
		} else {
			map<string, Gecode::IntVar>::iterator it = _constraintVariables.find(variableName);
			if (it == _constraintVariables.end()) { // create fresh variable
				Gecode::IntVar var(*this, _minValue, _maxValue);
				_constraintVariables[variableName] = var;
				branch(*this, var, INT_VAL_RANGE_MIN);
				return var;
			} else { // reuse old variable
				return it->second;
			}
		}
	} else
		throw "Unknown node type";
}
