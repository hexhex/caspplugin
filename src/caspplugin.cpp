#include "config.h"

#include "casp/gecodesolver.h"
#include "casp/caspconverter.h"

#include <dlvhex2/PluginInterface.h>
#include <dlvhex2/Term.h>
#include <dlvhex2/Registry.h>
#include <dlvhex2/OrdinaryAtomTable.h>
#include <dlvhex2/Table.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <string>
#include <sstream>
#include <cstdio>
#include <utility>

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace dlvhex {
  namespace casp {

    class ConsistencyAtom : public PluginAtom
    {
		public:
			ConsistencyAtom() : PluginAtom( "casp", 1)
			{
				addInputConstant();
				
				setOutputArity(0);
			}
      
			virtual void
			retrieve(const Query& query, Answer& answer) throw (PluginError)
			{
				Registry &registry = *getRegistry();

				registry.print(std::cout);

				const Term& expressionTerm = registry.terms.getByID(query.input[0]);
				const Term& domainTerm = registry.terms.getByID(query.input[1]);

				std::pair<OrdinaryAtomTable::AddressIterator, OrdinaryAtomTable::AddressIterator> all = registry.ogatoms.getAllByAddress();
				OrdinaryAtomTable::AddressIterator begin = all.first;
				OrdinaryAtomTable::AddressIterator end = all.second;

				std::vector<std::string> expressions;
				std::string domain;

				for (OrdinaryAtomTable::AddressIterator it = begin; it != end; it++) {
					const OrdinaryAtom &atom = *it;
					Term name = registry.terms.getByID(atom.tuple[0]);
					if (name.symbol == expressionTerm.symbol) {
						Term value = registry.terms.getByID(atom.tuple[1]);
						expressions.push_back(value.symbol);
					}
					if (name.symbol == domainTerm.symbol) {
						Term value = registry.terms.getByID(atom.tuple[1]);
						domain = value.symbol;
					}
				}

				GecodeSolver* solver = new GecodeSolver(expressions, domain);
				Gecode::DFS<GecodeSolver> solutions(solver);

				if (solutions.next()) {
					Tuple out;
					answer.get().push_back(out);
				}
			}
	};
    
	//
	// A plugin must derive from PluginInterface
	//
	class CASPPlugin : public PluginInterface
    {
		private:
			boost::shared_ptr<CaspConverter> converter;

		public:
      
    		CASPPlugin() : converter(new CaspConverter())
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


