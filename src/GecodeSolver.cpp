/* CASPPlugin -- Pluging for dlvhex - Answer-Set Programming with constraint programming
 * Copyright (C) 2013 Oleksandr Stashuk
 * Copyright (C) 2015 Alessandro Francesco De Rosis
 *
 * This file is part of caspplugin.
 *
 * CASPPlugin is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * CASPPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#include <string>
#include <map>

#include <dlvhex2/Error.h>

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include "casp/GecodeSolver.h"

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <dlvhex2/Printer.h>

using namespace Gecode;
using namespace std;

GecodeSolver::GecodeSolver(dlvhex::RegistryPtr reg,vector<dlvhex::OrdinaryAtom>& sumElement, int domainMinValue,int domainMaxValue, string globalConstraintName, string globalConstraintValue,
		boost::shared_ptr<SimpleParser>& simpleParser) :
		_reg(reg),
		_simpleParser(simpleParser) {

	_sumElement = sumElement;

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

void GecodeSolver::propagate(vector<string>& expressions) {
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

void GecodeSolver::propagate(string& expression) {
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
			for (int i = 0; i < _sumElement.size(); i++) {
				dlvhex::OrdinaryAtom atom=_sumElement[i];

				if(_reg->getTermStringByID(atom.tuple[1])==predicate && atom.tuple[2].address==position)
				{
					result = result+expr(*this,atom.tuple[3].address);
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
