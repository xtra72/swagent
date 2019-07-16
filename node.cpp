#include "node.h"
#include "message.h"

Node::Node()
: CP2110()
{
}

Node::Node(std::string const& _id, Object* _parent)
: CP2110(_id, _parent), channel_count_(8)
{
	properties_map_["channel_count"] = SetChannelCount;
}

uint32_t	Node::GetChannelCount(void)
{
		return	channel_count_;
}

bool	Node::SetChannelCount(uint32_t _channel_count)
{
	channel_count_ = _channel_count;

	return	true;
}

bool	Node::Active(void)
{
	const char*	cmd = "AT+START:1\n";
	
	OnWrite((uint8_t *)cmd, 12);
	return	true;
}

bool	Node::Sleep(void)
{
	const char*	cmd = "AT+STOP:1\n";
	
	OnWrite((uint8_t *)cmd, 11);
	return	true;
}

bool	Node::OnRead(uint8_t* _data, uint32_t _length)
{
	if (parent_ != NULL)
	{
		ActiveObject*	active_object = dynamic_cast<ActiveObject*>(parent_);
		if (active_object)
		{
			active_object->Post(new MessagePacketReceived(parent_->GetID(), id_, _data, _length));
		}
		else
		{
			TRACE_INFO("Parent is not active object!");
		}
	}
	else
	{
		TRACE_INFO("Parent is not exist!");
	}

	return	true;
}

bool	Node::SetChannelCount(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetChannelCount(_value.as_int());

	return	true;
}
