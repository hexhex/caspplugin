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

using namespace std;

class GecodeSolver: public Gecode::MaximizeSpace {
public:
	/// Actual model
	GecodeSolver(vector<string> expressions, string domain, string globalConstraintName, string globalConstraintValue);

	GecodeSolver(bool share, GecodeSolver& s) :
			Gecode::MaximizeSpace(share, s),
			minValue(s.minValue),
			maxValue(s.maxValue),
			constraintVariables(s.constraintVariables),
			costVariable(s.costVariable) {
	}

	virtual Gecode::Space*
	copy(bool share) {
		return new GecodeSolver(share, *this);
	}
	virtual Gecode::IntVar cost() const {
		return costVariable;
	}
private:
	int minValue;
	int maxValue;
	map<string, Gecode::IntVar> constraintVariables;
	Gecode::IntVar costVariable;

	Gecode::LinExpr makeExpression(ParseTree* tree);
};



#endif /* GECODESOLVER_H_ */
