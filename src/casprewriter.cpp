#include "casp/casprewriter.h"

#include "dlvhex2/PluginInterface.h"
#include "dlvhex2/Registry.h"
#include "dlvhex2/ProgramCtx.h"

#include <boost/algorithm/string/predicate.hpp>

#include <vector>

using namespace std;
using namespace dlvhex;


CaspRewriter::CaspRewriter() {

}

CaspRewriter::~CaspRewriter() {

}

void CaspRewriter::rewriteRule(ProgramCtx& ctx, vector<ID>& idb, ID ruleID) {
	RegistryPtr reg = ctx.registry();
	const Rule& rule = reg->rules.getByID(ruleID);

	bool needRewrite = false;
	BOOST_FOREACH(ID atomId, rule.head) {
		const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(atomId);
		if (boost::starts_with(atom.text, "expr"))
			needRewrite = true;
	}
	BOOST_FOREACH(ID atomId, rule.body) {
		if (!(atomId.isExternalAtom()) && !(atomId.isBuiltinAtom())) {
			const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(atomId);
			if (boost::starts_with(atom.text, "expr"))
				needRewrite = true;
		}
	}

	if (needRewrite) {
		vector<ID> newBody = rule.body;

		vector<ID> bodyWithoutCasp;
		vector<ID> caspIds;

		BOOST_FOREACH (ID bodyAtomId, newBody) {
			const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(bodyAtomId);
			if (boost::starts_with(atom.text, "expr"))
				caspIds.push_back(bodyAtomId);
			else
				bodyWithoutCasp.push_back(bodyAtomId);
		}

		BOOST_FOREACH (ID caspId, caspIds) {
			Rule newRule = rule;
			newRule.kind = ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR;

			newRule.body = bodyWithoutCasp;

			newRule.head.clear();
			newRule.head.push_back(ID::posLiteralFromAtom(ID::atomFromLiteral(caspId)));

			OrdinaryAtom auxCaspAtom = reg->lookupOrdinaryAtom(caspId);

			Term t = reg->terms.getByID(auxCaspAtom.tuple[0]);

			string negatedSymbol = "not_" + t.symbol;
			ID negatedId = reg->terms.getIDByString(negatedSymbol);
			if (negatedId == ID_FAIL) {
				t.symbol = negatedSymbol;

				auxCaspAtom.tuple[0] = reg->terms.storeAndGetID(t);
			}
			else {
				auxCaspAtom.tuple[0] = negatedId;
			}

			ID hid = reg->storeOrdinaryAtom(auxCaspAtom);

			newRule.head.push_back(hid);

			ID newRuleID = reg->storeRule(newRule);
			idb.push_back(newRuleID);
		}
	}
	idb.push_back(ruleID);
}

void CaspRewriter::rewrite(ProgramCtx& ctx) {
	BOOST_FOREACH (ID rid, ctx.idb) {
		rewriteRule(ctx, newIdb, rid);
	}
	ctx.idb = newIdb;
}
