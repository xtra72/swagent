#ifndef	MESSAGE_H__
#define MESSAGE_H__

#include <string>
#include "time.h"
#include "timer.h"
#include "libjson/libjson.h"

class	MessageResult
{
public:
	MessageResult(uint32_t _buffer_size = 0);
	virtual	~MessageResult();

	bool		IsExpired();
	void		Expire(uint32_t _result);

	uint32_t	Result();
	bool		SetData(uint8_t* _data, uint32_t _data_length);
	uint8_t*	GetData();
	uint32_t	GetDataLength();
protected:
	bool		expired_;
	uint32_t	result_;
	uint8_t*	buffer_;
	uint32_t	buffer_size_;
	uint32_t	data_length_;
};

#define		MESSAGE_TYPE_UNKNOWN				0
#define		MESSAGE_TYPE_PACKET_RECEIVED		1
#define		MESSAGE_TYPE_PACKET_SEND			2
#define		MESSAGE_TYPE_PACKET_SEND_CONFIRM	3
#define		MESSAGE_TYPE_SERVER_REQUEST			4
#define		MESSAGE_TYPE_SERVER_RESPONSE		5
#define		MESSAGE_TYPE_REPORT_STATUS			6
#define		MESSAGE_TYPE_REPORT_VALUE			7
#define		MESSAGE_TYPE_REPORT_VALUES			8
#define		MESSAGE_TYPE_RESPONSE_REPORT_VALUE	9

class	Message
{
public:
			Message(const Message& _message);
			Message(uint32_t	_type, std::string const& _receiver, std::string const& _sender);
	virtual	~Message();

			uint32_t	GetType()			{	return	type_;	}
			uint64_t	GetID()				{	return	id_;	}
			Date		GetCreationTime()	{	return	creation_time_;	}

	const	std::string&	GetSender()		{	return	sender_;	}
	const	std::string&	GetReceiver()	{	return	receiver_;	}

			bool		SetRetentionTime(Time _retention_time);
			Time		GetRemainTime();
			bool		IsExpired();


	virtual	void		Dump(std::ostream& os) const;
	static	uint32_t	ToType(std::string const& _string);
			std::string	ToString (uint32_t	_type);

	friend	std::ostream& ::operator<<(std::ostream& os, Message const& _message);

protected:
	uint32_t	type_;
	std::string	receiver_;
	std::string	sender_;
	uint64_t	id_;
	Date		creation_time_;
	Timer		expire_timer_;

			void	SetReceiver(std::string const& _receiver);

	friend 	class	MessageQueue;
};

class	MessagePacketReceived : public Message	
{
public:
	MessagePacketReceived(std::string const& _receiver, std::string const& _sender, void const* _pdata, uint32_t _length);
	~MessagePacketReceived();

			uint32_t	GetSize()	{	return	length_; }
			uint8_t*	GetData()	{	return	data_;	}
			uint8_t		GetData(int n)	{	return	data_[n];	}

	virtual	void		Dump(std::ostream& os) const;

protected:
	uint8_t*	data_;
	uint32_t	length_;
};

class	MessageLoRaPacketReceived : public MessagePacketReceived
{
public:
	MessageLoRaPacketReceived(std::string const& _receiver, std::string const& _sender, uint32_t _port, void const* _pdata, uint32_t _length);
	~MessageLoRaPacketReceived();

			uint32_t	GetPort(void)	{	return	port_;	}

protected:
	uint32_t	port_;
};

class	MessagePacketSend : public Message	
{
public:
	MessagePacketSend(std::string const& _receiver, std::string const& _sender, void const* _pdata, uint32_t _length);
	~MessagePacketSend();

			uint32_t	GetSize()	{	return	length_; }
			uint8_t*	GetData()	{	return	data_;	}
			uint8_t		GetData(int n)	{	return	data_[n];	}

	virtual	void		Dump(std::ostream& os) const;

protected:
	uint8_t*	data_;
	uint32_t	length_;
};

class	MessagePacketSendConfirm : public MessagePacketSend
{
public:
	MessagePacketSendConfirm(std::string const & _receiver, std::string const& _sender, void const* _pdata, uint32_t _length, MessageResult* _result);

			void		Expire(uint32_t _result);
			bool		SetData(uint8_t* _data, uint32_t _data_length);

protected:
	MessageResult*	result_;
};


class	MessageServerResponse : public Message	
{
public:
	MessageServerResponse(std::string const& _receiver, std::string const& _sender, JSONNode const& _result);
	~MessageServerResponse();


	const JSONNode&	Result()	{	return	result_;}
protected:
	JSONNode	result_;
};

class	MessageServerRequest: public Message	
{
public:
	MessageServerRequest(std::string const& _receiver, std::string const& _sender, JSONNode const& _params);
	MessageServerRequest(std::string const& _receiver, std::string const& _sender, std::string const& _message_id, JSONNode const& _params);
	~MessageServerRequest();

	const JSONNode& Params()	{	return	params_; }


protected:
	JSONNode params_;
};

class	MessageReportStatus : public Message	
{
public:
	MessageReportStatus(std::string const& _receiver, std::string const& _sender, std::string const& _endpoint_id, bool _status, uint32_t _duration);
	~MessageReportStatus();

	const std::string& 	GetEndpointID()	{ return	endpoint_id_;}
	bool				GetStatus()	{ return	status_;}
	uint32_t			GetDuration()	{ return	duration_;}

protected:
	std::string endpoint_id_;
	bool		status_;
	uint32_t	duration_;
	
};


#endif
