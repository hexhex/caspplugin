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

/**
 * @brief Defines the consistency atom of the plugin:
 * :- not casp&[dom, exprN, not_exprN, globalConstraints]
 */
class ConsistencyAtom : public PluginAtom
{
public:
	/**
	 * Simple constuctor, which accepts option for IIS learning
	 */
	ConsistencyAtom(int numberSumPredicate,boost::shared_ptr<LearningProcessor> learningProcessor,
			boost::shared_ptr<SimpleParser> simpleParser);
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
	ID _exprID;
	ID _notExprID;
	ID _domID;
	ID _maxID;
	ID _minID;
	bool _idSaved;

	// store constraint atom id
	void storeID( dlvhex::Registry& registry);
};
#endif /* CONSISTENCYATOM_H_ */
}
}