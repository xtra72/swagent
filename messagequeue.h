#ifndef	MESSAGE_QUEUE_H__
#define	MESSAGE_QUEUE_H__

#include <queue>
#include <list>
#include "locker.h"
#include "object.h"
#include "message.h"

class	ActiveObject;
class	MessageQueue : public Object
{
public:
	MessageQueue(std::string const& _receiver_id);
	~MessageQueue();

			void		Push(Message *_message);
			Message*	Pop(bool _do_relese =false);
			Message*	Front();
			uint32_t	Count();

			bool 	TimedWait(int _milliseconds);

			bool	ChangeReceiverID(std::string const& _receiver_id);
	
			bool	AddAlias(std::string const& _alias_id);
			bool	RemoveAlias(std::string const& _alias_id);

	static	bool	AddAlias(std::string const& _receiver_id, std::string const& _alias_id);

	static	bool	DefaultReceiver(ActiveObject* _object);
	static	bool	Post(Message* _message);

private:
	std::string				receiver_id_;
	Locker					locker_;
	std::list<Message *>	queue_;

};

#endif
