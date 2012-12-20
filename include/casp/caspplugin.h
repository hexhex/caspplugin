/*
 * caspplugin.h
 *
 *  Created on: Dec 11, 2012
 *      Author: faeton
 */

#ifndef CASPPLUGIN_H_
#define CASPPLUGIN_H_

#include "casp/caspconverter.h"

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/PluginInterface.h"
#include "dlvhex2/HexParserModule.h"
#include "dlvhex2/ProgramCtx.h"

DLVHEX_NAMESPACE_BEGIN

class CaspPlugin: public PluginInterface {
private:
	boost::shared_ptr<CaspConverter> converter;
public:
	CaspPlugin();
	virtual ~CaspPlugin();

	/**
	 * @brief output help message for this plugin
	 */
	virtual void printUsage(std::ostream& o) const;

	/**
	 * @brief processes options for this plugin, and removes recognized options from pluginOptions
	 *
	 * (do not free the pointers, the const char* directly come from argv)
	 *
	 * accepted options: --action-enable
	 * 					 --acthexNumberIterations
	 * 					 --acthexDurationIterations
	 */
	virtual void processOptions(std::list<const char*>& pluginOptions,
			ProgramCtx&);

	/**
	 * @brief creates parser modules for Actions and Built-in Declarations
	 *
	 * extend and the basic hex grammar; this parser also stores the query information into the plugin
	 */
	virtual std::vector<HexParserModulePtr> createParserModules(ProgramCtx&);

	virtual PluginConverterPtr createConverter(ProgramCtx& ctx);

	virtual PluginRewriterPtr createRewriter(ProgramCtx& ctx);

	/**
	 * @brief setup CtxData, myAuxiliaryPredicateMask, ModelCallback and FinalCallback
	 */
	virtual void setupProgramCtx(ProgramCtx&);
};

DLVHEX_NAMESPACE_END


#endif /* CASPPLUGIN_H_ */
