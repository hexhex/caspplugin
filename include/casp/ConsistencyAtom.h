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
#ifndef CONSISTENCYATOM_H_
#define CONSISTENCYATOM_H_
#include "config.h"

#include "casp/GecodeSolver.h"
#include "casp/LearningProcessor.h"
#include "casp/Utility.h"

#include <dlvhex2/HexParser.h>
#include <dlvhex2/HexParserModule.h>
#include <dlvhex2/HexGrammar.h>


#include <dlvhex2/PluginInterface.h>
#include <dlvhex2/ProgramCtx.h>
#include <dlvhex2/Term.h>
#include <dlvhex2/Registry.h>
#include <dlvhex2/OrdinaryAtomTable.h>
#include <dlvhex2/Table.h>

#include <dlvhex2/Printer.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <utility>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#define NDEBUG

namespace dlvhex {
namespace casp {


typedef boost::unordered_map <ID,set < Interpretation* > > MapPossibleConflict;

struct CPVariableAndConnection{
	Interpretation cpVariable;
	MapPossibleConflict possibleConflictCpVariable;
};
/**
 * @brief Defines the consistency atom of the plugin:
 * :- not casp&[domain, globalConstraints, expr, not_expr,sumElement]
 */
class ConsistencyAtom : public PluginAtom
{
public:
	/**
	 * Simple constuctor, which accepts option for IIS learning
	 */
	ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor,
			boost::shared_ptr<SimpleParser> simpleParser,const CPVariableAndConnection& cpVariableAndConnection,bool cspGraphLearning,bool cspAnticipateLearning);
	/**
	 * @brief Retrieves answer for query.
	 * Should not be called, as learning is enabled.
	 */
	virtual void retrieve(const Query& query, Answer& answer) throw (PluginError)
	{
		assert(false);
	}

	/**
	 * @brief Retrieves answer for query and learns IIS based on processor
	 */
	virtual void retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError);

private:
	boost::shared_ptr<LearningProcessor> learningProcessor;
	boost::shared_ptr<LearningProcessor> backwardlearningProcessor;
	boost::shared_ptr<SimpleParser> simpleParser;

	bool cspAnticipateLearning;
	bool cspGraphLearning;
	const Interpretation& cpVariables;
	const MapPossibleConflict& possibleConflictCpVariable;

	ID exprAuxID;
	ID not_exprAuxID;
	ID domainAuxID;
	ID maximizeAuxID;
	ID minimizeAuxID;
	ID sumElementAuxID;
	bool idSaved;
	PredicateMask pm;

	// store constraint atom id
	void storeID( RegistryPtr& registry);

	void anticipateLearning(RegistryPtr& reg,const InterpretationConstPtr& assigned,NogoodContainerPtr& nogoods,vector<string>& expression,vector<ID>& atomIds,
			vector<OrdinaryAtom> &sumData,int domainMinValue,int domainMaxValue,string& globalConstraintName,string& globalConstraintValue,Interpretation& toCheck);

	string getExpressionFromID(RegistryPtr& reg, const OrdinaryAtom& atom,bool replaceReversibleOperator );
};
#endif /* CONSISTENCYATOM_H_ */
}
}
