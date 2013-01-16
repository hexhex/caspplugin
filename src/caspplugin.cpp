#include "config.h"

#include "casp/gecodesolver.h"
#include "casp/caspconverter.h"
#include "casp/casprewriter.h"

#include <dlvhex2/PluginInterface.h>
#include <dlvhex2/Term.h>
#include <dlvhex2/Registry.h>
#include <dlvhex2/OrdinaryAtomTable.h>
#include <dlvhex2/Table.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

namespace dlvhex {
  namespace casp {

    class ConsistencyAtom : public PluginAtom
    {
		public:
			ConsistencyAtom() : PluginAtom( "casp", 1)
			{
				for (int i = 0; i < 30; i++)
					addInputPredicate();
				
				setOutputArity(0);
			}
      
			virtual void
			retrieve(const Query& query, Answer& answer) throw (PluginError)
			{
				Registry &registry = *getRegistry();

				std::vector<std::string> expressions;
				std::vector<std::string> sumData;
				std::string domain = "";
				std::string globalConstraintName = "";
				std::string globalConstraintValue = "";

				std::pair<Interpretation::TrueBitIterator, Interpretation::TrueBitIterator>
					trueAtoms = query.interpretation->trueBits();

				for (Interpretation::TrueBitIterator it = trueAtoms.first; it != trueAtoms.second; it++) {
					const OrdinaryAtom &atom = query.interpretation->getAtomToBit(it);

					Term name = registry.terms.getByID(atom.tuple[0]);

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
									res = registry.terms.getByID(atom.tuple[variableIndex]).symbol;
								result += res;
							}
							else result += val;
						}

						expressions.push_back(removeQuotes(result));
					}
				}

				// Call gecode solver
				GecodeSolver* solver = new GecodeSolver(expressions, sumData,
						domain, globalConstraintName, globalConstraintValue);
				Gecode::DFS<GecodeSolver> solutions(solver);

				// If there's at least one solution, then data is consistent
				if (solutions.next()) {
					Tuple out;
					answer.get().push_back(out);
				}
			}
		private:
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
			boost::shared_ptr<CaspConverter> converter;
			boost::shared_ptr<CaspRewriter> rewriter;

		public:
      
    		CASPPlugin() : converter(new CaspConverter()), rewriter(new CaspRewriter())
			{
				setNameVersion(PACKAGE_TARNAME,CASPPLUGIN_VERSION_MAJOR,CASPPLUGIN_VERSION_MINOR,CASPPLUGIN_VERSION_MICRO);
			}
		
			virtual std::vector<PluginAtomPtr> createAtoms(ProgramCtx&) const
			{
				std::vector<PluginAtomPtr> ret;
			
				ret.push_back(PluginAtomPtr(new ConsistencyAtom, PluginPtrDeleter<PluginAtom>()));
			
				return ret;
			}
      
			virtual void 
			processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx)
			{
			
			}

			virtual PluginConverterPtr createConverter(ProgramCtx& ctx) {
				return converter;
			}

			virtual PluginRewriterPtr createRewriter(ProgramCtx& ctx) {
				return rewriter;
			}
	};
    
    
	CASPPlugin theCaspPlugin;
    
  } // namespace casp
} // namespace dlvhex

//
// let it be loaded by dlvhex!
//

IMPLEMENT_PLUGINABIVERSIONFUNCTION

// return plain C type s.t. all compilers and linkers will like this code
extern "C"
void * PLUGINIMPORTFUNCTION()
{
	return reinterpret_cast<void*>(& dlvhex::casp::theCaspPlugin);
}


