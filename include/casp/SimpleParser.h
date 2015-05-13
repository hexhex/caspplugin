/* CASPPlugin -- Pluging for dlvhex - Answer-Set Programming with constraint programming
 * Copyright (C) 2013 Oleksandr Stashuk
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
#ifndef SIMPLEPARSER_H_
#define SIMPLEPARSER_H_

#include <string>
#include <map>
#include <set>

using namespace std;

enum ElementType {
	NUMBER, CONSTRAINT_VARIABLE, OPERATOR
};

struct ParseTree {
	string value;
	struct ParseTree *left, *right;
	ElementType type;
};

const int MAX = 500;

class SimpleParser {
private:
	map<string, ParseTree> _cachedTrees;

	int isOperator(string s);
	int priority(string s);
	string convertToPostfix(string infix);
public:
	SimpleParser();
	set<string> getConstraintVariables(struct ParseTree* root);
	void makeTree(string infix, struct ParseTree** root);
	void deleteTree(struct ParseTree* root);
};

#endif /* SIMPLEPARSER_H_ */
