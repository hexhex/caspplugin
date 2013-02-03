#include "casp/utility.h"

#include <string>
#include <vector>
#include <utility>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

/**
 * @brief This helper method is used to check whether string is a variable
 */
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

string removeQuotes(string s) {
	return s.substr(1, s.length() - 2);
}
/**
 * This method is used to change the operator to invertible one
 * in "not_expr" terms, e.g. not_expr("3>5") becomes expr("3<=5").
 */
string replaceInvertibleOperator(string expr) {
	typedef pair<string, string> string_pair;

	vector<string_pair> invertibleOperators;

	invertibleOperators.push_back(make_pair("==", "!="));
	invertibleOperators.push_back(make_pair("!=", "=="));
	invertibleOperators.push_back(make_pair(">=", "<"));
	invertibleOperators.push_back(make_pair("<=", ">"));
	invertibleOperators.push_back(make_pair(">", "<="));
	invertibleOperators.push_back(make_pair("<", ">="));

	BOOST_FOREACH (string_pair invertibleOperator, invertibleOperators) {
		if (expr.find (invertibleOperator.first) != string::npos) {
			boost::replace_all(expr, invertibleOperator.first, invertibleOperator.second);
			break;
		}
	}

	return expr;
}

bool isSeparator(string s) {
//	return s == "," || s == "v" || s == "." || s == ":" || s == "-" || s == "not" || s == "!";
	return s == "," || s == "v" || s == "." || s == ":" || s == "not";
}

string getValueInsideBrackets(string s) {
	int startIndex = s.find("(") + 1;
	int endIndex = s.find(")");
	return s.substr(startIndex, endIndex - startIndex);
}

