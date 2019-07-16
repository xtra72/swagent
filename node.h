#ifndef	NODE_H_
#define	NODE_H_

#include "cp211x.h"

class	Node : public CP2110
{
public:
	Node();
	Node(std::string const& _id, Object* _parent = NULL);

			uint32_t	GetChannelCount(void);
			bool		SetChannelCount(uint32_t _channel_count);

			bool		Active(void);
			bool		Sleep(void);
	virtual	bool		OnRead(uint8_t* _data, uint32_t _length);

protected:
	uint16_t	channel_count_;

	static	bool	SetChannelCount(Object* _object, JSONNode const& _value);
};
#endif

