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

#include "simpleparser.h"

class GecodeSolver: public Gecode::Space {
public:
	/// Actual model
	GecodeSolver(std::string expression);

	virtual void print(std::ostream& os) const {
	}

	GecodeSolver(bool share, GecodeSolver& s) :
			Space(share, s) {
	}

	virtual Space*
	copy(bool share) {
		return new GecodeSolver(share, *this);
	}
private:
	Gecode::IntVar* makeExpression(ParseTree* tree);
};



#endif /* GECODESOLVER_H_ */
