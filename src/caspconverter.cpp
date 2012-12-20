/*
 * caspconverter.cpp
 *
 *  Created on: Dec 17, 2012
 *      Author: faeton
 */

#include "casp/caspconverter.h"

#include <vector>
#include <set>
#include <string>

using namespace std;

CaspConverter::CaspConverter() {

}

CaspConverter::~CaspConverter() {

}

bool isSeparator(char c) {
	return c == ',' || c == 'v' || c == '.' || c == ':' || c == '-';
}

void CaspConverter::convert(istream& i, ostream& o) {
	// TODO: check for comments
	string result = "", temp = "";
	bool caspExpression = false;

	vector<string> guessData;

	set<string> variables;
	string currentVariable = "";

	string input;
	while (getline(i, input)) {
		for (size_t i = 0; i < input.size(); i++) {
			if (currentVariable == "" && isupper(input[i])) {
				currentVariable = input[i];
			} else if (currentVariable != "") {
				if (isalnum(input[i]) || input[i] == '_')
					currentVariable += input[i];
				else {
					variables.insert(currentVariable);
					currentVariable = "";
				}
			}

			if (input[i] == '$') {
				caspExpression = true;
			} else if (isSeparator((char) input[i])) {
				if (caspExpression) {
					string currentExpression = "";
					currentExpression += "expr(\"" + temp;
					currentExpression += "\"";
					for (set<string>::iterator it = variables.begin();
							it != variables.end(); it++) {
						currentExpression += "," + *it;
					}
					currentExpression += ")";

					result += currentExpression;
					guessData.push_back(currentExpression);
				} else
					result += temp;

				// Clear data
				temp = "";
				caspExpression = false;
				result += input[i];

				variables.clear();
			} else
				temp += input[i];

			if (caspExpression && temp == "dom") {
				// This is domain expression
				string s = "";
				for (int j = 0; j < input.size(); j++) {
					if (input[j] == '$') continue;
					if (input[j] == ')') s += "\"";
					s += input[j];
					if (input[j] == '(') s += "\"";
				}
				o << s << endl;

				temp = "";
				caspExpression = false;
				break;
			}
		}

		o << result << endl;

		result = "";
	}

}
