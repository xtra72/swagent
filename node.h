#ifndef	NODE_H_
#define	NODE_H_

#include "cp211x.h"

class	Node : public CP2110
{
public:
	Node();
	Node(std::string const& _id, Object* _parent = NULL);

	virtual	bool	OnRead(uint8_t* _data, uint32_t _length);
protected:
};
#endif

