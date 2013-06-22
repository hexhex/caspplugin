#include "casp/caspconverter.h"
#include "casp/utility.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <vector>
#include <set>
#include <string>

using namespace std;
using namespace boost;

DefaultConverter::DefaultConverter() {
}

DefaultConverter::~DefaultConverter() {
}

void DefaultConverter::convert(istream& i, ostream& o) {
	string input;
	while (getline(i, input)) {
		o << input << endl;
	}
}

CaspConverter::CaspConverter() {

}

CaspConverter::~CaspConverter() {

}

void CaspConverter::convert(istream& i, ostream& o) {
	vector<string> variables;
	vector<string> expressions;
	vector<string> sumPredicates;

	bool cspExpression = false;

	string input;

	string specialDirectives[3] = { "domain", "maximize", "minimize" };

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

		char_separator<char> sep("", " ,v.:-$%()<>=!+-/*", drop_empty_tokens);

		tokenizer<char_separator<char> > tokens(input, sep);
		vector<string> tokensList;
		for (tokenizer<char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			string token = *it;
			tokensList.push_back(token);
		}

		for (int k = 0; k < tokensList.size(); k++) {
			string token = tokensList[k];
			if (token == "%") {
				break;
			} else if (token == "$") {
				cspExpression = true;
			} else {
				expressions.push_back(token);
			}

			if (cspExpression) {
				bool op = false;

				if (token == ">" || token == "<" || token == "!" || token == "=") {
					if (tokensList[k + 1] == "=") {
						k++;
						token += tokensList[k];

						expressions[expressions.size() - 1] = token;
					}
					op = true;
				}

				if (op) {
					string caspExpression = "";

					// Check whether all brackets were closed properly
					int bracketsCount = 0;
					// write everything to the left part of expression
					int startIndex = -1;

					for (int i = expressions.size() - 1; i >= 0; i--) {
						if (expressions[i] == ")")
							bracketsCount++;
						else if (expressions[i] == "(")
							bracketsCount--;

						if (isSeparator(expressions[i]) && bracketsCount == 0) {
							startIndex = i;
							if (expressions[i] == ":" && expressions[i+1]=="-")
								startIndex+=2;
							break;
						}
					}

					for (int i = 0; i <= startIndex; i++) {
						o << expressions[i];
					}

					for (int i = startIndex + 1; i < expressions.size(); i++) {
						if (isVariable(expressions[i])) {
							variables.push_back(expressions[i]);
							caspExpression += expressions[i];
						}
						else if (starts_with(expressions[i], "sum")) {
							// transform sum(p,2) to sum_p_2 for proper parsing
							i += 2;
							string predicate = expressions[i];
							i += 2;
							string position = expressions[i];
							i += 1;
							sumPredicates.push_back(predicate);

							caspExpression += "sum_" + predicate + "_" + position;
						}
						else caspExpression += expressions[i];
					}
					expressions.clear();

					// write everything to the right part of expression, which is not in brackets
					bracketsCount = 0;

					k++;
					while (k < tokensList.size()) {
						token = tokensList[k];

						if (token == ")")
							bracketsCount++;
						else if (token == "(")
							bracketsCount--;

						if (isSeparator(token) && bracketsCount == 0) {
							// In each line, following replacements are done back and forth
							// '(' - '{'
							// ')' - '}'
							// ',' - ';'
							// This is due to the fact that hex parser splits improperly them inside of string term
							boost::replace_all(caspExpression, "(", "{");
							boost::replace_all(caspExpression, ")", "}");
							boost::replace_all(caspExpression, ",", ";");
								
							o << "expr" << variables.size() << "(\"" << caspExpression << "\"";
							for (int i = 0; i < variables.size(); i++) {
								o << "," << variables[i];
							}
							o << ")" << token;

							variables.clear();

							break;
						} else if (token != "$") {
							if (isVariable(token))
								variables.push_back(token);
							else if (starts_with(token, "sum")) {
								k += 2;
								string predicate = tokensList[k];
								k += 2;
								string position = tokensList[k];
								k += 1;
								sumPredicates.push_back(predicate);

								token = "sum_" + predicate + "_" + position;
							}

							caspExpression += token;
						}

						k++;
					}
					cspExpression = false;
				}
			}
		}
		for (int i = 0; i < expressions.size(); i++) {
			o << expressions[i];
		}
		o << endl;

		expressions.clear();
	}

	int maxSumPredicates = 15;

	o << ":- not &casp[domain,maximize,minimize";
	for (int i = 0; i < 11; i++) {
		o << ",expr" << i << ",not_expr" << i;
	}
	for (int i = 0; i < sumPredicates.size(); i++)
		o << "," << sumPredicates[i];

	for (int i = 0; i < maxSumPredicates - sumPredicates.size(); i++)
		o << "," << "dummy_sum_predicate" << i;
	o << "]()." << endl;
}
