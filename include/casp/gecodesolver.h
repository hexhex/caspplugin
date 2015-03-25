#ifndef GECODESOLVER_H_
#define GECODESOLVER_H_

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <dlvhex2/OrdinaryAtomTable.h>
#include <dlvhex2/Registry.h>

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

#include "simpleparser.h"

using namespace std;

class GecodeSolver: public Gecode::MaximizeSpace {
public:
	/// Actual model
	GecodeSolver(dlvhex::RegistryPtr reg,vector<dlvhex::OrdinaryAtom> sumElement, int domainMaxValue,int domainMinValue, string globalConstraintName, string globalConstraintValue,
			boost::shared_ptr<SimpleParser> simpleParser);

	GecodeSolver(bool share, GecodeSolver& s) :
			Gecode::MaximizeSpace(share, s),
			_minValue(s._minValue),
			_maxValue(s._maxValue),
			_constraintVariables(s._constraintVariables),
			_costVariable(s._costVariable),
			_simpleParser(s._simpleParser) {
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

	dlvhex::RegistryPtr _reg;

	vector<dlvhex::OrdinaryAtom> _sumElement;

	boost::shared_ptr<SimpleParser> _simpleParser;

	map<string, Gecode::IntVar> _constraintVariables;
	Gecode::IntVar _costVariable;

	Gecode::LinExpr makeExpression(ParseTree* tree);
};



#endif /* GECODESOLVER_H_ */
