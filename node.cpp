#include "node.h"
#include "message.h"

Node::Node()
: CP2110("node")
{
}

Node::Node(std::string const& _id, Object* _parent)
: CP2110(_id, _parent)
{
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
