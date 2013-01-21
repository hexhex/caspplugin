/*
 * simpleparser.h
 *
 *  Created on: Dec 3, 2012
 *      Author: faeton
 */

#ifndef SIMPLEPARSER_H_
#define SIMPLEPARSER_H_

#include <string>
#include <map>

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
	void makeTree(string infix, struct ParseTree** root);
	void deleteTree(struct ParseTree* root);
};

#endif /* SIMPLEPARSER_H_ */
