/*
 * gecodesolver.cpp
 *
 *  Created on: Dec 3, 2012
 *      Author: faeton
 */

#include <string>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "casp/gecodesolver.h"

using namespace Gecode;

GecodeSolver::GecodeSolver(std::string expression) {
	SimpleParser *parser = new SimpleParser;

	char * expr = new char[expression.size() + 1];
	std::copy(expression.begin(), expression.end(), expr);
	expr[expression.size()] = '\0';

	parser->setexpr(expr);
	parser->convert();

	ParseTree* tree = parser->makeTree();
	IntVar* leftExpression = makeExpression(tree->leftOperand);
	IntVar* rightExpression = makeExpression(tree->rightOperand);

	if (tree->value == '=')
		rel(*this, *leftExpression, IRT_EQ, *rightExpression);
	else
		rel(*this, *leftExpression, IRT_EQ, *rightExpression);
}

IntVar* GecodeSolver::makeExpression(ParseTree* tree) {
	Gecode::IntVar* res;
	if (isdigit(tree->value)) {
		int value = tree->value - '0';
		res = new Gecode::IntVar(*this, value, value);
		branch(*this, *res, Gecode::INT_VAL_RND);
	}
	return res;
}
