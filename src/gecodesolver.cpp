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

GecodeSolver::GecodeSolver(std::vector<std::string> expressions, std::string domain) {
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep("\".");
	tokenizer tokens(domain, sep);
	tokenizer::iterator tok_iter = tokens.begin();

	std::string stringMinValue = *tok_iter++;
	std::string stringMaxValue = *tok_iter++;

	minValue = atoi(stringMinValue.c_str());
	maxValue = atoi(stringMaxValue.c_str());

	SimpleParser parser;

	for (std::vector<std::string>::iterator it = expressions.begin(); it != expressions.end(); it++) {
		std::string expression = *it;

		char * expr = new char[expression.size() - 1];
		std::copy(expression.begin() + 1, expression.end() - 1, expr);
		expr[expression.size() - 2] = '\0';

		ParseTree* tree;
		parser.makeTree(expr, &tree);

		LinExpr leftExpression = makeExpression(tree->left);
		LinExpr rightExpression = makeExpression(tree->right);

		if (tree->value[0] == '=')
			rel(*this, leftExpression == rightExpression);
		else if (tree->value[0] == '>')
			rel(*this, leftExpression > rightExpression);
		else if (tree->value[0] == '<')
			rel(*this, leftExpression < rightExpression);
	}
}

Gecode::LinExpr GecodeSolver::makeExpression(ParseTree* tree) {
	if (tree->type == OPERATOR) {
		LinExpr leftVariable = makeExpression(tree->left);
		LinExpr rightVariable = makeExpression(tree->right);

		char op = tree->value[0];

		if (op == '+') return leftVariable + rightVariable;
		else if (op == '-') return leftVariable - rightVariable;
		else if (op == '*') return leftVariable * rightVariable;
		else if (op == '/') return leftVariable / rightVariable;
		else throw "Invalid operator";
	}
	else if(tree->type == NUMBER) {
		int value = atoi(tree->value);

		Gecode::IntVar var(*this, value, value);
		return var;
	}
	else if(tree->type == CONSTRAINT_VARIABLE) {
		std::string variableName = tree->value;

		std::map<std::string,Gecode::IntVar>::iterator it = constraintVariables.find(variableName);
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
