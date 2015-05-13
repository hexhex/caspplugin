/* CASPPlugin -- Pluging for dlvhex - Answer-Set Programming with constraint programming
 * Copyright (C) 2013 Oleksandr Stashuk
 * Copyright (C) 2015 Alessandro Francesco De Rosis
 *
 * This file is part of caspplugin.
 *
 * CASPPlugin is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * CASPPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
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

#include "SimpleParser.h"

using namespace std;

class GecodeSolver: public Gecode::MaximizeSpace {
public:
	/// Actual model
	GecodeSolver(dlvhex::RegistryPtr reg,vector<dlvhex::OrdinaryAtom>& sumElement, int domainMaxValue,int domainMinValue, string globalConstraintName, string globalConstraintValue,
			boost::shared_ptr<SimpleParser>& simpleParser);

	GecodeSolver(bool share, GecodeSolver& s) :
			Gecode::MaximizeSpace(share, s),
			_minValue(s._minValue),
			_maxValue(s._maxValue),
			_constraintVariables(s._constraintVariables),
			_costVariable(s._costVariable),
			_simpleParser(s._simpleParser) {
	}

	void propagate(vector<string>& expressions);
	void propagate(string& expression);

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
