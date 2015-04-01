#ifndef LEARNINGPROCESSOR_H_
#define LEARNINGPROCESSOR_H_

#include <dlvhex2/Nogood.h>
#include <vector>
#include <string>

#include "casp/gecodesolver.h"

using namespace dlvhex;
using namespace std;

class LearningProcessor {
protected:
	boost::shared_ptr<SimpleParser> _simpleParser;
public:
	LearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
		_simpleParser(simpleParser) {}
	virtual ~LearningProcessor() {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
			vector<ID>& atomIds, GecodeSolver *solver) = 0;
};

class NoLearningProcessor : public LearningProcessor {
public:
	NoLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
			LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class DeletionLearningProcessor : public LearningProcessor {
public:
	DeletionLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class ForwardLearningProcessor : public LearningProcessor {
public:
	ForwardLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class JumpForwardLearningProcessor : public LearningProcessor {
public:
	JumpForwardLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class BackwardLearningProcessor : public LearningProcessor {
public:
	BackwardLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class RangeLearningProcessor : public LearningProcessor {
public:
	RangeLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class CCLearningProcessor : public LearningProcessor {
public:
	CCLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

class WeightedCCLearningProcessor : public LearningProcessor {
public:
	WeightedCCLearningProcessor(boost::shared_ptr<SimpleParser> simpleParser):
				LearningProcessor(simpleParser) {}
	virtual void learnNogoods(NogoodContainerPtr& nogoods, vector<string>& expressions,
				vector<ID>& atomIds, GecodeSolver *solver);
};

#endif
