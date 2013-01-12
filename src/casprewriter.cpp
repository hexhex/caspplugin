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
	BOOST_FOREACH (ID headAtomId, rule.head) {
		const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(headAtomId);
		if (boost::starts_with(atom.text, "expr")) needRewrite = true;
	}
	BOOST_FOREACH (ID bodyAtomId, rule.body) {
		if (!(bodyAtomId.isExternalAtom())) {
			const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(bodyAtomId);
			if (boost::starts_with(atom.text, "expr"))
				needRewrite = true;
		}
	}

	if (needRewrite) {
		// Move head to body
		if (rule.head.size() > 1) throw PluginError("Constraints in disjunctive head are not implemented.");

		vector<ID> newBody = rule.body;

		if (rule.head.size() != 0) {
			ID nafLiteral = ID::nafLiteralFromAtom(rule.head[0]);
			newBody.push_back(nafLiteral);
		}

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

			t.symbol = "not_" + t.symbol;

			auxCaspAtom.tuple[0] = reg->terms.storeAndGetID(t);

			ID hid = reg->storeOrdinaryAtom(auxCaspAtom);

			newRule.head.push_back(hid);

			ID newRuleID = reg->storeRule(newRule);
			idb.push_back(newRuleID);
		}
	}
	else
		idb.push_back(ruleID);
}

void CaspRewriter::rewrite(ProgramCtx& ctx) {
	BOOST_FOREACH (ID rid, ctx.idb) {
		rewriteRule(ctx, newIdb, rid);
	}
	ctx.idb = newIdb;
}
