#ifndef CONSISTENCYATOM_H_
#define CONSISTENCYATOM_H_
#include "config.h"

#include "casp/gecodesolver.h"
#include "casp/learningprocessor.h"
#include "casp/utility.h"

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


typedef set <ID> SetID;
typedef boost::unordered_map <ID,set < SetID* > > MapPossibleConflict;

struct CPVariableAndConnection{
	SetID cpVariable;
	MapPossibleConflict possibleConflictCpVariable;
};
/**
 * @brief Defines the consistency atom of the plugin:
 * :- not casp&[dom, expr, not_expr, globalConstraints,sumElement]
 */
class ConsistencyAtom : public PluginAtom
{
public:
	/**
	 * Simple constuctor, which accepts option for IIS learning
	 */
	ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor,
			boost::shared_ptr<SimpleParser> simpleParserm,const CPVariableAndConnection& cpVariableAndConnection,bool cspGraphLearning);
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
	boost::shared_ptr<LearningProcessor> _learningProcessor;
	boost::shared_ptr<SimpleParser> _simpleParser;

	bool _cspGraphLearning;
	const SetID& _cpVariables;
	const MapPossibleConflict& _possibleConflictCpVariable;

	ID _exprID;
	ID _notExprID;
	ID _domID;
	ID _maxID;
	ID _minID;
	ID _sumElementID;
	bool _idSaved;
	PredicateMask _pm;

	// store constraint atom id
	void storeID( RegistryPtr& registry);

	void tryToLearnMore(RegistryPtr& reg,const InterpretationConstPtr& assigned,NogoodContainerPtr& nogoods,vector<string>& expression,vector<ID>& atomIds,
			vector<OrdinaryAtom> &sumData,int domainMinValue,int domainMaxValue,string& globalConstraintName,string& globalConstraintValue,SetID& toCheck);

	string getExpressionFromID(RegistryPtr& reg, const OrdinaryAtom& atom,bool replaceReversibleOperator );
};
#endif /* CONSISTENCYATOM_H_ */
}
}
