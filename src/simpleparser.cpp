/*
 * simpleparser.cpp
 *
 * Simple parser allows to recognize linear expression
 * Created on: Nov 22, 2012
 * Author: Oleksandr Stashuk
 */

#include "casp/gecodesolver.h"
#include "casp/simpleparser.h"

#include <stack>

SimpleParser::SimpleParser() {
	top = -1;
	strcpy(target, "");
	strcpy(stack, "");
	t = target;
	s = "";
}
void SimpleParser::setexpr(char *str) {
	s = str;
}
void SimpleParser::push(char c) {
	if (top == MAX)
		throw "Too long expression";
	else {
		top++;
		stack[top] = c;
	}
}
char SimpleParser::pop() {
	if (top == -1) {
		throw "Stack is empty";
		return -1;
	} else {
		char item = stack[top];
		top--;
		return item;
	}
}
void SimpleParser::convert() {
	while (*s) {
		if (*s == ' ' || *s == '\t') {
			s++;
			continue;
		}
		if (isdigit(*s) || isalpha(*s)) {
			while (isdigit(*s) || isalpha(*s)) {
				*t = *s;
				s++;
				t++;
			}
		}
		if (*s == '(') {
			push (*s);
			s++;
		}
		char opr;
		if (*s == '*' || *s == '+' || *s == '/' || *s == '%' || *s == '-'
				|| *s == '=' || *s == '<' || *s == '>') {
			if (top != -1) {
				opr = pop();
				while (priority(opr) >= priority(*s)) {
					*t = opr;
					t++;
					opr = pop();
				}
				push(opr);
				push (*s);
			} else
				push (*s);
			s++;
		}
		if (*s == ')') {
			opr = pop();
			while ((opr) != '(') {
				*t = opr;
				t++;
				opr = pop();
			}
			s++;
		}
	}
	while (top != -1) {
		char opr = pop();
		*t = opr;
		t++;
	}
	*t = '\0';
}

int SimpleParser::priority(char c) {
	if (c == '>' || c == '<' || c == '=')
		return 3;
	if (c == '*' || c == '/' || c == '%')
		return 2;
	else {
		if (c == '+' || c == '-')
			return 1;
		else
			return 0;
	}
}

ParseTree* SimpleParser::makeTree() {
	std::stack<ParseTree*> stack;
	for (size_t i = 0; i < strlen(target); i++) {
		char c = target[i];

		ParseTree *newTree = new ParseTree;
		if (isalpha(c) || isdigit(c)) {
			newTree->value = c;
			newTree->rightOperand = 0;
			newTree->leftOperand = 0;
			stack.push(newTree);
		}
		else {
			newTree-> value = c;

			newTree->rightOperand = stack.top();
			stack.pop();

			newTree->leftOperand = stack.top();
			stack.pop();

			stack.push(newTree);
		}
	}
	return stack.top();
}

