#ifndef CASPCONVERTER_H_
#define CASPCONVERTER_H_

#include "dlvhex2/PluginInterface.h"
#include <utility>

class DefaultConverter : public dlvhex::PluginConverter {
public:
	DefaultConverter();
	virtual ~DefaultConverter();

	virtual void convert(std::istream& i, std::ostream& o);
};



class CaspConverter : public dlvhex::PluginConverter {
public:
	CaspConverter(bool addGuessToHead);
	virtual ~CaspConverter();

	virtual void convert(std::istream& i, std::ostream& o);
private:
	bool _addGuessToHead;
};


#endif /* CASPCONCERTER_H_ */
