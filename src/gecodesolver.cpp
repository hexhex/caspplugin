/*
 * gecodesolver.cpp
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

using namespace Gecode;
using namespace std;

GecodeSolver::GecodeSolver(vector<string> expressions, string domain, string globalConstraintName, string globalConstraintValue) {
	if (domain == "") domain = "1..10";

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep("\".");
	tokenizer tokens(domain, sep);
	tokenizer::iterator tok_iter = tokens.begin();

	string stringMinValue = *tok_iter++;
	string stringMaxValue = *tok_iter++;

	minValue = atoi(stringMinValue.c_str());
	maxValue = atoi(stringMaxValue.c_str());

	SimpleParser parser;

	for (vector<string>::iterator it = expressions.begin(); it != expressions.end(); it++) {
		string expression = *it;

		ParseTree* tree;
		parser.makeTree(expression, &tree);

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

		parser.deleteTree(tree);
	}

	if (globalConstraintName != "") {
		ParseTree* tree;
		parser.makeTree(globalConstraintValue, &tree);
		LinExpr globalConstraintExpression = makeExpression(tree);

		if (globalConstraintName == "maximize")
			costVariable = expr(*this, globalConstraintExpression);
		if (globalConstraintName == "minimize")
			costVariable = expr(*this, -globalConstraintExpression);
	}
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

		map<string,Gecode::IntVar>::iterator it = constraintVariables.find(variableName);
		if (it == constraintVariables.end()) { // create fresh variable
			Gecode::IntVar var(*this, minValue, maxValue);
			constraintVariables[variableName] = var;
			return var;
		}
		else { // reuse old variable
			return it->second;
		}
	}
	else throw "Unknown node type";
}
