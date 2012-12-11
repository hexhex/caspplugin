/*
 * gecodesolver.h
 *
 *  Created on: Nov 22, 2012
 *      Author: faeton
 */

#ifndef GECODESOLVER_H_
#define GECODESOLVER_H_

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <map>

#include "simpleparser.h"

class GecodeSolver: public Gecode::Space {
public:
	/// Actual model
	GecodeSolver(std::vector<std::string> expressions, std::string domain);

	GecodeSolver(bool share, GecodeSolver& s) :
			Space(share, s), minValue(s.minValue),
			maxValue(s.maxValue),
			constraintVariables(s.constraintVariables) {
	}

	virtual Space*
	copy(bool share) {
		return new GecodeSolver(share, *this);
	}
private:
	int minValue, maxValue;
	std::map<std::string, Gecode::IntVar> constraintVariables;

	Gecode::LinExpr makeExpression(ParseTree* tree);
};



#endif /* GECODESOLVER_H_ */
