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
	return s == "," || s == "v" || s == "." || s == ":" || s == "-" || s == "not" || s == "!";
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

string getValueInsideBrackets(string s) {
	int startIndex = s.find("(") + 1;
	int endIndex = s.find(")");
	return s.substr(startIndex, endIndex - startIndex);
}

void CaspConverter::convert(istream& i, ostream& o) {
	vector<string> variables;
	vector<string> expressions;
	vector<string> sumPredicates;

	bool cspExpression = false;

	string input;

	string specialDirectives[3] = { "dom", "maximize", "minimize" };

	while (getline(i, input)) {
		bool stop = false;
		for (int i = 0; i < 3; i++) {
			string directive = specialDirectives[i];
			if (boost::starts_with(input, "$" + directive)) {
				input = getValueInsideBrackets(input);
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
				cspExpression = true;
			} else {
				expressions.push_back(token);
			}

			if (cspExpression) {
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
							if (isVariable(expressions[i]))
								variables.push_back(expressions[i]);
							if (starts_with(expressions[i], "sum")) {
								// transform sum(p) to sum_p for proper parsing
								sumPredicates.push_back(getValueInsideBrackets(expressions[i]));
								expressions[i] = "sum_" + getValueInsideBrackets(expressions[i]);
							}
							caspExpression = expressions[i] + caspExpression;
						}
					}
					expressions.clear();

					// write everything to the right part of expression
					it++;
					while (it != tokens.end()) {
						token = *it;

						if (isSeparator(token)) {
							o << "expr" << variables.size() << "(\"" << caspExpression << "\"";
							for (int i = 0; i < variables.size(); i++) {
								o << "," << variables[i];
							}
							o << ")" << token;

							break;
						} else if (token != "$") {
							if (isVariable(token))
								variables.push_back(token);
							else if (starts_with(token, "sum")) {
								// transform sum(p) to sum_p for proper parsing
								sumPredicates.push_back(getValueInsideBrackets(token));
								token = "sum_" + getValueInsideBrackets(token);
							}

							caspExpression += token;
						}

						it++;
					}
					cspExpression = false;
				}
			}
		}
		for (int i = 0; i < expressions.size(); i++)
			o << expressions[i];
		o << endl;

		expressions.clear();
	}

	int maxSumPredicates = 5;

	o << ":- not &casp[dom,maximize,minimize";
	for (int i = 0; i < 11; i++) {
		o << ",expr" << i << ",not_expr" << i;
	}
	for (int i = 0; i < sumPredicates.size(); i++)
		o << "," << sumPredicates[i];
	for (int i = 0; i < maxSumPredicates - sumPredicates.size(); i++)
		o << "," << "dummy_sum_predicate" << i;
	o << "]()." << endl;
}
