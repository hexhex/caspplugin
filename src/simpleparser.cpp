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

#define MAX 50
#define EMPTY -1

using namespace std;

SimpleParser::SimpleParser() {
}

struct basic_stack
{
	char data[MAX];
	int top;
};

int isempty(struct basic_stack *s)
{
	return (s->top == EMPTY) ? 1 : 0;
}

void emptystack(struct basic_stack* s)
{
	s->top=EMPTY;
}

void push(struct basic_stack* s,int item)
{
	if(s->top == (MAX-1))
	{
		printf("\nSTACK FULL");
	}
	else
	{
		++s->top;
		s->data[s->top]=item;
	}
}

char pop(struct basic_stack* s)
{
	char ret=(char)EMPTY;
	if(!isempty(s))
	{
		ret= s->data[s->top];
		--s->top;
	}
	return ret;
}

int SimpleParser::priority(char e)
{
	if(e == '*' || e == '/' || e =='%')
		return 3;
	else if(e == '+' || e == '-')
		return 2;
	else if(e == '=' || e == '>' || e == '<')
		return 1;

	return 0;
}

int SimpleParser::isOperator(char e)
{
	return priority(e) != 0;
}

void SimpleParser::convertToPostfix(char* infix, char * postfix)
{
	char *i,*p;
	struct basic_stack X;
	char n1;
	emptystack(&X);
	i = &infix[0];
	p = &postfix[0];

	while(*i)
	{
		while(*i == ' ' || *i == '\t')
		{
			i++;
		}

		if( isdigit(*i) || isalpha(*i) )
		{
			while( isdigit(*i) || isalpha(*i))
			{
				*p = *i;
				p++;
				i++;
			}
			*p = ' ';
			p++;
		}

		if( *i == '(' )
		{
			push(&X,*i);
			i++;
		}

		if( *i == ')')
		{
			n1 = pop(&X);
			while( n1 != '(' )
			{
				*p = n1;
				p++;
				*p = ' ';
				p++;
				n1 = pop(&X);
			}
			i++;
		}

		if( isOperator(*i) )
		{
			if(isempty(&X))
				push(&X,*i);
			else
			{
				n1 = pop(&X);
				while(priority(n1) >= priority(*i))
				{
					*p = n1;
					p++;
					*p = ' ';
					p++;
					n1 = pop(&X);
				}
				push(&X,n1);
				push(&X,*i);
			}
			i++;
		}
	}
	while(!isempty(&X))
	{
		n1 = pop(&X);
		*p = n1;
		p++;
		*p = ' ';
		p++;
	}
	*p = '\0';
}

void SimpleParser::makeTree(char *infix, struct ParseTree** root) {
	char postfix[MAX] = { 0 };
	convertToPostfix(infix, postfix);

	stack<ParseTree*> elems;

	ParseTree *newTree, *left, *right;

	char *p = &postfix[0];

	char data[MAX];
	char *current = data;
	ElementType currentType = NUMBER;

	while (*p) {
		if (isdigit(*p)) {
			*current = *p;
			current++;
		}
		else if (isalpha(*p)) {
			*current = *p;
			current++;
			currentType = CONSTRAINT_VARIABLE;
		}
		else if (*p == ' ' && current!=data) {
			// Finish current
			newTree = (ParseTree*)malloc(sizeof(struct ParseTree));
			*current = '\0';
			current++;

			newTree->value = (char*) malloc(sizeof(char)*strlen(data));
			newTree->type = currentType;
			strcpy(newTree->value, data);

			elems.push(newTree);
			current = data;
			currentType = NUMBER;
		}
		else if (isOperator(*p)) {
			// Add new element
			right = elems.top();
			elems.pop();

			left = elems.top();
			elems.pop();

			newTree = (ParseTree*) malloc(sizeof(struct ParseTree));

			newTree->value = (char*) malloc(sizeof(char)*2);
			newTree->value[0] = *p;
			newTree->value[1] = '\0';

			newTree->left = left;
			newTree->right = right;

			newTree->type = OPERATOR;

			elems.push(newTree);
		}
		p++;
	}
	*root = elems.top();
	elems.pop();
}
