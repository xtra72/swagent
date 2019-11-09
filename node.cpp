#include <unistd.h>
#include "node.h"
#include "message.h"
#include "utils.h"
#include "agent.h"

Node::Node()
: CP2110()
{
}

Node::Node(JSONNode const& _config, Object* _parent)
: CP2110("", _parent), frequency_(915000000), power_(14), touch_(false), encoder_(false), reverse_(false), contracted_(false), contract_time_(0), contract_time_diff_(0), last_update_time_(0), rssi_(-200), log_(false), last_data_time_(0), live_timeout_(10), encoder_count_(0), channel_data_(8), reset_control(NULL), live_check_(false)
{
	properties_map_["channel_count"] = SetChannelCount;
	properties_map_["frequency"] = SetFrequency;
	properties_map_["power"] = SetPower;
	properties_map_["encoder"] = SetEncoder;
	properties_map_["live_check"] = SetLiveCheck;
	properties_map_["reset_control"] = SetResetControl;

	Set(_config);
}

Node::Node(std::string const& _id, Object* _parent)
: CP2110(_id, _parent), frequency_(915000000), power_(14), touch_(false), encoder_(false), reverse_(false), contracted_(false), contract_time_(0), contract_time_diff_(0), last_update_time_(0), rssi_(-200), log_(false), last_data_time_(0), live_timeout_(10), encoder_count_(0), channel_data_(8), reset_control(NULL), live_check_(false)
{
	properties_map_["channel_count"] = SetChannelCount;
	properties_map_["frequency"] = SetFrequency;
	properties_map_["power"] = SetPower;
	properties_map_["encoder"] = SetEncoder;
	properties_map_["live_check"] = SetLiveCheck;
	properties_map_["reset_control"] = SetResetControl;
}


bool	Node::Touch(void)
{
	touch_ = true;

	return	true;
}

bool	Node::OnEncoder(int32_t _count)
{
	encoder_count_ = _count;
	touch_ = true;

	return	true;
}

void	Node::Preprocess()
{
	CP2110::Preprocess();


	if (!reset_control)
	{
		if (port_ == 0)
		{
			SetResetControl(22);
		}
		else if (port_ == 1)
		{
			SetResetControl(61);
		}
	}

	trace.SetDebug(true);
	//Reset();
	TRACE_DEBUG("Preprocess");
	usleep(1000);

	RFStart();
}

void	Node::Process()
{
	CP2110::Process();
	if (live_check_)
	{
		if (touch_)
		{
			//TRACE_DEBUG("Touched");
			touch_ = false;
			live_timeout_ = 10;
		}
		else
		{
			if (--live_timeout_  == 0)
			{
				TRACE_DEBUG("There is no reaction on the node.");
				ColdReset();
				usleep(1000000);
				RFStart();
				usleep(1000000);
				SetEncoder(encoder_count_, 0);
				
			}
			
		}
	}
}

bool	Node::RFStart(void)
{
	TRACE_DEBUG("Start");

	std::ostringstream	cmd;

	cmd << "AT+START:FREQ=" << frequency_;
	cmd << ",POW=" << power_;
	if (encoder_)
	{
		cmd << ",ENC=ON";
	}
	else
	{
		cmd << ",ENC=OFF";
	}

	
	return	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());
}

bool	Node::Reset(void)
{
	TRACE_DEBUG("Reset");

	std::ostringstream	cmd;

	cmd << "AT+RESET:0";
	
	return	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());
}

bool	Node::ColdReset(void)
{
	TRACE_DEBUG("ColdReset");

	if (!reset_control)
	{
		TRACE_WARN("Undefined reset control");

		return	false;
	}

	reset_control->Value(false);
	usleep(100000);
	reset_control->Value(true);
	
	return	true;
}

bool	Node::GetLog(void)
{
	return	log_;
}

bool	Node::SetLog(bool _enable)
{
	log_ = _enable;
	TRACE_DEBUG("Set Log : " << ((log_)?"ON":"OFF"));

	std::ostringstream	cmd;

	cmd << "AT+CFG:LOG=" << ((log_)?"ON":"OFF");
	
	return	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());
}

bool	Node::IsContracted(void)
{
	return	contracted_;
}

bool	Node::SetContract(bool _contract)
{
	return	contracted_ = _contract;
}

bool		Node::SetConfig(void)
{
	TRACE_DEBUG("Set Config : " << frequency_ << ", " << power_);

	std::ostringstream	cmd;

	cmd << "AT+CFG:FREQ=" << frequency_;
	cmd << ",POW=" << power_;
	if (encoder_)
	{
		cmd << ",ENC=ON";
	}
	else
	{
		cmd << ",ENC=OFF";
	}
	
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

uint32_t	Node::GetFrequency(void)
{
		return	frequency_;
}

bool	Node::SetFrequency(uint32_t _frequency)
{
	frequency_ = _frequency;
	TRACE_DEBUG("Set Frequency : " << _frequency);

	std::ostringstream	cmd;

	cmd << "AT+CFG:FREQ=" << frequency_;
	
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
	TRACE_DEBUG("Set Power  : " << _power);

	std::ostringstream	cmd;

	cmd << "AT+CFG:POW=" << power_;
	
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

bool	Node::GetEncoder(void)
{
	return	encoder_;
}

bool	Node::SetEncoder(bool _enable)
{
	encoder_ = _enable;
	TRACE_DEBUG("Set Encoder : " << ((encoder_)?"ON":"OFF"));

	std::ostringstream	cmd;

	cmd << "AT+CFG:ENC=" << ((encoder_)?"ON":"OFF");
	
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

bool	Node::SetEncoder(uint32_t _count, uint32_t _mid)
{
	TRACE_DEBUG("Set Encoder : " << _count);

	std::ostringstream	cmd;

	cmd << "AT+ENC:SET=" << _count;
	
	OnWrite((uint8_t *)cmd.str().c_str(), cmd.str().length());

	return	true;
}

bool	Node::SetResetControl(uint32_t _index)
{
	TRACE_DEBUG("Set Reset GPIO : " << _index);

	std::ostringstream	cmd;

	if (reset_control != NULL)
	{
		delete reset_control;
		reset_control = NULL;
	}

	reset_control = new GPIOOut(_index);

	return	true;
}

bool	Node::SetLiveCheck(bool _live_check)
{
	TRACE_DEBUG("Set Live Check : " << _live_check);

	live_check_ = _live_check;

	return	true;
}

bool	Node::GetLiveCheck()
{
	return	live_check_;
}

uint32_t	Node::GetChannelCount(void)
{
		return	channel_data_.size();
}

bool	Node::SetChannelCount(uint32_t _count)
{
	channel_data_.resize(_count);
	TRACE_DEBUG("Set Channel Count : " << _count);
	TRACE_DEBUG("channel_dta_.size() : " << channel_data_.size());

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

bool	Node::ClearDataAll(void)
{
	for(uint32_t i = 0 ; i < channel_data_.size() ; i++)
	{
		channel_data_[i].clear();
	}

	return	true;
}

bool	Node::Command(bool scan, bool trans)
{
	if (!contracted_)
	{
		TRACE_DEBUG("Node is not contracted!");
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

		Date	date;
		scan_start_time_ = date.GetSeconds();
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

	contract_time_ = _time;
	Date	date;
	contract_time_diff_ = (int32_t)contract_time_ - date.GetSeconds();

	return	Downlink(data, length);
}

bool	Node::Downlink(uint8_t* data, uint32_t length)
{
	std::ostringstream	cmd;

	TRACE_DEBUG("Downlink");
	cmd << "AT+DL:" << id_ << ",";
	for(uint32_t i = 0 ; i < length ; i++	)
	{
		uint8_t	hi = (data[i] >> 4) & 0x0F;
		uint8_t	lo = (data[i]     ) & 0x0F;

		if (hi < 10)
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

		if (lo < 10)
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
			//TRACE_DEBUG("OnRead : Post Message - Dest = " << parent_->GetID() << ", Length = " << _length);
			//TRACE_DEBUG("OnRead : " << (char *)_data);
			//TRACE_DEBUG_DUMP(_data,_length);
			active_object->Post(new MessagePacketReceived(parent_->GetID(), id_, _data, _length));
		}
		else
		{
			TRACE_DEBUG("Parent is not active object!");
		}
	}
	else
	{
		TRACE_DEBUG("Parent is not exist!");
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

bool	Node::SetEncoder(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetEncoder(_value.as_bool());

	return	true;
}

bool	Node::SetResetControl(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetResetControl(_value.as_int());

	return	true;
}

bool	Node::SetLiveCheck(Object* _object, JSONNode const& _value)
{
	Node*	node = dynamic_cast<Node*>(_object);
	if (!node)
	{
		return	false;	
	}

	node->SetLiveCheck(_value.as_bool());

	return	true;
}

uint32_t	Node::GetLastDataTime(void)
{
	return	last_data_time_;
}

bool		Node::SetLastDataTime(uint32_t _data_time)
{
	last_data_time_ = _data_time;

	return	true;
}

bool	Node::OnWrite(uint8_t* _data, uint32_t _length)
{

	uint32_t	writeLength = 0, writeLength2 = 0;
	uint8_t		newline[1] = { '\n'};
	
	TRACE_DEBUG("OnWrite(" << (char *)_data << ")");
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
					if ((length - 12) % (channel_data_.size() * 2) != 0)
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

					Message* message = new Agent::MessageData(agent->GetID(), agent->GetID(), GetID(), contract_time_diff_ + scan_start_time_ + time / 1000, adc_values, (length - 12) / 2);
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

					TRACE_DEBUG("Request Contract : " << contract_request_mid_ << ", " << GetID() << ", " << GetChannelCount());
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
				
					TRACE_DEBUG("MOtion Detection Started : " << GetID());
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
				
					TRACE_DEBUG("MOtion Detection already Started : " << GetID());
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
				
					TRACE_DEBUG("MOtion Detection Stopped : " << GetID());
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
				
					TRACE_DEBUG("MOtion Detection already stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_MOTION_DETECTED:
			{
				TRACE_DEBUG("Motion Detected received!");

				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Agent::MessageMotionDetected*	message = new Agent::MessageMotionDetected(agent->GetID(), agent->GetID(), GetID());

					TRACE_DEBUG("Notify motion detected : " << GetID());
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
				
					TRACE_DEBUG("Scan Started : " << GetID());
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
				
					TRACE_DEBUG("Scan already Started : " << GetID());
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
				
					TRACE_DEBUG("Scan Stopped : " << GetID());
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
				
					TRACE_DEBUG("Scan already stopped : " << GetID());
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
				
					TRACE_DEBUG("Trans Started : " << GetID());
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
				
					TRACE_DEBUG("Trans already Started : " << GetID());
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
				
					TRACE_DEBUG("Trans Stopped : " << GetID());
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
				
					TRACE_DEBUG("Trans already stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_READY_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageReadyStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_DEBUG("Ready Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_READY_ALREADY_STARTED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageReadyAlreadyStarted(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_DEBUG("Ready already Started : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_READY_STOPPED:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					Message*	message = new Agent::MessageReadyStopped(agent->GetID(), agent->GetID(), GetID());
				
					TRACE_DEBUG("Ready Stopped : " << GetID());
					if (!agent->Post(message))
					{
						delete message;
					}
				}
			}
			break;

		case	MSG_TYPE_KEEP_ALIVE:
			{
				Agent*	agent = dynamic_cast<Agent*>(parent_);
				if (agent != NULL)
				{
					uint32_t	battery = 0;
					
					if (length  == 12)
					{
						battery |= (uint32_t)(data[8] << 24);
						battery |= (uint32_t)(data[9] << 16);
						battery |= (uint32_t)(data[10] <<  8);
						battery |= (uint32_t)(data[11]      );
					}

					Message*	message = new Agent::MessageKeepAlive(agent->GetID(), agent->GetID(), GetID(), (battery / 100) / 10.0);
				
					TRACE_DEBUG("Keep Alive: " << GetID());
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
				TRACE_DEBUG_DUMP(data, length);
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
			TRACE_DEBUG("RSSI : " << rssi_);
			return	true;
		}
	}

	return	false;
}

bool	Node::OnStarted(char* _result)
{
	if (strncasecmp(_result, "OK",2) == 0)
	{
		TRACE_DEBUG("Successfully Started!");
	}
	else
	{
		TRACE_DEBUG("Starting failed! : " << ((_result != NULL)?_result:""));
	}

	return	true;
}

bool	Node::OnConfig(char* _result)
{
	if (strncasecmp(_result, "OK",2) == 0)
	{
		TRACE_DEBUG("Config success!");
	}
	else
	{
		TRACE_DEBUG("Config failed! : " << ((_result != NULL)?_result:""));
	}

	return	true;
}


