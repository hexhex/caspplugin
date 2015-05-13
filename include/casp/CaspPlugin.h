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
#include "config.h"

#include "casp/ConsistencyAtom.h"
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
#include <boost/lexical_cast.hpp>

#define NDEBUG

namespace dlvhex {
namespace casp {

//
// A plugin must derive from PluginInterface
//
class CASPPlugin : public PluginInterface
{
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

	/**
	 * @brief Processes command line options.
	 * The possible options include learning processor
	 */
	virtual void processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx);



	// create parser modules that extend and the basic hex grammar
	// this parser also stores the query information into the plugin
	virtual std::vector<HexParserModulePtr>	createParserModules(ProgramCtx& ctx);

private:
	boost::shared_ptr<SimpleParser> _simpleParser;
	boost::shared_ptr<LearningProcessor> _learningProcessor;

	bool cspGraphLearning;
	bool cspAnticipateLearning;

	CPVariableAndConnection cpVariableAndConnection;

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


