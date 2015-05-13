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
#ifndef LEARNINGPROCESSOR_H_
#define LEARNINGPROCESSOR_H_

#include <dlvhex2/Nogood.h>
#include <vector>
#include <string>

#include "casp/GecodeSolver.h"

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
