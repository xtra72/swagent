#include <sstream>
#include <string>
#include "assert.h"
#include "agent.h"
#include "utils.h"

Agent::MessageData::MessageData(std::string const& _receiver, std::string const& _sender, std::string const& _node_id, uint32_t _time, uint16_t* _data, uint32_t _length)
: Message(110, _receiver, _sender), node_id_(_node_id), time_(_time), data_(NULL), length_(_length)
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
: ActiveObject(_id), client_(_id + ".client", this)
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	message_handler_map_[MESSAGE_TYPE_CONNECTED] 	= OnMessageConnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DISCONNECTED] 	= OnMessageDisconnectedCallback;
	message_handler_map_[110] 	= OnMessageDataCallback;
}

Agent::Agent()
: ActiveObject("agent"), client_("client", this)
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	message_handler_map_[MESSAGE_TYPE_CONNECTED] 	= OnMessageConnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DISCONNECTED] 	= OnMessageDisconnectedCallback;
	message_handler_map_[110] 	= OnMessageDataCallback;
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

Node*	Agent::GetNode(std::string const& _id)
{
	for(auto node = node_list_.begin(); node != node_list_.end() ; node++)
	{
		if (_id == (*node)->GetID())
		{
			return	*node;	
		}
	}

	return	NULL;
}

bool	Agent::OnReceived(Message* _message)
{
	char rxBuffer[1024];
	MessagePacketReceived * message_packet_received = dynamic_cast<MessagePacketReceived*>(_message);
	if (!message_packet_received)
	{
		return	false;
	}

	memcpy(rxBuffer, message_packet_received->GetData(), message_packet_received->GetSize());
	rxBuffer[message_packet_received->GetSize()] = 0;

	char*	token = strtok(rxBuffer, ":");
	if (!token)
	{
		TRACE_WARN("Invalid format.");
		return	false;
	}

	if (strcasecmp(token, "+DATA") == 0)
	{
		return	OnPlusData(message_packet_received->GetSender(), token + strlen(token) + 1);
	}
	else if (strcasecmp(token, "+LOG") == 0)
	{
		TRACE_INFO("+LOG : " << token+strlen(token)+1);
	}
	else if (strcasecmp(token, "+START") == 0)
	{
	}
	else if (strcasecmp(token, "+STOP") == 0)
	{
	}
	else
	{
		TRACE_WARN("Not support AT command - " << token);
		return	false;
	}

	return	true;
}

bool	Agent::OnPlusData(std::string const &_node_id, char* _data)
{
	char*	time_field = strtok(_data, ",");
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

		if ((length > 12) && ((length - 12) % 16 == 0))
		{
			char* token = strtok(NULL, " ");
			if (strlen(token) != length*2)
			{
				std::ostringstream	oss;
				oss << "Invalid format : " << strlen(token) << ", " << length;
				TRACE_WARN(oss.str());
				return	false;
			}

			uint8_t	byte_array[128];

			if (!StringToUint8(token, byte_array, sizeof(byte_array), length))
			{
				std::ostringstream	oss;
				oss << "Invalid Data : " << byte_array;
				throw std::invalid_argument(oss.str());
			}

			time = 0;
			for(uint32_t i = 0 ; i < 4 ; i++)
			{
				time = (time << 8) + byte_array[8 + i];
			}

			uint16_t	adc_values[60];
			for(uint32_t i = 0 ; i < (length - 12) / 2 ; i++)
			{
				adc_values[i] = (byte_array[12 + i*2] << 8) + (byte_array[12 + i*2 + 1]);
			}

			Post(new MessageData(id_, id_, _node_id, time, adc_values, (length - 12) / 2));
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

	return	true;
}

bool	Agent::OnServerRequest(std::string const& _message)
{
	JSONNode	request;

	TRACE_INFO("On Server Request : " << _message);
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

	if (type == "status")
	{
		std::string	value;
		JSONNode	nodeIds;

		if (!GetMemberValue(request, "value", value))
		{
			TRACE_WARN("The value can not be found in the message.");
			return	false;
		}

		if (!GetMemberValue(request, "nodeId", nodeIds))
		{
			TRACE_WARN("The node id can not be found in the message.");
			return	false;
		}

		for(auto it = nodeIds.begin() ; it != nodeIds.end() ; it++)
		{
			Node*	node = GetNode(it->as_string());
			if (node != NULL)
			{
				if(value == "active")
				{
					node->Active();
				}
				else if (value == "sleep")
				{
					node->Sleep();
				}
				else
				{
					TRACE_WARN("Not supported status : " << value);	
				}
			}
			else
			{
				TRACE_WARN("Node[" << it->as_string() << "] is not exist!");	
			}
		}
	}
	else if (type == "led")
	{
	}
	else
	{
		TRACE_WARN("Not supported command type : " << type);
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
						node->SetSerialID(serial);
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

bool	Agent::ContractRequestToServer(void)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("msgId", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("timeStamp", oss_time_stamp.str()));

	JSONNode	sensor_node_list(JSON_ARRAY);
	sensor_node_list.set_name("sensorNode");

	for(auto it = node_list_.begin(); it != node_list_.end() ; it++)
	{
		JSONNode	sensor_node;

		sensor_node.push_back(JSONNode("nodeId", (*it)->GetID()));
		sensor_node.push_back(JSONNode("chCount", (*it)->GetChannelCount()));

		sensor_node_list.push_back(sensor_node);
	}
	payload.push_back(sensor_node_list);

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/contract/req";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::ControlResponseToServer(void)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("msgId", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("timeStamp", oss_time_stamp.str()));

	payload.push_back(JSONNode("result", "fail"));
	payload.push_back(JSONNode("message", "timeout"));

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/command/res";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushDataToServer(std::string const& _payload)
{
	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/data/push";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), _payload);

	if (!client_.Publish(pub))
	{
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushStatusToServer(void)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("msgId", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("timeStamp", oss_time_stamp.str()));

	JSONNode	health(JSON_ARRAY);
	health.set_name("health");

	for(auto it = node_list_.begin() ; it != node_list_.end() ; it++)
	{
		JSONNode	node;
		
		node.push_back(JSONNode("nodeId", (*it)->GetID()));
		node.push_back(JSONNode("availability", "up"));
		node.push_back(JSONNode("battery", 3.4));

		health.push_back(node);
	}

	payload.push_back(health);

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/health/push";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushEncoderDataToServer(void)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("msgId", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("timeStamp", oss_time_stamp.str()));

	JSONNode	enData(JSON_ARRAY);
	enData.set_name("enData");

	JSONNode	value(JSON_NUMBER);
	value = 0;
	enData.push_back(value);
	value = 1;
	enData.push_back(value);
	value = 2;
	enData.push_back(value);

	payload.push_back(enData);

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/encoder/push";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::OnMessageDataCallback(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	static	uint32_t	data_time = 0;
	static	std::list<std::vector<uint16_t>>	mfl_list_;
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}
	
	Agent::MessageData *message_data = dynamic_cast<Agent::MessageData*>(_message);
	if (!message_data)
	{
		TRACE_ERROR2(agent, "Receive Data : " << message_data->GetLength());
		return	false;
	}

	Node*	node = agent->GetNode(message_data->GetNodeId());
	if (!node)
	{
		TRACE_ERROR2(agent, "Node not found : " << message_data->GetNodeId());
		return	false;
	}

	if ((data_time / 1000) != (message_data->GetTime() / 1000))
	{
		Date		current_time = Date::GetCurrent();
		JSONNode	payload;
	
		std::ostringstream	oss_msg_id;
		oss_msg_id << current_time.GetMicroseconds();
		payload.push_back(JSONNode("msgId", oss_msg_id.str()));

		std::ostringstream	oss_time_stamp;
		oss_time_stamp << current_time.GetSeconds();
		payload.push_back(JSONNode("timeStamp", oss_time_stamp.str()));
	
		JSONNode	raw_mfl(JSON_ARRAY);
		raw_mfl.set_name("rawMfl");

		for(auto mfl = mfl_list_.begin() ; mfl != mfl_list_.end() ; mfl++)
		{
			JSONNode	array(JSON_ARRAY);
			JSONNode	number;

			for(auto item = mfl->begin() ; item != mfl->end() ; item++)
			{
				number = *item;
				array.push_back(number);
			}

			raw_mfl.push_back(array);
		}
		mfl_list_.clear();

		JSONNode	channel_data_item;
		channel_data_item.push_back(JSONNode("nodeId", message_data->GetNodeId()));
		channel_data_item.push_back(raw_mfl);

		JSONNode	channel_data(JSON_ARRAY);
		channel_data.set_name("chData");
		channel_data.push_back(channel_data_item);

		payload.push_back(channel_data);

		agent->PushDataToServer(payload.write());
	}

	data_time = message_data->GetTime();	
	for(uint32_t i = 0 ; i < message_data->GetLength() ; i+=node->GetChannelCount())
	{
		std::vector<uint16_t>	mfl;

		for(uint32_t j  = 0 ; (i+j) < message_data->GetLength() && (j < node->GetChannelCount()) ; j++)
		{	
			mfl.push_back(message_data->GetValue(i+j));
		}
		mfl_list_.push_back(mfl);
	}

	TRACE_INFO2(agent, "Data : " << message_data->GetTime() );

	return	true;
}

bool	Agent::OnMessageConnectedCallback(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}

	TRACE_INFO2(agent, "On Message Connected");
	MQTTClient::MessageConnected *message_connected = dynamic_cast<MQTTClient::MessageConnected*>(_message);
	if (!message_connected)
	{
		TRACE_ERROR2(agent, "Invalid message" << *_message);
		return	false;
	}

	agent->ContractRequestToServer();

	return	true;
}

bool	Agent::OnMessageDisconnectedCallback(ActiveObject *_object, Message* _message)
{
	return	true;
}
