#include <iostream>
#include <cstddef>
#include <unistd.h>
#include <sys/time.h>
#include <map>
#include <list>
#include "trace.h"
#include "assert.h"
#include "messagequeue.h"
#include "time2.h"
#include "object.h"

using namespace std;

static	map<string, string>			global_route_map_;
static	map<string, MessageQueue*>	message_queue_map_;
static	map<string, string>			alias_map_;
static	bool						global_transaction_trace_ = true;

MessageQueue::MessageQueue(string const& _receiver_id)
: Object(), receiver_id_(_receiver_id), locker_(), queue_()
{
	map<string, MessageQueue*>::iterator it = message_queue_map_.find(receiver_id_);
	if (it != message_queue_map_.end())
	{
		throw std::logic_error("Duplicate Registration");	
	}

	message_queue_map_[receiver_id_] = this;

	TRACE_DEBUG("Message Queue for " << _receiver_id << " created.");
}

MessageQueue::~MessageQueue()
{

	map<string, MessageQueue*>::iterator it = message_queue_map_.find(receiver_id_);
	if (it != message_queue_map_.end())
	{
		TRACE_DEBUG("Remove message Queue for " << receiver_id_);
		message_queue_map_.erase(it);
	}

	while(queue_.size() > 0)
	{
		Message *message = queue_.front();
		queue_.pop_front();

		delete message;
	}
}

void MessageQueue::Push
(
	Message* _message
)
{
	locker_.Lock();

	if (_message == NULL)
	{
		TRACE_ERROR("Woops! Pushed message is NULL!");	
	}
	else
	{
		queue_.push_back(_message);
	}
	
	locker_.Unlock();
}

Message* MessageQueue::Front()
{
	locker_.Lock();
	Message*	front = queue_.front();
	locker_.Unlock();

	return	front;
}

Message* MessageQueue::Pop
(
	bool _do_release
)
{
	locker_.Lock();

	Message *message = NULL;

	if (queue_.size() != 0)
	{
		message = queue_.front();
		queue_.pop_front();

		if (message != NULL)
		{
			if (_do_release)
			{
				delete message;
				message = NULL;
			}
		}
		else
		{
			TRACE_ERROR("Queue element is NULL!!!");	
			TRACE_ERROR("Queue Size : " << queue_.size());
		}
	}

	locker_.Unlock();
	return	message;
}

uint32_t	MessageQueue::Count()
{
	locker_.Lock();
	uint32_t	size = queue_.size();
	locker_.Unlock();

	return	size;
}

bool MessageQueue::TimedWait
(
	int	_milliseconds
)
{
	struct timeval	timer;
	struct timeval	expired_time;
	struct timeval	timeout;

	timeout.tv_sec = _milliseconds / 1000;
	timeout.tv_usec = _milliseconds % 1000 * 1000;

	gettimeofday(&timer, NULL);
	timeradd(&timer, &timeout, &expired_time);

	while(timercmp(&timer, &expired_time, <))
	{
		if (queue_.size() != 0)
		{
			return	true;	
		}

		usleep(Time::MILLISECOND);	
		gettimeofday(&timer, NULL);
	}

	return	false;
}

bool	MessageQueue::ChangeReceiverID(std::string const& _receiver_id)
{
	map<string, MessageQueue*>::iterator it = message_queue_map_.find(receiver_id_);
	if (it != message_queue_map_.end())
	{
		TRACE_DEBUG("Remove message Queue for " << receiver_id_);
		message_queue_map_.erase(it);
	}

	TRACE_DEBUG("Message Queue has been renamed from " << receiver_id_ << " to " << _receiver_id);

	receiver_id_ = _receiver_id;
	message_queue_map_[receiver_id_] = this;

	return	true;
}

bool	MessageQueue::AddAlias(std::string const& _alias_id)
{
	map<string, string>::iterator it = alias_map_.find(_alias_id);
	if (it != alias_map_.end())
	{
		TRACE_WARN("It is already used as Alias for [ " << it->second << " ]");
		return	false;	
	}

	alias_map_[_alias_id] = receiver_id_;
	TRACE_DEBUG(_alias_id << "is registered as an alias for " << receiver_id_);

	return	true;
}

bool	MessageQueue::RemoveAlias(std::string const& _alias_id)
{
	map<string, string>::iterator it = alias_map_.find(_alias_id);
	if (it != alias_map_.end())
	{
		TRACE_DEBUG(_alias_id << "has been removed from alias for " << it->second);
		alias_map_.erase(it);
	}

	return	true;	
}

///////////////////////////////////////////////////////////////////
//	Static Functions
///////////////////////////////////////////////////////////////////
bool	MessageQueue::AddAlias(std::string const& _receiver_id, std::string const& _alias_id)
{
	map<string, string>::iterator it = alias_map_.find(_alias_id);
	if (it != alias_map_.end())
	{
		TRACE_WARN2(NULL, "It is already used as Alias for [ " << it->second << " ]");
		return	false;	
	}

	alias_map_[_alias_id] = _receiver_id;
	TRACE_DEBUG2(NULL, _alias_id << "is registered as an alias for " << _receiver_id);

	return	true;
}


bool	MessageQueue::Post(Message* _message)
{
	ASSERT(_message);

	std::string	receiver = _message->GetReceiver();
	std::string	sender = _message->GetSender();

	if (receiver == "") 
	{
		receiver = global_route_map_[sender];
		if (receiver == "")
		{
			TRACE_WARN2(NULL, "Message target unknown");
			return	false;	
		}
	}

	auto it = message_queue_map_.find(receiver);
	if (it == message_queue_map_.end())
	{
		map<string, string>::iterator alias_it = alias_map_.find(receiver);
		if (alias_it == alias_map_.end())
		{
			TRACE_WARN2(NULL, "MSG[" << _message->GetID() << "] : Message queue of receiver [" << receiver << "] not found");
			return	false;
		}

		it = message_queue_map_.find(alias_it->second);
		if (it == message_queue_map_.end())
		{
			TRACE_WARN2(NULL, "MSG[" << _message->GetID() << "] : Receiver [" << receiver << "] not found");
			return	false;
		}
	}

	if (global_transaction_trace_)
	{
		TRACE_DEBUG2(NULL, "MSG[" << _message->GetID() << "] : " << sender << " -> " << receiver);
	}

	it->second->Push(_message);

	return	true;
}
