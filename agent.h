#ifndef	AGENT_H__
#define	AGENT_H__

#include <list>
#include "node.h"
#include "activeobject.h"
#include "mqttclient.h"

class	Agent : public ActiveObject
{
public:
	struct MFL 
	{
		MFL(uint16_t _x = 0, uint16_t _y = 0, uint16_t _z = 0): x(_x), y(_y), z(_z) {};
		uint16_t	x;
		uint16_t	y;
		uint16_t	z;
	};
	class	MessageData: public Message	
	{
		public:
			MessageData(std::string const& _receiver, std::string const& _sender, uint32_t _time, uint16_t* _data, uint32_t _length);
			~MessageData();

			uint32_t	GetTime(void);
			uint32_t	GetLength()	{return	length_;}
			uint16_t	GetValue(uint32_t index);

		protected:
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

		bool	OnServerRequest(std::string const& _message);
		bool	OnReceived(Message* _message);

protected:
	std::list<Node*>	node_list_;
	MQTTClient	client_;

	uint32_t	data_time;
	std::list<MFL>		mfl_list_;

	void	Preprocess();
	void	Process();
	void	Postprocess();

	static	bool	SetNode(Object* _object, JSONNode const& _value);
	static	bool	SetServer(Object* _object, JSONNode const& _value);
	
	static	bool	ReceiveData(ActiveObject *_object, Message* _message);
};
#endif
