#ifndef CASPREWRITER_H
#define CASPREWRITER_H

#include "dlvhex2/PluginInterface.h"
#include <vector>

class DefaultRewriter : public dlvhex::PluginRewriter {
public:
	DefaultRewriter();
	virtual ~DefaultRewriter();

	virtual void rewrite(dlvhex::ProgramCtx& ctx);
};


class CaspRewriter : public dlvhex::PluginRewriter {
public:
	CaspRewriter(bool addGuessToHead);
	virtual ~CaspRewriter();

	virtual void rewrite(dlvhex::ProgramCtx& ctx);
private:
	bool _addGuessToHead;
	std::vector<dlvhex::ID> newIdb;

	void rewriteRule(dlvhex::ProgramCtx& ctx, std::vector<dlvhex::ID>& idb, dlvhex::ID ruleID);
};



#endif /* CASPCONVERTER_CPP_ */
