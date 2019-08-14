#ifndef	AGENT_H__
#define	AGENT_H__

#include <list>
#include <vector>
#include "timer.h"
#include "node.h"
#include "activeobject.h"
#include "mqttclient.h"

#define		MESSAGE_TYPE_DATA								101
#define		MESSAGE_TYPE_REQUEST_CONTRACT					102
#define		MESSAGE_TYPE_MOTION_DETECTION_STARTED			103
#define		MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STARTED	104
#define		MESSAGE_TYPE_MOTION_DETECTION_STOPPED			105
#define		MESSAGE_TYPE_MOTION_DETECTION_ALREADY_STOPPED	106
#define		MESSAGE_TYPE_MOTION_DETECTED					107
#define		MESSAGE_TYPE_SCAN_STARTED			108
#define		MESSAGE_TYPE_SCAN_ALREADY_STARTED	109
#define		MESSAGE_TYPE_SCAN_STOPPED			110
#define		MESSAGE_TYPE_SCAN_ALREADY_STOPPED	111
#define		MESSAGE_TYPE_TRANS_STARTED			112
#define		MESSAGE_TYPE_TRANS_ALREADY_STARTED	113
#define		MESSAGE_TYPE_TRANS_STOPPED				114
#define		MESSAGE_TYPE_TRANS_ALREADY_STOPPED		115
#define		MESSAGE_TYPE_KEEP_ALIVE					116

class	Agent : public ActiveObject
{
public:
	class	MessageData: public Message	
	{
		public:
			MessageData(std::string const& _receiver, std::string const& _sender, std::string const& _node_id, uint32_t _time, uint16_t* _data, uint32_t _length);
			~MessageData();

			const std::string&	GetNodeId(void)	{	return	node_id_;	}
					uint32_t	GetTime(void);
					uint32_t	GetLength()	{return	length_;}
					uint16_t	GetValue(uint32_t index);

		protected:
			std::string	node_id_;
			uint32_t	time_;
			uint16_t*	data_;
			uint32_t	length_;
	};

	class	MessageRequestContract: public Message	
	{
		public:
			MessageRequestContract(std::string const& _receiver, std::string const& _sender, std::string const& _nid, uint32_t _channel_count);
			~MessageRequestContract();

			const std::string&	GetNID(void)	{	return	nid_;	}
					uint32_t	GetChannelCount(void)	{	return	channel_count_;	};

		protected:
			std::string	nid_;
			uint32_t	channel_count_;
	};

	class	MessageMotionDetectionStarted : public Message	
	{
		public:
			MessageMotionDetectionStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageMotionDetectionStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageMotionDetectionAlreadyStarted : public Message	
	{
		public:
			MessageMotionDetectionAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageMotionDetectionAlreadyStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageMotionDetectionStopped : public Message	
	{
		public:
			MessageMotionDetectionStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageMotionDetectionStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageMotionDetectionAlreadyStopped : public Message	
	{
		public:
			MessageMotionDetectionAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageMotionDetectionAlreadyStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageMotionDetected: public Message	
	{
		public:
			MessageMotionDetected(std::string const& _receiver, std::string const& _sender, std::string const& _node_id);
			~MessageMotionDetected();

			const std::string&	GetNodeId(void)	{	return	node_id_;	}

		protected:
			std::string	node_id_;
	};

	class	MessageScanStarted : public Message	
	{
		public:
			MessageScanStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageScanStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageScanAlreadyStarted : public Message	
	{
		public:
			MessageScanAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageScanAlreadyStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageScanStopped : public Message	
	{
		public:
			MessageScanStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageScanStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageScanAlreadyStopped : public Message	
	{
		public:
			MessageScanAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageScanAlreadyStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageTransStarted : public Message	
	{
		public:
			MessageTransStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageTransStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageTransAlreadyStarted : public Message	
	{
		public:
			MessageTransAlreadyStarted(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageTransAlreadyStarted();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageTransStopped : public Message	
	{
		public:
			MessageTransStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageTransStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageTransAlreadyStopped : public Message	
	{
		public:
			MessageTransAlreadyStopped(std::string const& _receiver, std::string const& _sender, std::string const& _nid);
			~MessageTransAlreadyStopped();

			const std::string&	GetNID(void)	{	return	nid_;	}

		protected:
			std::string	nid_;
	};

	class	MessageKeepAlive: public Message	
	{
		public:
			MessageKeepAlive(std::string const& _receiver, std::string const& _sender, std::string const& _nid, float _battery);
			~MessageKeepAlive();

			const std::string&	GetNID(void)	{	return	nid_;	}
				float			GetBattery(void)	{	return	battery_;	}

		protected:
			std::string	nid_;
			float		battery_;
	};

	class	ContractResponseReceiver : public MQTTClient::Subscriber
	{
	public:	
		ContractResponseReceiver(Agent* _parent, std::string const& _topic) ;

		virtual	void	OnMessage(std::string const& _message);
	};

	class	ServerRequestReceiver : public MQTTClient::Subscriber
	{
	public:	
		ServerRequestReceiver(Agent* _parent, std::string const& _topic) ;

		virtual	void	OnMessage(std::string const& _message);
	};

	Agent();
	Agent(std::string const& _id);

		Node*	GetNode(std::string const& _id);

		bool	StartStatusReport(void);
		bool	StopStatusReport(void);
		bool	StartContractRequestToServer(void);
		bool	StopContractRequestToServer(void);
		bool	ContractRequestToServer(void);
		bool	ControlResponseToServer(void);
		bool	ContractRequestToServer(char* _node_id, uint8_t _channel_count);
		bool	PushMotionDetectedToServer(std::string const& _nodeId);
		bool	PushDataToServer(std::string const& _payload);
		bool	PushStatusToServer(std::string const& _nid, float _battery);
		bool	PushEncoderDataToServer(int32_t _count);
		bool	PushEncoderDataToServer(uint32_t _time, int32_t _count);
		bool	PushResponseToServer(std::string const& _nodeId, bool _result, std::string const& _message);
		bool	PushLogNotificationToServer(std::string const& _nodeId, std::string const& _message);

		bool	OnPlusData(std::string const& _node_id, char* _data);
		bool	OnPlusLog(std::string const& _node_id, char* _log);
		bool	OnStat(std::string const &_node_id, char* _stat);
		bool	OnEncoder(std::string const &_node_id, char* _stat);
		bool	OnStarted(std::string const &_node_id, char* _result);
		bool	OnServerRequest(std::string const& _message);
		bool	OnReceived(Message* _message);
		bool	OnContractResponse(std::string const& _message);

protected:
	std::list<Node*>	node_list_;
	MQTTClient			client_;
	IntervalTimer		contract_timer_; 
	std::string			contract_request_mid_;
	IntervalTimer		status_report_timer_; 
	uint32_t			status_report_interval_;
	uint32_t			contract_request_interval_;
	bool				plus_log_output_;
	bool				check_mid_;
	bool				time_order_;
	uint32_t			record_time_;
	uint32_t			encoder_start_time_;
	std::list<int32_t>	encoder_count_;

	uint32_t	data_time;
	std::list<std::vector<uint16_t>>		mfl_list_;

	void	Preprocess();
	void	Process();
	void	Postprocess();

	static	bool	SetNode(Object* _object, JSONNode const& _value);
	static	bool	SetServer(Object* _object, JSONNode const& _value);
	static	bool	SetLog(Object* _object, JSONNode const& _value);
	static	bool	SetStatusReportInterval(Object* _object, JSONNode const& _value);
	static	bool	SetContractRequestInterval(Object* _object, JSONNode const& _value);
	static	bool	SetDataOrder(Object* _object, JSONNode const& _value);
	
	static	bool	OnMessageConnectedCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageDisconnectedCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageDataCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageDataCallback2(ActiveObject *_object, Message* _message);
	static	bool	OnMessageMotionDetectedCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageConfirmCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageRequestContractCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageKeepAlive(ActiveObject *_object, Message* _message);
	static	void	OnContractRequest(void* _params);
	static	void	OnStatusReport(void* _params);
};
#endif
