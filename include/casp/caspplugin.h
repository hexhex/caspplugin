#include "config.h"

#include "casp/gecodesolver.h"
#include "casp/caspconverter.h"
#include "casp/casprewriter.h"
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
	ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor,
			boost::shared_ptr<SimpleParser> simpleParser) :
				PluginAtom( "casp", 0),
				_learningProcessor(learningProcessor),
				_simpleParser(simpleParser),_idSaved(false)
	{
		// This add predicates for all input parameters
		for (int i = 0; i < 40; i++)
			addInputPredicate();

		setOutputArity(0);
	}

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
	virtual void
	retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError)
	{
		Registry &registry = *getRegistry();

		std::vector<std::string> expressions;
		std::vector<std::string> sumData;
		std::string domain = "";
		std::string globalConstraintName = "";
		std::string globalConstraintValue = "";

		std::pair<Interpretation::TrueBitIterator, Interpretation::TrueBitIterator>
		trueAtoms = query.assigned->trueBits();

		vector<ID> atomIds;

		// Iterate over all input interpretation
		for (Interpretation::TrueBitIterator it = trueAtoms.first; it != trueAtoms.second; it++) {
			const OrdinaryAtom &atom = query.assigned->getAtomToBit(it);
			Term name = registry.terms.getByID(atom.tuple[0]);
			if(!query.interpretation->getFact(*it)){
				continue;
			}
			string expr="";
			if(!_idSaved){
				StoreID(registry);
			}
			if (atom.tuple[0]==_exprID) {
				Term value = registry.terms.getByID(atom.tuple[1]);
				expr = value.symbol;
			}
			else if (atom.tuple[0]==_notExprID) {
				Term value = registry.terms.getByID(atom.tuple[1]);
				expr = replaceInvertibleOperator(value.symbol);
			}
			else if (atom.tuple[0]==_domID) {
				domain = removeQuotes(registry.terms.getByID(atom.tuple[1]).symbol);
			}
			else if (atom.tuple[0]==_maxID ||atom.tuple[0]==_minID) {
				globalConstraintName = name.symbol;
				globalConstraintValue = removeQuotes(registry.terms.getByID(atom.tuple[1]).symbol);
			}
			else { // this predicate received as input to sum aggregate function
				sumData.push_back(atom.text);
			}
			if (expr != "") { // handle expr and not_expr
				// In each line, following replacements are done back and forth
				// '{' - '('
				// '}' - ')'
				// ';' - ','
				// This is due to the fact that hex parser splits improperly them inside of string term
				boost::replace_all(expr, "{", "(");
				boost::replace_all(expr, "}", ")");
				boost::replace_all(expr, ";", ",");
				// Replace all ASP variables with their actual values.
				int variableIndex = 2;

				boost::char_separator<char> sep(" ", ",v.:-$%<>=+-/*\"<>=()", boost::drop_empty_tokens);

				std::string result = "";
				boost::tokenizer<boost::char_separator<char> > tokens(expr, sep);
				for ( boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
					std::string val = *it;
					if (isVariable(val)) {
						string res;
						ID id = atom.tuple[variableIndex++];
						if (id.isIntegerTerm()) {
							ostringstream os(res);
							os << id.address;
							res = os.str();
						}
						else if (id.isTerm()) {
							res = registry.terms.getByID(id).symbol;
						}
						else if (id.isOrdinaryAtom()) {
							res = registry.lookupOrdinaryAtom(id).text;
						}
						result += res;
					}
					else result += val;
				}

				// Store the replaced expression without quotes
				expressions.push_back(removeQuotes(result));
				atomIds.push_back(registry.ogatoms.getIDByTuple(atom.tuple));
			}
		}

		// Call gecode solver
		GecodeSolver* solver = new GecodeSolver(sumData, domain, globalConstraintName, globalConstraintValue, _simpleParser);
		solver->propagate(expressions);

		Gecode::Search::Options opt;
		Gecode::BAB<GecodeSolver> solutions(solver,opt);

		// If there's at least one solution, then data is consistent
		if (solutions.next()) {
			Tuple out;
			answer.get().push_back(out);
		}
		else if (nogoods != 0){ // otherwise we need to learn IIS from it
			GecodeSolver* otherSolver = new GecodeSolver(sumData, domain, globalConstraintName, globalConstraintValue, _simpleParser);
			_learningProcessor->learnNogoods(nogoods, expressions, atomIds, otherSolver);
			delete otherSolver;
		}
		delete solver;
	}
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
	void StoreID( dlvhex::Registry& registry)
	{
		_exprID=registry.storeConstantTerm("expr1");
		_notExprID=registry.storeConstantTerm("not_expr1");
		_domID=registry.storeConstantTerm("domain");
		_maxID=registry.storeConstantTerm("maximize");
		_minID=registry.storeConstantTerm("minimize");
		_idSaved=true;
	}

};


//
// A plugin must derive from PluginInterface
//
class CASPPlugin : public PluginInterface
{
private:
	boost::shared_ptr<PluginConverter> _converter;
	boost::shared_ptr<PluginRewriter> _rewriter;
	boost::shared_ptr<SimpleParser> _simpleParser;
	boost::shared_ptr<LearningProcessor> _learningProcessor;

public:

	  // stored in ProgramCtx, accessed using getPluginData<ChoicePlugin>()
	  class CtxData:
	    public PluginData
	  {
	  public:
	    /** \brief Stores if plugin is enabled. */
	    bool enabled;

	    CtxData(bool enabled_=false):enabled(enabled_){}
	    virtual ~CtxData() {};
	  };


	CASPPlugin();

	/**	Gecode::BAB<GecodeSolver> solutions(innerSolver);
				// If it is inconsistent, IIS found, break
				if (!solutions.next()) {
					processedFlags[i] = 1;
					propagateIndex = i;
					break;

	 * @brief Creates single consistency atom
	 */
	virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx&) const;

	/**
	 * @brief prints possible command line options
	 */
	virtual void printUsage(std::ostream& o) const;

//	/**
//	 * @brief Processes command line options.
//	 * The possible options include learning processor
//	 */
	virtual void processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx);


//	virtual PluginConverterPtr createConverter(ProgramCtx& ctx) {
//		return _converter;
//	}

	// create parser modules that extend and the basic hex grammar
	// this parser also stores the query information into the plugin
	virtual std::vector<HexParserModulePtr>	createParserModules(ProgramCtx& ctx);

	virtual PluginRewriterPtr createRewriter(ProgramCtx& ctx) {
		return _rewriter;
	}
};




CASPPlugin theCaspPlugin;

} // namespace casp
} // namespace dlvhex

IMPLEMENT_PLUGINABIVERSIONFUNCTION

// return plain C type s.t. all compilers and linkers will like this code
extern "C"
void * PLUGINIMPORTFUNCTION()
{
	return reinterpret_cast<void*>(& dlvhex::casp::theCaspPlugin);
}


