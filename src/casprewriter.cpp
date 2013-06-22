#include "casp/casprewriter.h"

#include "dlvhex2/PluginInterface.h"
#include "dlvhex2/Registry.h"
#include "dlvhex2/ProgramCtx.h"

#include <boost/algorithm/string/predicate.hpp>

#include <vector>

using namespace std;
using namespace dlvhex;

DefaultRewriter::DefaultRewriter() {
}

DefaultRewriter::~DefaultRewriter() {
}

void DefaultRewriter::rewrite(ProgramCtx& ctx) {
}

CaspRewriter::CaspRewriter(bool addGuessToHead=false) {
	_addGuessToHead = addGuessToHead;
}

CaspRewriter::~CaspRewriter() {
}

void CaspRewriter::rewriteRule(ProgramCtx& ctx, vector<ID>& idb, ID ruleID) {
	RegistryPtr reg = ctx.registry();
	const Rule& rule = reg->rules.getByID(ruleID);

	bool needRewrite = false;
	BOOST_FOREACH(ID atomId, rule.body) {
		if (!(atomId.isExternalAtom()) && !(atomId.isBuiltinAtom())) {
			const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(atomId);
			if (boost::starts_with(atom.text, "expr")) {
				needRewrite = true;
				break;
			}
		}
	}

	if (_addGuessToHead) {
		BOOST_FOREACH(ID atomId, rule.head) {
			if (!(atomId.isExternalAtom()) && !(atomId.isBuiltinAtom())) {
				const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(atomId);
				if (boost::starts_with(atom.text, "expr")) {
					needRewrite = true;
					break;
				}
			}
		}
	}


	if (needRewrite) {
		vector<ID> bodyWithoutCasp;
		vector<ID> caspIds;

		if (_addGuessToHead) {
			BOOST_FOREACH (ID headAtomId, rule.head) {
				if (headAtomId.isOrdinaryAtom()) {
					const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(headAtomId);

					if (boost::starts_with(atom.text, "expr")) {
						caspIds.push_back(ID(headAtomId));
					}
				}
			}
		}


		BOOST_FOREACH (ID bodyAtomId, rule.body) {
			if (bodyAtomId.isOrdinaryAtom()) {
				const OrdinaryAtom& atom = reg->lookupOrdinaryAtom(bodyAtomId);

				if (boost::starts_with(atom.text, "expr")) {
					caspIds.push_back(ID(bodyAtomId));

				}
				else {
					bodyWithoutCasp.push_back(ID(bodyAtomId));
				}
			}
			else {
				bodyWithoutCasp.push_back(ID(bodyAtomId));
			}
		}

		BOOST_FOREACH (ID caspId, caspIds) {
			Rule newRule = Rule(rule);
			newRule.kind = ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR | ID::PROPERTY_RULE_DISJ;

			newRule.body = bodyWithoutCasp;

			newRule.head.clear();
			if (caspId.isLiteral()) {
				newRule.head.push_back(ID::posLiteralFromAtom(ID::atomFromLiteral(caspId)));
			}
			else {
				newRule.head.push_back(ID::posLiteralFromAtom(caspId));
			}	

			OrdinaryAtom auxCaspAtom = reg->lookupOrdinaryAtom(caspId);
			Term t = reg->terms.getByID(auxCaspAtom.tuple[0]);

			string negatedSymbol = "not_" + t.symbol;
			ID negatedId = reg->terms.getIDByString(negatedSymbol);
			if (negatedId == ID_FAIL) {
				t.symbol = negatedSymbol;

				auxCaspAtom.tuple[0] = reg->terms.storeAndGetID(t);
			} else {
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
