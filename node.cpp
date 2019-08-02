#include "node.h"
#include "message.h"
#include "utils.h"
#include "agent.h"

Node::Node()
: CP2110()
{
}

Node::Node(JSONNode const& _config, Object* _parent)
: CP2110("", _parent), frequency_(915000000), power_(18), contracted_(false), last_update_time_(0), rssi_(-200)
{
	properties_map_["channel_count"] = SetChannelCount;
	properties_map_["frequency"] = SetFrequency;
	properties_map_["power"] = SetPower;

	Set(_config);
}

Node::Node(std::string const& _id, Object* _parent)
: CP2110(_id, _parent), frequency_(915000000), power_(18), contracted_(false), last_update_time_(0), rssi_(-200)
{
	properties_map_["channel_count"] = SetChannelCount;
	properties_map_["frequency"] = SetFrequency;
	properties_map_["power"] = SetPower;
}

bool	Node::IsContracted(void)
{
	return	contracted_;
}

bool	Node::SetContract(bool _contract)
{
	return	contracted_ = _contract;
}

uint32_t	Node::GetFrequency(void)
{
		return	frequency_;
}

bool	Node::SetFrequency(uint32_t _frequency)
{
	frequency_ = _frequency;
	TRACE_INFO("Set Frequency : " << _frequency);

	std::ostringstream	cmd;

	cmd << "AT+CFG:" << _frequency;
	
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

uint32_t	Node::GetPower(void)
{
		return	power_;
}

bool	Node::SetPower(uint32_t _power)
{
	power_ = _power;
	TRACE_INFO("Set Power  : " << _power);

	return	true;
}

uint32_t	Node::GetChannelCount(void)
{
		return	channel_data_.size();
}

bool	Node::SetChannelCount(uint32_t _count)
{
	channel_data_.resize(_count);
	TRACE_INFO("Set Channel Count : " << _count);
	TRACE_INFO("channel_dta_.size() : " << channel_data_.size());

	return	true;
}

uint32_t	Node::GetDataCount(uint32_t _channel)
{
	if (channel_data_.size() <= _channel)
	{
		return	0;
	}

	return	channel_data_[_channel].size();
}
	

uint16_t	Node::GetData(uint32_t _channel, uint32_t _index)
{
	if (channel_data_.size() <= _channel)
	{
		std::ostringstream	oss;

		oss << "Channel count is " << channel_data_.size();
		throw std::out_of_range(oss.str());
	}

	if (channel_data_[_channel].size() <= _index)
	{
		std::ostringstream	oss;

		oss << "Channel[" << _channel << "] data is " << channel_data_[_channel].size();
		throw std::out_of_range(oss.str());
	}

	return	channel_data_[_channel][_index];
}

bool	Node::AppendData(uint32_t _channel, uint16_t _value)
{
	if (channel_data_.size() <= _channel)
	{
		std::ostringstream	oss;

		oss << "Channel count is " << channel_data_.size();
		throw std::out_of_range(oss.str());
	}

	channel_data_[_channel].push_back(_value);

	return	true;
}

bool	Node::ClearData(uint32_t _channel)
{
	if (_channel < channel_data_.size())
	{
		channel_data_[_channel].clear();
	}

	return	true;
}

bool	Node::Command(bool scan, bool trans)
{
	if (!contracted_)
	{
		TRACE_INFO("Node is not contracted!");
		return	false;
	}

	std::ostringstream	cmd;

	cmd << "AT+CMD:" << id_;
	if (scan)
	{
		cmd << ",SCAN";
	}

	if (trans)
	{
		cmd << ",TRANS";
	}

	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

bool	Node::RequestStat(void)
{
	std::ostringstream	cmd;

	cmd << "AT+STAT:" << id_;
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}


bool	Node::Scan(bool start, uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	if (start)
	{
		data[length++] = MSG_TYPE_SCAN_START;
	}
	else
	{
		data[length++] = MSG_TYPE_SCAN_STOP;
	}
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 4;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Sleep(uint32_t _sleep, uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	data[length++] = MSG_TYPE_SLEEP;
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 8;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;
	data[length++] = (_sleep >> 24) & 0xFF;
	data[length++] = (_sleep >> 16) & 0xFF;
	data[length++] = (_sleep >>  8) & 0xFF;
	data[length++] = (_sleep      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Detect(bool start, uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	if (start)
	{
		data[length++] = MSG_TYPE_MOTION_DETECTION_START;
	}
	else
	{
		data[length++] = MSG_TYPE_MOTION_DETECTION_STOP;
	}
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 4;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Transfer(bool start, uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	if (start)
	{
		data[length++] = MSG_TYPE_TRANS_START;
	}
	else
	{
		data[length++] = MSG_TYPE_TRANS_STOP;
	}
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 4;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Ready(uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	data[length++] = MSG_TYPE_READY;
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 4;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Contract(uint32_t _time, uint32_t _mid)
{
	uint8_t	data[128];
	uint32_t	length = 0;

	data[length++] = 0;
	data[length++] = 0;
	data[length++] = (short_address_ >> 8) & 0xFF;
	data[length++] = (short_address_     ) & 0xFF;
	data[length++] = MSG_TYPE_CONTRACT_CONFIRM;
	data[length++] = 0;
	data[length++] = 0;
	data[length++] = 8;
	data[length++] = (_mid >> 24) & 0xFF;
	data[length++] = (_mid >> 16) & 0xFF;
	data[length++] = (_mid >>  8) & 0xFF;
	data[length++] = (_mid      ) & 0xFF;
	data[length++] = (_time >> 24) & 0xFF;
	data[length++] = (_time >> 16) & 0xFF;
	data[length++] = (_time >> 8) & 0xFF;
	data[length++] = (_time      ) & 0xFF;

	return	Downlink(data, length);
}

bool	Node::Downlink(uint8_t* data, uint32_t length)
{
	std::ostringstream	cmd;

	TRACE_INFO("Downlink");
	cmd << "AT+DL:" << id_ << ",";
	for(uint32_t i = 0 ; i < length ; i++	)
	{
		uint8_t	hi = (data[i] >> 4) & 0x0F;
		uint8_t	lo = (data[i]     ) & 0x0F;

		if (hi < 9)
		{
			cmd << (char)(hi + '0');
		}
		else if (hi < 16)
		{
			cmd << (char)(hi - 10 + 'A');
		}
		else
		{
			return	false;
		}

		if (lo < 9)
		{
			cmd << (char)(lo + '0');
		}
		else if (lo < 16)
		{
			cmd << (char)(lo - 10 + 'A');
		}
		else
		{
			return	false;
		}
	}

	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

bool	Node::OnRead(uint8_t* _data, uint32_t _length)
{
	if (parent_ != NULL)
	{
		ActiveObject*	active_object = dynamic_cast<ActiveObject*>(parent_);
		if (active_object)
		{
			//TRACE_INFO("OnRead : Post Message - Dest = " << parent_->GetID() << ", Length = " << _length);
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

bool	Node::SetFrequency(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetFrequency(_value.as_int());

	return	true;
}

bool	Node::SetPower(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetPower(_value.as_int());

	return	true;
}

bool	Node::OnWrite(uint8_t* _data, uint32_t _length)
{

	uint32_t	writeLength = 0, writeLength2 = 0;
	uint8_t		newline[1] = { '\n'};
	
	TRACE_INFO("OnWrite(" << (char *)_data << ")");
	HidUart_Write(uart_, _data, _length, &writeLength);
	HidUart_Write(uart_, newline, 1, &writeLength2);

	return	(_length == writeLength);
}

bool	Node::OnData(uint8_t* data, uint32_t length)
{
	try
	{
		switch(data[4])
		{
		case	MSG_TYPE_DATA:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					if ((length - 12) % 16 != 0)
					{
						std::ostringstream	oss;
						oss << "Invalid Data : " << data;
						throw std::invalid_argument(oss.str());
					}

					uint32_t	time = 0;
					for(uint32_t i = 0 ; i < 4 ; i++)
					{
						time = (time << 8) + data[8 + i];
					}

					uint16_t	adc_values[60];
					for(uint32_t i = 0 ; i < (length - 12) / 2 ; i++)
					{
						adc_values[i] = (data[12 + i*2] << 8) + (data[12 + i*2 + 1]);
					}

					Message* message = new Agent::MessageData(agent->GetID(), agent->GetID(), GetID(), time, adc_values, (length - 12) / 2);
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_CONTRACT_REQUEST:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageRequestContract(agent->GetID(), agent->GetID(), GetID(), GetChannelCount());
				
					contract_request_mid_ = message->GetID();

					TRACE_INFO("Request Contract : " << contract_request_mid_ << ", " << GetID() << ", " << GetChannelCount());
					if (!agent->Post(message))
					{
						delete message;
					}
				}

			}	
			break;

		case	MSG_TYPE_MOTION_DETECTION_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageMotionDetectionStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("MOtion Detection Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_MOTION_DETECTION_ALREADY_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageMotionDetectionAlreadyStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("MOtion Detection already Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_MOTION_DETECTION_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageMotionDetectionStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("MOtion Detection Stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_MOTION_DETECTION_ALREADY_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageMotionDetectionAlreadyStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("MOtion Detection already stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_MOTION_DETECTED:
			{
				TRACE_INFO("Motion Detected received!");

				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Agent::MessageMotionDetected*	message = new Agent::MessageMotionDetected(agent->GetID(), agent->GetID(), GetID());

					TRACE_INFO("Notify motion detected : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
				else
				{
					TRACE_ERROR("Agent lost");
				}
			}
			break;
		
		case	MSG_TYPE_SCAN_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageScanStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Scan Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_SCAN_ALREADY_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageScanAlreadyStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Scan already Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_SCAN_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageScanStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Scan Stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_SCAN_ALREADY_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageScanAlreadyStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Scan already stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_TRANS_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageTransStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Trans Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_TRANS_ALREADY_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageTransAlreadyStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Trans already Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_TRANS_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageTransStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Trans Stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_TRANS_ALREADY_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageTransAlreadyStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_INFO("Trans already stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_PING:
			{
			}
			break;

		default:
			{
				TRACE_WARN("Invalid port. : " << (uint32_t)data[4]);
				TRACE_INFO_DUMP(data, length);
			}
		}

		Date	date;
		last_update_time_ = date.GetSeconds();
	}
	catch(std::invalid_argument& e)
	{
		TRACE_WARN("Invalid format.");
		return	false;
	}

	return	true;
}

bool	Node::OnStat(char* _stat)
{
	char*	token = strtok(_stat,"=");
	if (strcasecmp(token, "RSSI") == 0)
	{
		char*	value = strtok(NULL, ",");
		if (value != NULL)
		{
			rssi_ = strtol(value, 0, 10);
			TRACE_INFO("RSSI : " << rssi_);
			return	true;
		}
	}

	return	false;
}
