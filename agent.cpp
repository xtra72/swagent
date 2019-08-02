#include <sstream>
#include <string>
#include "assert.h"
#include "agent.h"
#include "utils.h"

Agent::MessageRequestContract::MessageRequestContract(std::string const& _receiver, std::string const& _sender, std::string const& _nid,uint32_t _channel_count)
: Message(MESSAGE_TYPE_REQUEST_CONTRACT, _receiver,_sender), nid_(_nid), channel_count_(_channel_count)
{
}

Agent::MessageRequestContract::~MessageRequestContract()
{
}

Agent::MessageData::MessageData(std::string const& _receiver, std::string const& _sender, std::string const& _node_id, uint32_t _time, uint16_t* _data, uint32_t _length)
: Message(MESSAGE_TYPE_DATA, _receiver, _sender), node_id_(_node_id), time_(_time), data_(NULL), length_(_length)
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

Agent::MessageMotionDetectionStarted::MessageMotionDetectionStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_MOTION_DETECTION_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageMotionDetectionStarted::~MessageMotionDetectionStarted()
{
}

Agent::MessageMotionDetectionAlreadyStarted::MessageMotionDetectionAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageMotionDetectionAlreadyStarted::~MessageMotionDetectionAlreadyStarted()
{
}

Agent::MessageMotionDetectionStopped::MessageMotionDetectionStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_MOTION_DETECTION_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageMotionDetectionStopped::~MessageMotionDetectionStopped()
{
}

Agent::MessageMotionDetectionAlreadyStopped::MessageMotionDetectionAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageMotionDetectionAlreadyStopped::~MessageMotionDetectionAlreadyStopped()
{
}

Agent::MessageMotionDetected::MessageMotionDetected(std::string const& _receiver, std::string const& _sender, std::string const& _node_id)
: Message(MESSAGE_TYPE_MOTION_DETECTED, _receiver, _sender), node_id_(_node_id)
{
}

Agent::MessageMotionDetected::~MessageMotionDetected()
{
}

Agent::MessageScanStarted::MessageScanStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_SCAN_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageScanStarted::~MessageScanStarted()
{
}

Agent::MessageScanAlreadyStarted::MessageScanAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_SCAN_ALREADY_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageScanAlreadyStarted::~MessageScanAlreadyStarted()
{
}

Agent::MessageScanStopped::MessageScanStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_SCAN_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageScanStopped::~MessageScanStopped()
{
}

Agent::MessageScanAlreadyStopped::MessageScanAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_SCAN_ALREADY_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageScanAlreadyStopped::~MessageScanAlreadyStopped()
{
}

Agent::MessageTransStarted::MessageTransStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_TRANS_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageTransStarted::~MessageTransStarted()
{
}

Agent::MessageTransAlreadyStarted::MessageTransAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_TRANS_ALREADY_STARTED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageTransAlreadyStarted::~MessageTransAlreadyStarted()
{
}

Agent::MessageTransStopped::MessageTransStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_TRANS_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageTransStopped::~MessageTransStopped()
{
}

Agent::MessageTransAlreadyStopped::MessageTransAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid)
: Message(MESSAGE_TYPE_TRANS_ALREADY_STOPPED, _receiver,_sender), nid_(_nid)
{
}

Agent::MessageTransAlreadyStopped::~MessageTransAlreadyStopped()
{
}

Agent::ContractResponseReceiver::ContractResponseReceiver(Agent* _parent, std::string const& _topic) 
: MQTTClient::Subscriber(_topic) 
{
	parent_ = _parent;
}

	
void	Agent::ContractResponseReceiver::OnMessage(std::string const& _message)
{
	if (parent_ != NULL)
	{
		Agent*	agent = dynamic_cast<Agent*>(parent_);

		agent->OnContractResponse(_message);
	}
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
: ActiveObject(_id), client_(_id + ".client", this), status_report_interval_(3600), contract_request_interval_(10), plus_log_output_(false), check_mid_(false), time_order_(true)
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	properties_map_["log"] = SetLog;
	properties_map_["status_report_interval"] = SetStatusReportInterval;
	properties_map_["order"] = SetDataOrder;
	message_handler_map_[MESSAGE_TYPE_CONNECTED]		= OnMessageConnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DISCONNECTED] 	= OnMessageDisconnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DATA]				= OnMessageDataCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTED] 	= OnMessageMotionDetectedCallback;
	message_handler_map_[MESSAGE_TYPE_REQUEST_CONTRACT]	= OnMessageRequestContractCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_ALREADY_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_ALREADY_STOPPED]	= OnMessageConfirmCallback;
}

Agent::Agent()
: ActiveObject("agent"), client_("client", this), status_report_interval_(3600), contract_request_interval_(10), plus_log_output_(false), check_mid_(false), time_order_(true)
{
	properties_map_["node"] = SetNode;
	properties_map_["server"] = SetServer;
	properties_map_["log"] = SetLog;
	properties_map_["status_report_interval"] = SetStatusReportInterval;
	properties_map_["order"] = SetDataOrder;
	message_handler_map_[MESSAGE_TYPE_CONNECTED]		= OnMessageConnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DISCONNECTED] 	= OnMessageDisconnectedCallback;
	message_handler_map_[MESSAGE_TYPE_DATA]				= OnMessageDataCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTED] 	= OnMessageMotionDetectedCallback;
	message_handler_map_[MESSAGE_TYPE_REQUEST_CONTRACT]	= OnMessageRequestContractCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_SCAN_ALREADY_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_ALREADY_STARTED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_STOPPED]	= OnMessageConfirmCallback;
	message_handler_map_[MESSAGE_TYPE_TRANS_ALREADY_STOPPED]	= OnMessageConfirmCallback;
}

void	Agent::Preprocess()
{
	for(auto node = node_list_.begin(); node != node_list_.end() ; node++)
	{
		(*node)->Start();
	}
	client_.Start();

	std::ostringstream	oss_ctl_req;
	oss_ctl_req << "cwr/mfl/" << id_ << "/ctl/req";
	ServerRequestReceiver*	ctl_request_receiver = new ServerRequestReceiver(this, oss_ctl_req.str());
	if (ctl_request_receiver)
	{
		client_.Register(ctl_request_receiver);
	}

	std::ostringstream	oss_contract_res;
	oss_contract_res << "cwr/mfl/" << id_ << "/contract/res";
	ContractResponseReceiver*	contract_response_receiver = new ContractResponseReceiver(this, oss_contract_res.str());
	if (contract_response_receiver)
	{
		client_.Register(contract_response_receiver);
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
		return	OnPlusLog(message_packet_received->GetSender(), token + strlen(token) + 1);
	}
	else if (strcasecmp(token, "+STAT") == 0)
	{
		return	OnStat(message_packet_received->GetSender(), token + strlen(token) + 1);
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
		uint32_t	length = strtoul(length_field, NULL, 10);

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

		for(auto node = node_list_.begin() ; node != node_list_.end() ; node++)
		{
			if ((*node)->GetID() == _node_id)
			{
				(*node)->OnData(byte_array, length);
			}
			break;
		}
	}
	catch(std::out_of_range& e)
	{
		TRACE_WARN("Out of Range ." << time_field << ", " << length_field);
		return	false;
	}
	catch(std::invalid_argument& e)
	{
		TRACE_WARN("Invalid format." << time_field << ", " << length_field);
		return	false;
	}

	return	true;
}

bool	Agent::OnPlusLog(std::string const &_node_id, char* _log)
{
	if (plus_log_output_)
	{
		TRACE_INFO("+LOG : " << _log);
	}

	return	true;
}

bool	Agent::OnStat(std::string const &_node_id, char* _stat)
{
	for(auto node = node_list_.begin() ; node != node_list_.end() ; node++)
	{
		if ((*node)->GetID() == _node_id)
		{
			return	(*node)->OnStat(_stat);
		}
	}

	return	false;
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

	std::string	mid_string;
	uint32_t	mid;
	if (!GetMemberValue(request, "mid", mid_string))
	{
		TRACE_WARN("The mid can not be found in the message. : " << _message);
		return	false;
	}

	mid = strtoul(mid_string.c_str(), NULL, 10);

	std::string	ctl;
	uint32_t	sleep;
	if (!GetMemberValue(request, "ctl", ctl))
	{
		TRACE_WARN("The value can not be found in the message.");
		return	false;
	}

	if (ctl == "s")
	{
		if (!GetMemberValue(request, "sleep", sleep))
		{
			TRACE_WARN("The sleep can not be found in the message.");
			return	false;
		}
	}

	for(auto it = node_list_.begin() ; it != node_list_.end() ; it++)
	{
		if((ctl == "scan_start") || (ctl == "a"))
		{
			(*it)->Scan(true, mid);
		}
		else if(ctl == "scan_stop")
		{
			(*it)->Scan(false, mid);
		}
		else if ((ctl == "sleep") || (ctl == "s"))
		{
			(*it)->Sleep(sleep, mid);
		}
		else if ((ctl == "detect_start") || (ctl == "d"))
		{
			(*it)->Detect(true, mid);
		}
		else if ((ctl == "ready") || (ctl == "r"))
		{
			(*it)->Ready(mid);
		}
		else if (ctl == "detect_stop")
		{
			(*it)->Detect(false, mid);
		}
		else if (ctl == "trans_start")
		{
			(*it)->Transfer(true, mid);
		}
		else if (ctl == "trans_stop")
		{
			(*it)->Transfer(false, mid);
		}
		else
		{
			TRACE_WARN("Not supported status : " << ctl);	
		}
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
		Node* node = new Node(*item, agent);
		agent->node_list_.push_back(node);
		TRACE_INFO2(agent, "New node[" << node->GetID() << "] added!");
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

bool	Agent::SetLog(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);
	
	Agent*	agent = dynamic_cast<Agent *>(_object );
	if (!agent)
	{
		return	false;
	}

	for(auto item = _value.begin() ; item != _value.end() ; item++)
	{
		if (item->name() == "plus_log_output_")
		{
			agent->plus_log_output_ = item->as_bool();
		}
	}

	return	true;
}

bool	Agent::SetStatusReportInterval(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);
	
	Agent*	agent = dynamic_cast<Agent *>(_object );
	if (!agent)
	{
		return	false;
	}

	agent->status_report_interval_ = _value.as_int();

	return	true;
}

bool	Agent::SetDataOrder(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);
	
	Agent*	agent = dynamic_cast<Agent *>(_object );
	if (!agent)
	{
		return	false;
	}
	
	if (_value.as_string() == "time")
	{
		agent->time_order_ = true;
	}
	else if (_value.as_string() == "channel")
	{
		agent->time_order_ = false;
	}
	else
	{
		return	false;
	}

	return	true;
}

bool	Agent::SetContractRequestInterval(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);
	
	Agent*	agent = dynamic_cast<Agent *>(_object );
	if (!agent)
	{
		return	false;
	}

	agent->contract_request_interval_ = _value.as_int();

	return	true;
}

bool	Agent::StartContractRequestToServer(void)
{
	if (!contract_timer_.IsRun())
	{
		contract_timer_.SetInterval(contract_request_interval_ * (uint64_t)000000, Agent::OnContractRequest, this, true);
	}

	return	true;
}

bool	Agent::StopContractRequestToServer(void)
{
	if (contract_timer_.IsRun())
	{
		contract_timer_.Stop();
	}

	return	true;
}

bool	Agent::ContractRequestToServer(void)
{
	Date		current_time = Date::GetCurrent();
	bool		all_contracted = true;

	for(auto it = node_list_.begin(); it != node_list_.end() ; it++)
	{
		if (!(*it)->IsContracted())
		{
			JSONNode	payload;

			std::ostringstream	oss_msg_id;
			oss_msg_id << current_time.GetMicroseconds();
			payload.push_back(JSONNode("mid", oss_msg_id.str()));
			contract_request_mid_ = oss_msg_id.str();

			payload.push_back(JSONNode("nid", (*it)->GetID()));
			payload.push_back(JSONNode("ch", (*it)->GetChannelCount()));

			std::ostringstream	oss_topic;

			oss_topic << "cwr/mfl/" << id_ << "/contract/req";

			MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);
			if (!client_.Publish(pub))
			{
				delete pub;
				TRACE_WARN("Publish failed!");
			}

			all_contracted = false;
		}
	}

	if (all_contracted)
	{
		TRACE_WARN("All node is contracted.");
		StopContractRequestToServer();
	}

	return	true;
}

bool	Agent::ContractRequestToServer(char* _node_id, uint8_t _channel_count)
{
	Date		current_time = Date::GetCurrent();

	for(auto it = node_list_.begin(); it != node_list_.end() ; it++)
	{
		if ((*it)->GetID() == _node_id)
		{
			JSONNode	payload;

			std::ostringstream	oss_msg_id;
			oss_msg_id << current_time.GetMicroseconds();
			payload.push_back(JSONNode("mid", oss_msg_id.str()));
			contract_request_mid_ = oss_msg_id.str();

			payload.push_back(JSONNode("nid", _node_id));
			payload.push_back(JSONNode("ch", _channel_count));

			std::ostringstream	oss_topic;

			oss_topic << "cwr/mfl/" << id_ << "/contract/req";

			TRACE_INFO("Contract request : " << _node_id << ", " << (uint32_t)_channel_count);
			MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);
			if (!client_.Publish(pub))
			{
				delete pub;
				TRACE_WARN("Publish failed!");
			}
		}
	}


	return	true;
}

bool	Agent::OnContractResponse(std::string const& _message)
{
	JSONNode	response = libjson::parse(_message);
	std::string	mid;
	std::string	nid;
	uint32_t	ts;

	TRACE_INFO("Contract response received!");
	if (!GetMemberValue(response, "mid", mid))
	{
		TRACE_WARN("MID is not exist.");
		return	false;
	}	

	if (check_mid_)
	{
		if (contract_request_mid_ != mid)
		{
			TRACE_WARN("Contract request message IDs do not match.");
			return	false;
		}
	}

	if (!GetMemberValue(response, "ts", ts))
	{
		TRACE_WARN("Time Stamp is not exist.");
		return	false;
	}

	if (!GetMemberValue(response, "nid", nid))
	{
		TRACE_WARN("Node id is not exist.");
		return	false;
	}

	for(auto it = node_list_.begin() ; it != node_list_.end() ; it++)
	{
		if ((*it)->GetID() == nid)
		{
			TRACE_WARN("Node[" << nid << "] contracted!");
			(*it)->Contract(ts, strtoul(mid.c_str(), NULL, 10));
			return	true;
		}
	}

	TRACE_WARN("Node not found!");

	return	false;
}

bool	Agent::ControlResponseToServer(void)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("mid", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("ts", oss_time_stamp.str()));

	payload.push_back(JSONNode("res", "fail"));
	payload.push_back(JSONNode("msg", "timeout"));

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/ctl/res";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);
	if (!client_.Publish(pub))
	{
		delete pub;
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
		delete pub;
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushResponseToServer(std::string const& _nodeId, bool _result, std::string const& _message)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;
	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/ctl/res";

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("mid", oss_msg_id.str()));
	payload.push_back(JSONNode("nid", _nodeId));
	if (_result)
	{
		payload.push_back(JSONNode("res", "s"));
	}
	else
	{
		payload.push_back(JSONNode("res", "f"));
	}
	payload.push_back(JSONNode("msg", _message));

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		delete pub;
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushLogNotificationToServer(std::string const& _nodeId, std::string const& _message)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;
	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/log/push";

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("mid", oss_msg_id.str()));
	payload.push_back(JSONNode("nid", _nodeId));
	payload.push_back(JSONNode("msg", _message));

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		delete pub;
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::PushMotionDetectedToServer(std::string const& _nodeId)
{
	Date		current_time = Date::GetCurrent();
	JSONNode	payload;

	std::ostringstream	oss_msg_id;
	oss_msg_id << current_time.GetMicroseconds();
	payload.push_back(JSONNode("mid", oss_msg_id.str()));

	std::ostringstream	oss_time_stamp;
	oss_time_stamp << current_time.GetSeconds();
	payload.push_back(JSONNode("ts", oss_time_stamp.str()));

		
	payload.push_back(JSONNode("nid", _nodeId));

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << id_ << "/move";

	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

	if (!client_.Publish(pub))
	{
		delete pub;
		TRACE_WARN("Publish failed!");
		return	false;
	}

	return	true;
}

bool	Agent::StartStatusReport(void)
{
	if (!status_report_timer_.IsRun())
	{
		status_report_timer_.SetInterval(status_report_interval_ * (uint64_t)1000000, Agent::OnStatusReport, this);
	}

	return	true;
}

bool	Agent::StopStatusReport(void)
{
	status_report_timer_.Stop();

	return	true;
}

bool	Agent::PushStatusToServer(void)
{
	Date		current_time = Date::GetCurrent();
	for(auto it = node_list_.begin() ; it != node_list_.end() ; it++)
	{
		JSONNode	payload;

		std::ostringstream	oss_msg_id;
		oss_msg_id << current_time.GetMicroseconds();
		payload.push_back(JSONNode("mid", oss_msg_id.str()));

		std::ostringstream	oss_time_stamp;
		oss_time_stamp << current_time.GetSeconds();
		payload.push_back(JSONNode("ts", oss_time_stamp.str()));

		payload.push_back(JSONNode("nid", (*it)->GetID()));
		payload.push_back(JSONNode("vol", 3.4));

		std::ostringstream	oss_topic;

		oss_topic << "cwr/mfl/" << id_ << "/health/push";

		MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);

		if (!client_.Publish(pub))
		{
			delete pub;
			TRACE_WARN("Publish failed!");
		}
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
		delete pub;
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

	uint32_t	message_time = message_data->GetTime();
	
	for(uint32_t loop = 0 ; loop < message_data->GetLength() ; loop += node->GetChannelCount())
	{

		if ((data_time / 1000) != message_time  / 1000)
		{
			Date		current_time = Date::GetCurrent();
			JSONNode	payload;

			std::ostringstream	oss_msg_id;
			oss_msg_id << current_time.GetMicroseconds();
			payload.push_back(JSONNode("mid", oss_msg_id.str()));

			std::ostringstream	oss_time_stamp;
			oss_time_stamp << current_time.GetSeconds();
			payload.push_back(JSONNode("ts", oss_time_stamp.str()));

			JSONNode	data_array(JSON_ARRAY);
			data_array.set_name("dat");

			JSONNode	node_data;
			node_data.push_back(JSONNode("nid", node->GetID()));
			JSONNode	channel_data(JSON_ARRAY);
			channel_data.set_name("ch");
			if (agent->time_order_)
			{
				for(uint32_t i = 0 ; i < node->GetDataCount(0) ; i++)
				{
					JSONNode	array(JSON_ARRAY);
					for(uint32_t channel = 0 ; channel < node->GetChannelCount() ; channel++)
					{
						JSONNode	number(JSON_NUMBER);

						number = node->GetData(channel, i);
						array.push_back(number);
					}

					channel_data.push_back(array);
				}

				for(uint32_t i = 0 ; i < node->GetDataCount(0) ; i++)
				{
					node->ClearData(i);
				}
			}
			else
			{
				for(uint32_t i = 0 ; i < node->GetChannelCount() ; i++)
				{
					JSONNode	array(JSON_ARRAY);
					for(uint32_t j = 0 ; j < node->GetDataCount(i) ; j++)
					{
						JSONNode	number(JSON_NUMBER);

						number = node->GetData(i, j);
						array.push_back(number);
					}

					channel_data.push_back(array);

					node->ClearData(i);
				}
			}
			node_data.push_back(channel_data);

			data_array.push_back(node_data);

			payload.push_back(data_array);

			agent->PushDataToServer(payload.write());
			data_time = message_time;
		}

		for(uint32_t j  = 0 ; (loop + j) < message_data->GetLength() && (j < node->GetChannelCount()) ; j++)
		{	
			node->AppendData(j, message_data->GetValue(loop+j));
		}

		message_time += 2;
	}

	//TRACE_INFO2(agent, "Data : " << message_data->GetTime() << ", " << message_data->GetLength());

	return	true;
}

bool	Agent::OnMessageMotionDetectedCallback(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	static	std::list<std::vector<uint16_t>>	mfl_list_;
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}

	
	MessageMotionDetected*	message = dynamic_cast<MessageMotionDetected *>(_message);

	return	agent->PushMotionDetectedToServer(message->GetNodeId());
}

bool	Agent::OnMessageRequestContractCallback(ActiveObject* _object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}

	TRACE_INFO2(agent, "On Message Request Contract");
	Agent::MessageRequestContract *message = dynamic_cast<Agent::MessageRequestContract*>(_message);
	if (!message)
	{
		TRACE_ERROR2(agent, "Invalid message" << *_message);
		return	false;
	}

	JSONNode	payload;

	std::ostringstream	oss_mid;

	oss_mid << message->GetID();

	payload.push_back(JSONNode("mid", oss_mid.str()));
	payload.push_back(JSONNode("nid", message->GetNID()));
	payload.push_back(JSONNode("ch",  message->GetChannelCount()));

	std::ostringstream	oss_topic;

	oss_topic << "cwr/mfl/" << agent->GetID() << "/contract/req";

	TRACE_INFO2(agent, "Contract request : " << payload.write());
	MQTTClient::Publisher*	pub = new MQTTClient::Publisher(oss_topic.str(), payload);
	if (!agent->client_.Publish(pub))
	{
		delete pub;
		TRACE_WARN2(agent, "Publish failed!");
		return	false;
	}


	return	true;
}

bool	Agent::OnMessageConfirmCallback(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	static	std::list<std::vector<uint16_t>>	mfl_list_;
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}


	std::string	log;
	std::string	nid;

	switch(_message->GetType())
	{
	case	MESSAGE_TYPE_MOTION_DETECTION_STARTED: 
		{
			MessageMotionDetectionStarted*	message = dynamic_cast<MessageMotionDetectionStarted *>(_message);
			nid = message->GetNID();
			log = "Motion detection started";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STARTED:
		{
			MessageMotionDetectionAlreadyStarted*	message = dynamic_cast<MessageMotionDetectionAlreadyStarted *>(_message);
			nid = message->GetNID();
			log = "Motion detection already started";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_MOTION_DETECTION_STOPPED:
		{
			MessageMotionDetectionStopped*	message = dynamic_cast<MessageMotionDetectionStopped*>(_message);
			nid = message->GetNID();
			log = "Motion detection stoppped";	
		}
		break;
	case	MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STOPPED:
		{
			MessageMotionDetectionAlreadyStopped*	message = dynamic_cast<MessageMotionDetectionAlreadyStopped*>(_message);
			nid = message->GetNID();
			log = "Motion detection already stoppped";	
		}
		break;
	case	MESSAGE_TYPE_SCAN_STARTED:
		{
			MessageScanStarted*	message = dynamic_cast<MessageScanStarted*>(_message);
			nid = message->GetNID();
			log = "Scan started";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_SCAN_ALREADY_STARTED:
		{
			MessageScanAlreadyStarted*	message = dynamic_cast<MessageScanAlreadyStarted*>(_message);
			nid = message->GetNID();
			log = "Scan already started";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_SCAN_STOPPED:
		{
			MessageScanStopped*	message = dynamic_cast<MessageScanStopped*>(_message);
			nid = message->GetNID();
			log = "Scan stopped";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_SCAN_ALREADY_STOPPED:
		{
			MessageScanAlreadyStopped*	message = dynamic_cast<MessageScanAlreadyStopped*>(_message);
			nid = message->GetNID();
			log = "Scan operation is not in progress.";
		}
		break;
	case	MESSAGE_TYPE_TRANS_STARTED:
		{
			MessageTransStarted*	message = dynamic_cast<MessageTransStarted*>(_message);
			nid = message->GetNID();
			log = "Data transfer started";	

			agent->PushResponseToServer(nid, true, log);
		}
		break;
	case	MESSAGE_TYPE_TRANS_ALREADY_STARTED:
		{
			MessageTransAlreadyStarted*	message = dynamic_cast<MessageTransAlreadyStarted*>(_message);
			nid = message->GetNID();
			log = "Data transfer already started";	
		}
		break;
	case	MESSAGE_TYPE_TRANS_STOPPED:
		{
			MessageTransStopped*	message = dynamic_cast<MessageTransStopped*>(_message);
			nid = message->GetNID();
			log = "Data transfer stopped";	
		}
		break;
	case	MESSAGE_TYPE_TRANS_ALREADY_STOPPED:
		{
			MessageTransAlreadyStopped*	message = dynamic_cast<MessageTransAlreadyStopped*>(_message);
			nid = message->GetNID();
			log = "Data transfer already stopped";	
		}
		break;

	default:
		TRACE_WARN2(NULL, "Unknown message type : " << _message->GetType());
	}


	return	agent->PushLogNotificationToServer(nid, log);
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

	//agent->StartContractRequestToServer();
	//agent->StartStatusReport();

	return	true;
}

bool	Agent::OnMessageDisconnectedCallback(ActiveObject *_object, Message* _message)
{
	Agent*	agent = dynamic_cast<Agent*>(_object);
	if (!agent)
	{
		TRACE_ERROR2(NULL, "Object is not Agent");
		return	false;
	}

	//agent->StopContractRequestToServer();
	//agent->StopStatusReport();

	return	true;
}

void	Agent::OnContractRequest(void* _params)
{
	Agent*	agent = (Agent*)_params;
	if (agent != NULL)
	{
		agent->ContractRequestToServer();
	}
}


void	Agent::OnStatusReport(void* _params)
{
	Agent*	agent = (Agent*)_params;
	if (agent != NULL)
	{
		agent->PushStatusToServer();
	}
}

