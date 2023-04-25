#ifndef SOFTPWMCHANNEL_H
#define SOFTPWMCHANNEL_H

class SoftPwmChannel{
	public:
	
	int localMillisCounter;
	bool isStarted; 
	bool state;
	
	char port;
	int duty;
	int period;
	
	SoftPwmChannel();
	SoftPwmChannel(int port,int dutyInPercentage,int period);
	~SoftPwmChannel();
	
	void startSoftPwm();
	void stopSoftPwm();
	void updateSoftPwmStatus(int outerMilliseconds);
};

#endif