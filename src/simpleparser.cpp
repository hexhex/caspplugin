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
#include <stdio.h>
#include <stdlib.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;

SimpleParser::SimpleParser() {
}

int SimpleParser::priority(string s)
{
	if(s == "*" || s == "/" || s =="%")
		return 3;
	else if(s == "+" || s == "-")
		return 2;
	else if(s == "==" || s == ">" || s == "<" || s == "<=" || s == ">=" || s == "=")
		return 1;

	return 0;
}


int SimpleParser::isOperator(string s)
{
	return priority(s) != 0;
}

string SimpleParser::convertToPostfix(string infix)
{
	string p;

	stack<string> x;

	char_separator<char> sep(" ", ",v.:-$%+-*/<>=()", drop_empty_tokens);

	tokenizer<char_separator<char> > tokens(infix, sep);
	vector<string> tokenList;

	for ( tokenizer<char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
		string token = *it;
		tokenList.push_back(token);
	}
	for (int i = 0; i < tokenList.size(); i++) {
		string token = tokenList[i];

		if (token == "(") {
			x.push(token);
		}
		else if (token == ")") {
			string n1 = x.top();
			x.pop();

			while (n1 != "(") {
				p += n1 + " ";
				n1 = x.top();
				x.pop();
			}
		}
		else if (isOperator(token)) {
			// check whether next token is also operator
			if (i < tokenList.size() - 1 && isOperator(tokenList[i+1])) {
				token += tokenList[i+1];
				i++;
			}

			if (x.empty())
				x.push(token);
			else {
				string n1 = x.top();

				while (priority(n1) >= priority(token)) {
					x.pop();

					p += n1 + " ";

					if (x.empty()) break;

					n1 = x.top();
				}
				x.push(token);
			}
		}
		else {
			p += token + " ";
		}
	}

	while(!x.empty())
	{
		string n1 = x.top();
		x.pop();

		p += n1 + " ";
	}

	return p;
}

void SimpleParser::makeTree(string infix, struct ParseTree** root) {
	if (_cachedTrees.find(infix) != _cachedTrees.end()) {
		*root = new ParseTree(_cachedTrees[infix]);
		return;
	}

	string postfix = convertToPostfix(infix);

	stack<ParseTree*> elems;

	ParseTree *newTree, *left, *right;

	istringstream in(postfix);

	string s;
	while (in>>s) {
		if (isOperator(s)) {
			// Add new element
			right = elems.top();
			elems.pop();

			left = elems.top();
			elems.pop();

			newTree = new ParseTree;

			newTree->value = s;
			newTree->left = left;
			newTree->right = right;
			newTree->type = OPERATOR;

			elems.push(newTree);
		}

		else {
			newTree = new ParseTree;

			newTree->value = s;

			ElementType type = NUMBER;

			for (int i = 0; i < s.length(); i++)
				if (!isdigit(s[i])) type = CONSTRAINT_VARIABLE;

			newTree->type = type;

			elems.push(newTree);
		}
	}
	*root = elems.top();
	elems.pop();
}

void SimpleParser::deleteTree(ParseTree* root) {
	if (root->type == OPERATOR) {
		deleteTree(root->left);
		deleteTree(root->right);
	}
	delete root;
}
