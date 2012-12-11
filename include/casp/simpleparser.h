/*
 * simpleparser.h
 *
 *  Created on: Dec 3, 2012
 *      Author: faeton
 */

#ifndef SIMPLEPARSER_H_
#define SIMPLEPARSER_H_

#include <string>

enum ElementType {
	NUMBER, CONSTRAINT_VARIABLE, ASP_VARIABLE, OPERATOR
};

struct ParseTree {
	char *value;
	struct ParseTree *left, *right;
	ElementType type;
};

const int MAX = 500;

class SimpleParser {
private:
	int isOperator(char e);
	int priority(char e);
	void convertToPostfix(char* infix, char * postfix);
public:
	SimpleParser();
	void makeTree(char *infix, struct ParseTree** root);
};

#endif /* SIMPLEPARSER_H_ */
