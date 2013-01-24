#ifndef UTILITY_H_
#define UTILITY_H_

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
bool isVariable(string s);

/**
 * @brief This helper method removes quotes from the string
 */
string removeQuotes(string s);

/**
 * @brief This method is used to change the operator to invertible one
 * in "not_expr" terms, e.g. not_expr("3>5") becomes expr("3<=5").
 */
string replaceInvertibleOperator(string expr);

/**
 * @brief This helper method checks, whether string is a separator
 */
bool isSeparator(string s);

/**
 * @brief This helper method get's value of string, which is
 * contained inside of the brackets
 */
string getValueInsideBrackets(string s);

#endif /* UTILITY_H_ */
