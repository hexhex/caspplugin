#include "config.h"
#include "casp/gecodesolver.h"

#include <dlvhex2/PluginInterface.h>
#include <dlvhex2/Term.h>
#include <dlvhex2/Registry.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <sstream>
#include <cstdio>

namespace dlvhex {
  namespace casp {

    class ConsistencyAtom : public PluginAtom
    {
		public:
			ConsistencyAtom() : PluginAtom("cons", 1)
			{
				addInputConstant();
				
				setOutputArity(1);
			}
      
			virtual void
			retrieve(const Query& query, Answer& answer) throw (PluginError)
			{
				Registry &registry = *getRegistry();
				const Term& term = registry.terms.getByID(query.input[0]);

				GecodeSolver* solver = new GecodeSolver(term.getQuotedString().c_str());
				Gecode::DFS<GecodeSolver> solutions(solver);

				std::string res;
				if (solutions.next()) {
					res = "ok";
				}
				else
					res = "failed";

				Tuple out;
				Term newterm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, '"' + res + '"');
				out.push_back(registry.storeTerm(newterm));
				answer.get().push_back(out);
			}
	};
    
	//
	// A plugin must derive from PluginInterface
	//
	class CASPPlugin : public PluginInterface
    {
		public:
      
    		CASPPlugin()
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


