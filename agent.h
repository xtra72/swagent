#ifndef	AGENT_H__
#define	AGENT_H__

#include <list>
#include <vector>
#include "node.h"
#include "activeobject.h"
#include "mqttclient.h"

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

	class	ServerRequestReceiver : public MQTTClient::Subscriber
	{
	public:	
		ServerRequestReceiver(Agent* _parent, std::string const& _topic) ;

		virtual	void	OnMessage(std::string const& _message);
	};

	Agent();
	Agent(std::string const& _id);

		Node*	GetNode(std::string const& _id);

		bool	ContractRequestToServer(void);
		bool	ControlResponseToServer(void);
		bool	PushDataToServer(std::string const& _payload);
		bool	PushStatusToServer(void);
		bool	PushEncoderDataToServer(void);

		bool	OnPlusData(std::string const& _node_id, char* _data);
		bool	OnServerRequest(std::string const& _message);
		bool	OnReceived(Message* _message);

protected:
	std::list<Node*>	node_list_;
	MQTTClient	client_;

	uint32_t	data_time;
	std::list<std::vector<uint16_t>>		mfl_list_;

	void	Preprocess();
	void	Process();
	void	Postprocess();

	static	bool	SetNode(Object* _object, JSONNode const& _value);
	static	bool	SetServer(Object* _object, JSONNode const& _value);
	
	static	bool	OnMessageConnectedCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageDisconnectedCallback(ActiveObject *_object, Message* _message);
	static	bool	OnMessageDataCallback(ActiveObject *_object, Message* _message);
};
#endif
