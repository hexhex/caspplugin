#ifndef CASPREWRITER_H
#define CASPREWRITER_H

#include "dlvhex2/PluginInterface.h"
#include <vector>

class CaspRewriter : public dlvhex::PluginRewriter {
public:
	CaspRewriter();
	virtual ~CaspRewriter();

	virtual void rewrite(dlvhex::ProgramCtx& ctx);
private:
	std::vector<dlvhex::ID> newIdb;

	void rewriteRule(dlvhex::ProgramCtx& ctx, std::vector<dlvhex::ID>& idb, dlvhex::ID ruleID);
};



#endif /* CASPCONVERTER_CPP_ */
