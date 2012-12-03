/*
 * simpleparser.h
 *
 *  Created on: Dec 3, 2012
 *      Author: faeton
 */

#ifndef SIMPLEPARSER_H_
#define SIMPLEPARSER_H_

#include <string>

struct ParseTree;

struct ParseTree {
	char value;
	ParseTree *leftOperand;
	ParseTree *rightOperand;
};

const int MAX = 500;

class SimpleParser {
private:
	char target[MAX], stack[MAX];
	char *s, *t;
	int top;
public:
	SimpleParser();
	void setexpr(char *str);
	void push(char c);
	char pop();
	void convert();
	int priority(char c);
	ParseTree* makeTree();
};

#endif /* SIMPLEPARSER_H_ */
