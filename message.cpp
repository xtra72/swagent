#include <string.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include "message.h"
#include "mutex.h"
#include "activeobject.h"

using namespace std;

struct	MessageTypeString
{
	uint32_t		type;
	string			name;
}
message_type_string[] =
{
	{ TYPE_UNKNOWN,			"Unknown" },
	{ TYPE_PACKET_RECEIVED,	"Packet received" },
	{ TYPE_PACKET_SEND,		"Packet send" }
};

static Locker							message_list_locker;
static std::map<uint64_t, Message*>		message_map_;

MessageResult::MessageResult(uint32_t _buffer_size)
: expired_(false), result_(0), buffer_(NULL), buffer_size_(0), data_length_(0)
{
	if (_buffer_size)
	{
		buffer_ = new uint8_t[_buffer_size];
		buffer_size_ = _buffer_size;
	}
}

MessageResult::~MessageResult()
{
	if (buffer_)
	{
		delete buffer_;	
	}
}

bool	MessageResult::IsExpired()
{
	return	expired_;
}


void	MessageResult::Expire(uint32_t _result)
{
	result_ = _result;
	expired_ = true;
}

uint32_t	MessageResult::Result()
{
	return	result_;
}

bool		MessageResult::SetData(uint8_t* _data, uint32_t _data_length)
{
	if (_data_length < buffer_size_)
	{
		memcpy(buffer_, _data, _data_length);
		data_length_ = _data_length;

		return	true;
	}

	return	false;
}

uint8_t*	MessageResult::GetData()
{
	if (data_length_)
	{
		return	buffer_;
	}

	return	NULL;
}

uint32_t	MessageResult::GetDataLength()
{
	return	data_length_;
}

Message::Message(const Message& _message)
: type_(_message.type_), receiver_(_message.receiver_), sender_(_message.sender_) 
{
	creation_time_ = Date::GetCurrent();
	id_ = creation_time_.GetMicroseconds();
	SetRetentionTime(5000000);

	message_map_[id_] = this;
}

Message::Message(uint32_t	_type, std::string const& _receiver, std::string const& _sender)
: type_(_type), receiver_(_receiver), sender_(_sender) 
{
	creation_time_ = Date::GetCurrent();
	id_ = creation_time_.GetMicroseconds();
	SetRetentionTime(5000000);

	message_map_[id_] = this;
}

Message::~Message()
{
	std::map<uint64_t, Message*>::iterator	it = message_map_.find(id_);
	if (it != message_map_.end())
	{
		message_map_.erase(it);
	}
}

void	Message::SetReceiver(std::string const& _receiver)
{	
	receiver_ = _receiver;	
}

bool	Message::SetRetentionTime(Time _retention_time)
{
	expire_timer_.Set(creation_time_);
	expire_timer_.Add(_retention_time);

	return	true;
}

Time	Message::GetRemainTime()
{
	return	expire_timer_.RemainTime();
}

bool	Message::IsExpired()
{
	return	expire_timer_.RemainTime() == 0;
}

const string&	ToString
(
	Message* _message
)
{
	static	string	unknown("unknown");

	for(int i = 0 ; sizeof(message_type_string) / sizeof(MessageTypeString) ; i++)
	{
	 	if (_message->GetType() == message_type_string[i].type)
		{
			return	message_type_string[i].name;
		}
	}

	return	unknown;
}

uint32_t	Message::ToType(std::string const& _string)
{
	for(int i = 0 ; sizeof(message_type_string) / sizeof(MessageTypeString) ; i++)
	{
	 	if (_string == message_type_string[i].name)
		{
			return	message_type_string[i].type;
		}
	}

	return	TYPE_UNKNOWN;
}

std::string	Message::ToString ( uint32_t _type)
{
	for(int i = 0 ; sizeof(message_type_string) / sizeof(MessageTypeString) ; i++)
	{
	 	if (_type == message_type_string[i].type)
		{
			return	message_type_string[i].name;
		}
	}

	return	message_type_string[0].name;
}

std::ostream& operator<<(std::ostream& os, Message const& _message)
{
	_message.Dump(os);

	return	os;
}

void	Message::Dump(std::ostream& os) const
{
}


MessagePacketReceived::MessagePacketReceived(std::string const& _receiver, std::string const& _sender, void const* _data, uint32_t _length)
: Message(TYPE_PACKET_RECEIVED, _receiver, _sender), length_(_length)
{
	if (_length == 0)
	{
		data_ = NULL;
	}
	else
	{
		data_ = new uint8_t[_length + 1];

		memset(data_, 0, _length + 1);
		memcpy(data_, _data, _length);
	}
}

MessagePacketReceived::~MessagePacketReceived()
{
	if(data_ != NULL)
	{
		delete data_;	
	}
}


void	MessagePacketReceived::Dump(ostream& os) const
{
	uint32_t	i;

	os << "Type : PACKET" << std::endl;
	for(i = 0 ; i < length_ ; i++)
	{
		if ((i % 16) == 0)
		{
			os << std::hex << std::setw(4) << std::setfill('0') << i << " : ";
		}

		os << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t *)data_)[i] << " ";
		if ((i + 1) % 16 == 0)
		{
			os<< std::endl;	
		}
	}

	if (i % 16 != 0)
	{
		os<< std::endl;	
	}
}


MessageLoRaPacketReceived::MessageLoRaPacketReceived(std::string const& _receiver, std::string const& _sender, uint32_t _port, void const* _data, uint32_t _length)
: MessagePacketReceived(_receiver, _sender, _data, _length), port_(_port)
{
}

MessageLoRaPacketReceived::~MessageLoRaPacketReceived()
{
}


MessagePacketSend::MessagePacketSend(std::string const& _receiver, std::string const& _sender, void const* _data, uint32_t _length)
: Message(TYPE_PACKET_SEND, _receiver, _sender), length_(_length)
{
	if (_length == 0)
	{
		data_ = NULL;
	}
	else
	{
		data_ = new uint8_t[_length + 1];

		memset(data_, 0, _length + 1);
		memcpy(data_, _data, _length);
	}
}

MessagePacketSend::~MessagePacketSend()
{
	if(data_ != NULL)
	{
		delete data_;	
	}
}


void	MessagePacketSend::Dump(ostream& os) const
{
	uint32_t	i;

	os << "Type : PACKET" << std::endl;
	for(i = 0 ; i < length_ ; i++)
	{
		if ((i % 16) == 0)
		{
			os << std::hex << std::setw(4) << std::setfill('0') << i << " : ";
		}

		os << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t *)data_)[i] << " ";
		if ((i + 1) % 16 == 0)
		{
			os<< std::endl;	
		}
	}

	if (i % 16 != 0)
	{
		os<< std::endl;	
	}
}


MessagePacketSendConfirm::MessagePacketSendConfirm(std::string const& _receiver, std::string const& _sender, void const* _data, uint32_t _length, MessageResult* _result)
:	MessagePacketSend(_receiver, _sender, _data, _length), result_(_result)
{
	type_ = TYPE_PACKET_SEND_CONFIRM;
}

void	MessagePacketSendConfirm::Expire(uint32_t _result)
{
	result_->Expire(_result);
}

bool	MessagePacketSendConfirm::SetData(uint8_t* _data, uint32_t _data_length)
{
	return	result_->SetData(_data, _data_length);
}

/////////////////////////////////////////////////////////////////
// Server Response
/////////////////////////////////////////////////////////////////
MessageServerResponse::MessageServerResponse(std::string const& _receiver, std::string const& _sender, JSONNode const& _result)
: Message(TYPE_SERVER_RESPONSE, _receiver, _sender), result_(_result)
{
}

MessageServerResponse::~MessageServerResponse()
{
}

/////////////////////////////////////////////////////////////////
// Server Request
/////////////////////////////////////////////////////////////////
MessageServerRequest::MessageServerRequest(std::string const& _receiver, std::string const& _sender, JSONNode const& _params)
: Message(TYPE_SERVER_REQUEST, _receiver, _sender), params_(_params)
{
}

MessageServerRequest::MessageServerRequest(std::string const& _receiver, std::string const& _sender, std::string const& _message_id, JSONNode const& _params)
: Message(TYPE_SERVER_REQUEST, _receiver, _sender), params_(_params)
{
	params_.push_back(JSONNode("message_id", _message_id));
}

MessageServerRequest::~MessageServerRequest()
{
}

/////////////////////////////////////////////////////////////////
// Server Request
/////////////////////////////////////////////////////////////////
MessageReportStatus::MessageReportStatus(std::string const& _receiver, std::string const& _sender, std::string const& _endpoint_id, bool _status, uint32_t _duration)
: Message(TYPE_REPORT_STATUS, _receiver, _sender), endpoint_id_(_endpoint_id), status_(_status), duration_(_duration)
{
}

MessageReportStatus::~MessageReportStatus()
{
}

