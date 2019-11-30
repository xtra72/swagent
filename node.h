#ifndef	NODE_H_
#define	NODE_H_

#include "cp211x.h"
#include "gpio.h"

class	Node : public CP2110
{
public:
	enum MSG_TYPE
	{
		MSG_TYPE_DATA	= 0x01,
		MSG_TYPE_ACK	= 0x02,
		MSG_TYPE_PING	= 0x03,
		MSG_TYPE_CONTRACT_REQUEST = 0x04,
		MSG_TYPE_CONTRACT_CONFIRM = 0x05,
		MSG_TYPE_MOTION_DETECTION_START = 0x06,
		MSG_TYPE_MOTION_DETECTION_STARTED = 0x07,
		MSG_TYPE_MOTION_DETECTION_ALREADY_STARTED = 0x08,
		MSG_TYPE_MOTION_DETECTION_STOP = 0x09,
		MSG_TYPE_MOTION_DETECTION_STOPPED = 0x0A,
		MSG_TYPE_MOTION_DETECTION_ALREADY_STOPPED = 0x0B,
		MSG_TYPE_MOTION_DETECTED = 0x0C,
		MSG_TYPE_SCAN_START = 0x0D,
		MSG_TYPE_SCAN_STARTED = 0x0E,
		MSG_TYPE_SCAN_ALREADY_STARTED = 0x0F,
		MSG_TYPE_SCAN_STOP = 0x10,
		MSG_TYPE_SCAN_STOPPED = 0x11,
		MSG_TYPE_SCAN_ALREADY_STOPPED = 0x12,
		MSG_TYPE_TRANS_START = 0x13,
		MSG_TYPE_TRANS_STARTED = 0x14,
		MSG_TYPE_TRANS_ALREADY_STARTED = 0x15,
		MSG_TYPE_TRANS_STOP = 0x16,
		MSG_TYPE_TRANS_STOPPED = 0x17,
		MSG_TYPE_TRANS_ALREADY_STOPPED = 0x18,
		MSG_TYPE_SLEEP = 0x19,
		MSG_TYPE_STOP= 0x1A,
		MSG_TYPE_READY= 0x1B,
		MSG_TYPE_KEEP_ALIVE = 0x1C,

		MSG_TYPE_READY_STARTED = 0x1D,
		MSG_TYPE_READY_ALREADY_STARTED = 0x1E,
		MSG_TYPE_READY_STOPPED = 0x1F,
		MSG_TYPE_SLEEP_STARTED = 0x20
	};

	Node();
	Node(JSONNode const& _config, Object* _parent = NULL);
	Node(std::string const& _id, Object* _parent = NULL);

			bool		RFStart(void);
			bool		Reset(void);
			bool		ColdReset(void);
			bool		Touch(void);

			bool		IsContracted(void);
			bool		SetContract(bool _contract);

			uint32_t	GetChannelCount(void);
			bool		SetChannelCount(uint32_t _channel_count);

			bool		SetConfig(void);

			uint32_t	GetFrequency(void);
			bool		SetFrequency(uint32_t _frequency);

			uint32_t	GetPower(void);
			bool		SetPower(uint32_t _power);

			bool		GetEncoder(void);
			bool		SetEncoder(bool _enable);
			bool		SetEncoder(uint32_t _count, uint32_t _mid);

			bool		SetResetControl(uint32_t _index);

			bool		SetLiveCheck(bool _live_check);
			bool		GetLiveCheck();

			int32_t		GetRSSI(void);

			bool		GetLog(void);
			bool		SetLog(bool _enable);

			bool		RequestStat(void);

			bool		Command(bool scan, bool trans);
			bool		Ready(uint32_t _mid);
			bool		Scan(bool start, uint32_t _mid);
			bool		Sleep(uint32_t _sleep, uint32_t _mid);
			bool		Detect(bool start, uint32_t _mid);
			bool		Transfer(bool start, uint32_t _mid);
			bool		Contract(uint32_t _time_stamp, uint32_t _mid);
			bool		Downlink(uint8_t* data, uint32_t length);
			bool		OnData(uint8_t* data, uint32_t length);
			bool		OnStat(char* _stat);
			bool		OnStarted(char* _stat);
			bool		OnConfig(char* _stat);
			bool		OnEncoder(int32_t _count);
	virtual	bool		OnRead(uint8_t* _data, uint32_t _length);
	virtual	bool		OnWrite(uint8_t* _data, uint32_t _length);

			uint32_t	GetDataCount(uint32_t _channel);
			uint16_t	GetData(uint32_t _channel, uint32_t _index);
			bool		AppendData(uint32_t _channel, uint16_t _value);
			bool		ClearData(uint32_t _channel);
			bool		ClearDataAll(void);

			uint32_t	GetLastDataTime(void);
			bool		SetLastDataTime(uint32_t _data_time);


protected:
	uint16_t			short_address_;
	uint16_t			channel_count_;
	uint32_t			frequency_;
	uint32_t			power_;
	bool				touch_;
	bool				encoder_;
	bool				reverse_;
	uint64_t			contract_request_mid_;
	bool				contracted_;
	uint32_t			contract_time_;
	int32_t				contract_time_diff_;
	uint32_t			last_update_time_;
	int32_t				rssi_;
	bool				log_;
	uint32_t			scan_start_time_;
	uint32_t			last_data_time_;
	uint32_t			live_timeout_;
	int32_t				encoder_count_;
	Timer				live_timer_;

	std::vector<std::vector<uint16_t>>  channel_data_;
	GPIOOut				*reset_control;
	bool				live_check_;

	virtual	void	Preprocess();
	virtual void	Process();

	static	bool	SetChannelCount(Object* _object, JSONNode const& _value);
	static	bool	SetFrequency(Object* _object, JSONNode const& _value);
	static	bool	SetPower(Object* _object, JSONNode const& _value);
	static	bool	SetEncoder(Object* _object, JSONNode const& _value);
	static	bool	SetResetControl(Object* _object, JSONNode const& _value);
	static	bool	SetLiveCheck(Object* _object, JSONNode const& _value);
};
#endif

