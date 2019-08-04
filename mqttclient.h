#ifndef	MQTT_CLIENT_H__
#define	MQTT_CLIENT_H__

#include <map>
#include <list>
#include <string>
#include "libjson/libjson.h"
#include "mosquitto.h"
#include "thread.h"
#include "time.h"
#include "timer.h"
#include "message.h"

#define		MESSAGE_TYPE_CONNECTED				1000
#define		MESSAGE_TYPE_DISCONNECTED			1001
#define		MESSAGE_TYPE_MQTT_PUBLISH			1002
#define		MESSAGE_TYPE_MQTT_ESSAGE			1003

class	MQTTClient : public Object
{
public:
	class	MessageConnected: public Message	
	{
	public:
		MessageConnected(std::string const& _receiver, std::string const& _sender);
		~MessageConnected();
	};

	class	MessageDisconnected: public Message	
	{
	public:
		MessageDisconnected(std::string const& _receiver, std::string const& _sender);
		~MessageDisconnected();
	};

	class	Publisher : public Object
	{
	public:
		Publisher(std::string const& _topic);
		Publisher(std::string const& _topic, std::string const& _message);
		Publisher(std::string const& _topic, JSONNode const& _message);

		std::string	const&	Topic(void);
		std::string	const&	Message(void);

				uint32_t	GetQoS(void);
				bool		SetQoS(uint32_t _qos);

				void		SetMID(int _mid);
				int			GetMID();

				void		SetPublishDate(Date const& _date);

		virtual	void		Finished(void);
	protected:
		std::string	topic_;
		std::string	message_;
		int			mid_;
		Date		create_date_;
		Date		publish_date_;
		uint32_t	qos_;
	};

	class	Subscriber : public Object
	{
	public:
		Subscriber(std::string const& _topic);

		std::string	const&	Topic(void);

		void	SetMID(int _mid);
		int		GetMID();

	virtual	void	OnMessage(std::string const& _message);
	protected:
		std::string	topic_;
		int			mid_;
		Date		create_date_;
	};


	MQTTClient(std::string const& _id = "", Object* _parent = NULL);
	~MQTTClient();

			std::string GetClientID(void);
			bool		SetClientID(std::string const& _client_id);

			std::string	GetUsername(void);
			bool		SetUsername(std::string const& _username);

			std::string	GetPassword(void);
			bool		SetPassword(std::string const& _password);

			std::string	GetServerURL(void);
			bool		SetServerURL(std::string const& _url);

			uint16_t	GetServerPort(void);
			bool		SetServerPort(uint16_t _port);

			bool		GetProperties(JSONNode& _properties);

			bool		Register(Subscriber* subscribe);
			Subscriber*	GetSubscriber(std::string const& _topic);

	virtual	bool		Init(void);
			void		Start();
			void		Stop();
			
			bool		Connect();
			bool		Disconnect();
			

			bool		Publish(Publisher* publish);

	virtual	bool		Send(uint8_t *_data, uint32_t _data_len);

	virtual	void		OnConnect(int result);
	virtual	void		OnDisconnect(int rc);
	virtual	void		OnPublish(int mid);
	virtual	void		OnSubscribe(int mid, int qos_count, const int *granted_qos);
	virtual	void		OnMessage(const struct mosquitto_message *_message);
	virtual	void		OnLog(int level, const char *str);

protected:
	struct mosquitto*	mosquitto_;	
	uint32_t			protocol_version_;

			std::string	server_url_;
			uint16_t	server_port_;
			bool		with_tls_;
			std::string	ca_cert_file_;
			Time        keep_alive_;
			Time        server_retry_interval_;
			Timer       server_retry_timeout_;
			bool		server_connected_;
			bool		first_connection_;
			uint32_t	server_retry_count_;

			std::string	client_id_;
			std::string	username_;
			std::string	password_;
	
			Thread		thread_;
			bool		stop_;

	std::map<std::string, Subscriber*>	subscribe_map_;
	std::list<Publisher*>				publish_list_;

			void		Preprocess(void);
			void		Process(void);
			void		Postprocess(void);

	static 	bool		ConfigURL(Object* _client, JSONNode const& _value);
	static	bool		ConfigPort(Object* _client, JSONNode const& _value);
	static 	bool		ConfigClientID(Object*  _client, JSONNode const& _value);
	static 	bool		ConfigUsername(Object* _client, JSONNode const& _value);
	static 	bool		ConfigPassword(Object* _client, JSONNode const& _value);
	static 	bool		ConfigWithTLS(Object* _client, JSONNode const& _value);
	static 	bool		ConfigCACertFile(Object* _client, JSONNode const& _value);

	friend	void		::OnConnectCB(struct mosquitto *mosq, void *obj, int result);
	friend	void		::OnDisconnectCB(struct mosquitto *mosq, void *obj, int rc);
	friend	void		::OnPublishCB(struct mosquitto *mosq, void *obj, int mid);
	friend	void		::OnSubscribeCB(struct mosquitto *_mosq, void *_obj, int mid, int qos_count, const int *granted_qos);
	friend	void		::OnMessageCB(struct mosquitto *_mosq, void *_obj, const struct mosquitto_message *_message);
	friend	void		::OnLogCB(struct mosquitto *mosq, void *obj, int level, const char *str);

	friend				std::ostream& operator<<(std::ostream& out, MQTTClient const& _client) ;

	static	void*		ThreadMain(void* _data);
};

#endif
