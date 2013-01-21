#include "config.h"

#include "casp/gecodesolver.h"
#include "casp/caspconverter.h"
#include "casp/casprewriter.h"
#include "casp/learningprocessor.h"

#include <dlvhex2/PluginInterface.h>
#include <dlvhex2/ProgramCtx.h>
#include <dlvhex2/Term.h>
#include <dlvhex2/Registry.h>
#include <dlvhex2/OrdinaryAtomTable.h>
#include <dlvhex2/Table.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <utility>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

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
			ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor) :
				PluginAtom( "casp", 0),
				_learningProcessor(learningProcessor)
			{
				// This add predicates for all input parameters
				for (int i = 0; i < 30; i++)
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
				assert(nogoods != 0);
				Registry &registry = *getRegistry();

				std::vector<std::string> expressions;
				std::vector<std::string> sumData;
				std::string domain = "";
				std::string globalConstraintName = "";
				std::string globalConstraintValue = "";

				std::pair<Interpretation::TrueBitIterator, Interpretation::TrueBitIterator>
					trueAtoms = query.interpretation->trueBits();

				vector<ID> atomIds;

				// Iterate over all input data
				for (Interpretation::TrueBitIterator it = trueAtoms.first; it != trueAtoms.second; it++) {
					const OrdinaryAtom &atom = query.interpretation->getAtomToBit(it);

					Term name = registry.terms.getByID(atom.tuple[0]);

					// Store in local variables input data in predicates
					std::string expr = "";
					if (boost::starts_with(name.symbol,"expr")) {
						Term value = registry.terms.getByID(atom.tuple[1]);
						expr = value.symbol;
					}
					else if (boost::starts_with(name.symbol,"not_expr")) {
						Term value = registry.terms.getByID(atom.tuple[1]);
						expr = replaceInvertibleOperator(value.symbol);
					}
					else if (name.symbol == "dom") {
						domain = removeQuotes(registry.terms.getByID(atom.tuple[1]).symbol);
					}
					else if (name.symbol == "maximize" || name.symbol == "minimize") {
						globalConstraintName = name.symbol;

						globalConstraintValue = removeQuotes(registry.terms.getByID(atom.tuple[1]).symbol);
					}
					else { // this predicate received as input to sum aggregate function
						sumData.push_back(atom.text);
					}
					if (expr != "") { // handle expr and not_expr
						// Replace all ASP variables with their actual values.
						int variableIndex = 2;

						boost::char_separator<char> sep(" ", ",v.:-$%<>=+-/*\"<>=", boost::drop_empty_tokens);

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
								else
									res = registry.terms.getByID(id).symbol;
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
				GecodeSolver* solver = new GecodeSolver(sumData, domain, globalConstraintName, globalConstraintValue);
				solver->propagate(expressions);
				Gecode::BAB<GecodeSolver> solutions(solver);

				// If there's at least one solution, then data is consistent
				if (solutions.next()) {
					Tuple out;
					answer.get().push_back(out);
				}
				else { // otherwise we need to learn IIS from it
					GecodeSolver* otherSolver = new GecodeSolver(sumData, domain, globalConstraintName, globalConstraintValue);
					_learningProcessor->learnNogoods(nogoods, expressions, atomIds, otherSolver);
					delete otherSolver;
				}
				delete solver;
			}
		private:
			boost::shared_ptr<LearningProcessor> _learningProcessor;

			/**
			 * @brief This helper method is used to check whether string is a variable
			 */
			bool isVariable(string s) {
				bool res = true;
				if (s[0] >= 'A' && s[0] <= 'Z') {
					for (int i = 1; i < s.length(); i++) {
						if (!isalnum(s[i]) && s[i] != '_') {
							res = false;
							break;
						}
					}
				}
				else res = false;
				return res;
			}
			string removeQuotes(string s) {
				return s.substr(1, s.length() - 2);
			}
			/**
			 * This method is used to change the operator to invertible one
			 * in "not_expr" terms, e.g. not_expr("3>5") becomes expr("3<=5").
			 */
			string replaceInvertibleOperator(string expr) {
				typedef pair<string, string> string_pair;

				vector<string_pair> invertibleOperators;

				invertibleOperators.push_back(make_pair("==", "!="));
				invertibleOperators.push_back(make_pair("!=", "=="));
				invertibleOperators.push_back(make_pair(">=", "<"));
				invertibleOperators.push_back(make_pair("<=", ">"));
				invertibleOperators.push_back(make_pair(">", "<="));
				invertibleOperators.push_back(make_pair("<", ">="));

				BOOST_FOREACH (string_pair invertibleOperator, invertibleOperators) {
					if (expr.find (invertibleOperator.first) != string::npos) {
						boost::replace_all(expr, invertibleOperator.first, invertibleOperator.second);
						break;
					}
				}

				return expr;
			}
	};
    
	//
	// A plugin must derive from PluginInterface
	//
	class CASPPlugin : public PluginInterface
    {
		private:
			boost::shared_ptr<CaspConverter> _converter;
			boost::shared_ptr<CaspRewriter> _rewriter;
			boost::shared_ptr<LearningProcessor> _learningProcessor;

		public:
      
    		CASPPlugin() :
    			_converter(new CaspConverter()),
    			_rewriter(new CaspRewriter()),
    			_learningProcessor(new BackwardLearningProcessor())
			{
				setNameVersion(PACKAGE_TARNAME,CASPPLUGIN_VERSION_MAJOR,CASPPLUGIN_VERSION_MINOR,CASPPLUGIN_VERSION_MICRO);
			}
		
			virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx&) const
			{
				std::vector<PluginAtomPtr> ret;
			
				ret.push_back(PluginAtomPtr(new ConsistencyAtom(_learningProcessor), PluginPtrDeleter<PluginAtom>()));
			
				return ret;
			}
      
			virtual void 
			processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx)
			{
			
			}

			virtual PluginConverterPtr createConverter(ProgramCtx& ctx) {
				return _converter;
			}

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


