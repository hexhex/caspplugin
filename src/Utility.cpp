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
#include "casp/Utility.h"

#include <string>
#include <vector>
#include <utility>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
using namespace std;

/**
 * @brief This helper method is used to check whether string is a ComparisonOperator
 */
bool isComparisonOperator(string s)
{
	s=removeQuotes(s);
	return s=="=="||s==">="||s=="<="||s==">"||s=="<";
}

/**
 * @brief This helper method is used to check whether string is a variable
 */
bool isVariable(string s) {
	bool res = true;
	if (s[0] >= 'A' && s[0] <= 'Z') {
		for (int i = 1; i < s.length(); i++) {
			if (!isalnum(s[i]) && s[i] != '_') {
				res = false;
				break;
			}
		}
	} else
		res = false;
	return res;
}

string removeQuotes(string s) {
	return s.substr(1, s.length() - 2);
}
/**
 * This method is used to change the operator to invertible one
 * in "not_expr" terms, e.g. not_expr("3>5") becomes expr("3<=5").
 */
string replaceInvertibleOperator(string expr) {
	typedef pair<string, string> string_pair;

	vector<string_pair> invertibleOperators;

	invertibleOperators.push_back(make_pair("==", "!="));
	invertibleOperators.push_back(make_pair("!=", "=="));
	invertibleOperators.push_back(make_pair(">=", "<"));
	invertibleOperators.push_back(make_pair("<=", ">"));
	invertibleOperators.push_back(make_pair(">", "<="));
	invertibleOperators.push_back(make_pair("<", ">="));

	BOOST_FOREACH (string_pair invertibleOperator, invertibleOperators) {
		if (expr.find (invertibleOperator.first) != string::npos) {
			boost::replace_all(expr, invertibleOperator.first, invertibleOperator.second);
			break;
		}
	}

	return expr;
}

bool isSeparator(string s) {
//	return s == "," || s == "v" || s == "." || s == ":" || s == "-" || s == "not" || s == "!";
	return s == "," || s == "v" || s == "." || s == ":" || s == "not";
}

string getValueInsideBrackets(string s) {
	int startIndex = s.find("(") + 1;
	int endIndex = s.find(")");
	return s.substr(startIndex, endIndex - startIndex);
}

