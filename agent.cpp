#include <sstream>
#include <string>
#include "assert.h"
#include "agent.h"
#include "utils.h"

Agent::MessageData::MessageData(std::string const& _receiver, std::string const& _sender, uint32_t _time, uint16_t* _data, uint32_t _length)
: Message(110, _receiver, _sender), time_(_time), data_(NULL), length_(_length)
{
	if (_length != 0)
	{
		data_ = new uint16_t[_length];
		memcpy(data_, _data, sizeof(uint16_t) * _length);
	}
}

Agent::MessageData::~MessageData()
{
	if (data_ != NULL)
	{
		delete [] data_;
	}
}

uint32_t	Agent::MessageData::GetTime(void)
{
	return	time_;
}

uint16_t	Agent::MessageData::GetValue(uint32_t index)
{
	if (index < length_)
	{
		return	data_[index];
	}

	return	0;
}

Agent::ServerRequestReceiver::ServerRequestReceiver(Agent* _parent, std::string const& _topic) 
: MQTTClient::Subscriber(_topic) 
{
	parent_ = _parent;
}

	
void	Agent::ServerRequestReceiver::OnMessage(std::string const& _message)
{
	if (parent_ != NULL)
	{
		Agent*	agent = dynamic_cast<Agent*>(parent_);

		agent->OnServerRequest(_message);
	}
}

Agent::Agent(std::string const& _id)
: ActiveObject(_id), client_(_id + ".client")
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	message_handler_map_[110] 	= ReceiveData;
}

Agent::Agent()
: ActiveObject("agent"), client_("client")
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	message_handler_map_[110] 	= ReceiveData;
}

void	Agent::Preprocess()
{
	for(auto node = node_list_.begin(); node != node_list_.end() ; node++)
	{
		(*node)->Start();
	}
	client_.Start();

	std::ostringstream	oss;

	oss << "cwr/mfl/" << id_ << "/command/req";
	ServerRequestReceiver*	receiver = new ServerRequestReceiver(this, oss.str());
	if (receiver)
	{
		client_.Register(receiver);
	}
}

void	Agent::Process()
{
	ActiveObject::Process();
}

void	Agent::Postprocess()
{
	client_.Stop();
	for(auto node = node_list_.begin(); node != node_list_.end() ; node++)
	{
		(*node)->Stop();
	}
}

bool	Agent::OnReceived(Message* _message)
{
	char rxBuffer[1024];
	MessagePacketReceived * packet = dynamic_cast<MessagePacketReceived*>(_message);

	memcpy(rxBuffer, packet->GetData(), packet->GetSize());
	rxBuffer[packet->GetSize()] = 0;

	char*	token = strtok(rxBuffer, ":");
	if (!token)
	{
		TRACE_WARN("Invalid format.");
		return	false;
	}

	if (strcasecmp(token, "+DATA") == 0)
	{
		char*	time_field = strtok(NULL, ",");
		if (!time_field)
		{
			TRACE_WARN("Invalid format.");
			return	false;
		}

		char*	length_field = strtok(NULL, ",");
		if (!time_field)
		{
			TRACE_WARN("Invalid format.");
			return	false;
		}

		try
		{
			uint32_t	time = (uint32_t)std::stof(time_field, NULL);
			uint32_t	length = strtoul(length_field, NULL, 10);

			if ((length > 12) && ((length - 12) % 6 == 0))
			{
				token = strtok(NULL, " ");
				if (strlen(token) != length*2)
				{
					std::ostringstream	oss;
					oss << "Invalid format : " << strlen(token) << ", " << length;
					TRACE_WARN(oss.str());
					return	false;
				}

				uint8_t	data[128];

				if (!StringToUint8(token, data, sizeof(data), length))
				{
					std::ostringstream	oss;
					oss << "Invalid Data : " << data;
					throw std::invalid_argument(oss.str());
				}

				time = 0;
				for(uint32_t i = 0 ; i < 4 ; i++)
				{
					time = (time << 8) + data[8 + i];
				}

				uint16_t	adc_values[60];
				for(uint32_t i = 0 ; i < (length - 12) / 2 ; i++)
				{
					adc_values[i] = (data[12 + i*2] << 8) + (data[12 + i*2 + 1]);
				}

				
				Post(new MessageData(id_, id_, time, adc_values, (length - 12) / 2));
			}
			else
			{
				std::ostringstream	oss;
				oss << "Invalid Data Length : " << length ;
				throw std::invalid_argument(oss.str());
			}
		}
		catch(std::invalid_argument& e)
		{
			TRACE_WARN("Invalid format." << time_field << ", " << length_field);
			return	false;
		}
	}
	else
	{
		TRACE_WARN("Not support AT command - " << token);
		return	false;
	}

	return	true;
}

bool	Agent::OnServerRequest(std::string const& _message)
{
	JSONNode	request;
	try
	{
		request = libjson::parse(_message);	

	}
	catch(std::invalid_argument& e)
	{
		TRACE_INFO("Invalid request : " << _message);
		return	false;
	}


	std::string	type;

	if (!GetMemberValue(request, "type", type))
	{
		TRACE_WARN("The type can not be found in the message.");
		return	false;
	}

	return	true;
}

bool	Agent::SetNode(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	Agent*	agent = dynamic_cast<Agent*>(_object);
	if (!agent)
	{
		return	false;
	}

	TRACE_INFO2(agent, "Set Node : " << _value.write_formatted());
	for(auto item = _value.begin() ; item != _value.end() ; item++)
	{
		std::string	node_id;

		if (GetMemberValue(*item, "id", node_id))
		{
			Node* node = new Node(node_id, agent);
			if (node)
			{
				JSONNode	driver_properties;
				TRACE_INFO2(agent, "Create node : " << node_id);
				agent->node_list_.push_back(node);
				if (GetMemberValue(*item, "driver", driver_properties))
				{
					std::string serial;
					if (GetMemberValue(driver_properties, "serial", serial))
					{
						node->SetSerial(serial);
					}
				}
			}
		}
		else
		{
			TRACE_INFO2(agent, "ID not found!");
		}
	}

	return	true;
}

bool	Agent::SetServer(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);
	
	Agent*	agent = dynamic_cast<Agent *>(_object );
	if (!agent)
	{
		return	false;
	}
	
	for(auto item = _value.begin() ; item != _value.end() ; item++)
	{
		if (item->name() == "url")
		{
			agent->client_.SetServerURL(item->as_string());
		}
		else if (item->name() == "port")
		{
			agent->client_.SetServerPort(item->as_int());
		}
	}

	return	true;
}

bool	Agent::ReceiveData(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	static	uint32_t	data_time = 0;
	static	std::list<Agent::MFL>	mfl_list_;
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}

	
	Agent::MessageData *message = dynamic_cast<Agent::MessageData*>(_message);
	if (!message)
	{
		TRACE_ERROR2(agent, "Receive Data : " << message->GetLength());
		return	false;
	}

	if ((data_time / 1000) != (message->GetTime() / 1000))
	{
		JSONNode	node;

		JSONNode	mfl_array(JSON_ARRAY);
		mfl_array.set_name("rawMfl");

		for(auto mfl = mfl_list_.begin() ; mfl != mfl_list_.end() ; mfl++)
		{
			JSONNode	array(JSON_ARRAY);
			JSONNode	number;

			number = mfl->x;
			array.push_back(number);
			number = mfl->y;
			array.push_back(number);
			number = mfl->z;
			array.push_back(number);

			mfl_array.push_back(array);
		}
		mfl_list_.clear();

		TRACE_INFO2(agent, "MSG : " << mfl_array.write_formatted());
	}

	data_time = message->GetTime();	
	for(uint32_t i = 0 ; i < message->GetLength() ; i+=3)
	{
		mfl_list_.push_back(MFL(message->GetValue(i), message->GetValue(i+1), message->GetValue(i+2)));
	}

	TRACE_INFO2(agent, "Data : " << message->GetTime() );
	return	true;
}
