#include "casp/caspconverter.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <vector>
#include <set>
#include <string>

using namespace std;
using namespace boost;

CaspConverter::CaspConverter() {

}

CaspConverter::~CaspConverter() {

}

bool isSeparator(string s) {
	return s == "," || s == "v" || s == "." || s == ":" || s == "-" || s == "not";
}

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

void CaspConverter::convert(istream& i, ostream& o) {
	vector<string> variables;
	vector<string> expressions;

	string input;

	string specialDirectives[3] = { "dom", "maximize", "minimize" };

	while (getline(i, input)) {
		bool stop = false;
		for (int i = 0; i < 3; i++) {
			string directive = specialDirectives[i];
			if (boost::starts_with(input, "$" + directive)) {
				int startIndex = input.find("(") + 1;
				int endIndex = input.find(")");
				input = input.substr(startIndex, endIndex - startIndex);
				o << directive << "(\"" << input << "\")." << endl;
				stop = true;
				break;
			}
		}
		if (stop)
			continue;

		char_separator<char> sep("", " ,v.:-$%", drop_empty_tokens);

		tokenizer<char_separator<char> > tokens(input, sep);
		vector<string> tokensList;
		for (tokenizer<char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			string token = *it;

			if (token == "%") {
				break;
			} else if (token == "$") {
				int mainOperatorLength = -1;
				if (boost::starts_with(token, ">=") || boost::starts_with(token, "<=") || boost::starts_with(token, "=="))
					mainOperatorLength = 2;
				else if (boost::starts_with(token, ">") || boost::starts_with(token, "<"))
					mainOperatorLength = 1;

				if (mainOperatorLength != -1) {
					string op = token.substr(0, mainOperatorLength);

					token = token.substr(mainOperatorLength);

					string caspExpression = "";
					// write everything to the left part of expression
					for (int i = expressions.size() - 1; i >= 0; i--) {
						if (isSeparator(expressions[i])) {
							for (int j = 0; j <= i; j++) {
								o << expressions[j];
							}
							break;
						} else {
							caspExpression = expressions[i] + caspExpression;
							if (isVariable(expressions[i]))
								variables.push_back(expressions[i]);
						}
					}
					expressions.clear();

					// write everything to the right part of expression
					it++;
					while (it != tokens.end()) {
						token = *it;

						if (isSeparator(token)) {
							o << "expr" << variables.size() << "(\"" << caspExpression << "\"";
							for (int i = 0; i < variables.size(); i++)
								o << "," << variables[i];
							o << ")" << token;

							break;
						} else if (token != "$") {
							caspExpression += token;
							if (isVariable(token))
								variables.push_back(token);
						}

						it++;
					}
				}
			}
			else {
				expressions.push_back(token);
			}
		}
		for (int i = 0; i < expressions.size(); i++)
			o << expressions[i];
		o << endl;
		expressions.clear();
	}

	o << ":- not &casp[expr0,expr1,expr2,expr3,expr4,expr5,expr6,expr7,expr8,expr9,expr10,dom,maximize,minimize]()." << endl;
}
