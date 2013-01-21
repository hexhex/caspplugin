/*
R * gecodesolver.cpp
 *
 *  Created on: Dec 3, 2012
 *      Author: faeton
 */

#include <string>
#include <map>

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "casp/gecodesolver.h"

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

using namespace Gecode;
using namespace std;

GecodeSolver::GecodeSolver(vector<string> sumData, string domain,
		string globalConstraintName, string globalConstraintValue, boost::shared_ptr<SimpleParser> simpleParser) :
			_simpleParser(simpleParser) {

	if (domain == "") domain = "1..10";

	_sumData = sumData;

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep("\".");
	tokenizer tokens(domain, sep);
	tokenizer::iterator tok_iter = tokens.begin();

	string stringMinValue = *tok_iter++;
	string stringMaxValue = *tok_iter++;

	_minValue = atoi(stringMinValue.c_str());
	_maxValue = atoi(stringMaxValue.c_str());

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

// TODO: cache parsed trees
void GecodeSolver::propagate(string expression) {
	// If the expression is not specified, ignore it
	// This could be the case for learning
	if (expression == "") return;

	ParseTree* tree;
	_simpleParser->makeTree(expression, &tree);

	LinExpr leftExpression = makeExpression(tree->left);
	LinExpr rightExpression = makeExpression(tree->right);

	if (tree->value == "==")
		rel(*this, leftExpression == rightExpression);
	else if (tree->value == "!=")
		rel(*this, leftExpression != rightExpression);
	else if (tree->value == ">")
		rel(*this, leftExpression > rightExpression);
	else if (tree->value == "<")
		rel(*this, leftExpression < rightExpression);
	else if (tree->value == ">=")
		rel(*this, leftExpression >= rightExpression);
	else if (tree->value == "<=")
		rel(*this, leftExpression <= rightExpression);

	_simpleParser->deleteTree(tree);
}

Gecode::LinExpr GecodeSolver::makeExpression(ParseTree* tree) {
	if (tree->type == OPERATOR) {
		LinExpr leftVariable = makeExpression(tree->left);
		LinExpr rightVariable = makeExpression(tree->right);

		string op = tree->value;

		if (op == "+") return leftVariable + rightVariable;
		else if (op == "-") return leftVariable - rightVariable;
		else if (op == "*") return leftVariable * rightVariable;
		else if (op == "/") return leftVariable / rightVariable;
		else throw "Invalid operator";
	}
	else if(tree->type == NUMBER) {
		int value = atoi(tree->value.c_str());

		Gecode::IntVar var(*this, value, value);
		return var;
	}
	else if(tree->type == CONSTRAINT_VARIABLE) {
		string variableName = tree->value;

		if (boost::starts_with(variableName, "sum_")) {
			string aggregatedPredicate = variableName.substr(4);

			LinExpr result = expr(*this, 1==1);
			ParseTree* sumTree;
			for (int i = 0; i < _sumData.size(); i++) {
				string s = _sumData[i];
				_simpleParser->makeTree(s, &sumTree);

				result = result + makeExpression(sumTree);
				_simpleParser->deleteTree(sumTree);
			}
			return result;
		}
		else {
			map<string,Gecode::IntVar>::iterator it = _constraintVariables.find(variableName);
			if (it == _constraintVariables.end()) { // create fresh variable
				Gecode::IntVar var(*this, _minValue, _maxValue);
				_constraintVariables[variableName] = var;
				return var;
			}
			else { // reuse old variable
				return it->second;
			}
		}
	}
	else throw "Unknown node type";
}
