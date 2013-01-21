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
	GecodeSolver(vector<string> sumData, string domain, string globalConstraintName, string globalConstraintValue);

	GecodeSolver(bool share, GecodeSolver& s) :
			Gecode::MaximizeSpace(share, s),
			_minValue(s._minValue),
			_maxValue(s._maxValue),
			_constraintVariables(s._constraintVariables),
			_costVariable(s._costVariable) {
	}

	void propagate(vector<string> expressions);
	void propagate(string expression);

	virtual Gecode::Space*
	copy(bool share) {
		return new GecodeSolver(share, *this);
	}
	virtual Gecode::IntVar cost() const {
		return _costVariable;
	}
private:
	int _minValue;
	int _maxValue;

	vector<string> _sumData;

	map<string, Gecode::IntVar> _constraintVariables;
	Gecode::IntVar _costVariable;

	Gecode::LinExpr makeExpression(ParseTree* tree);
};



#endif /* GECODESOLVER_H_ */
