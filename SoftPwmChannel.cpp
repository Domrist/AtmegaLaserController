#include "SoftPwmChannel.h"

SoftPwmChannel::SoftPwmChannel(){this->state = false;
	this->state = false;
	this->duty = 0;
	this->localMillisCounter = 0;
	this->isStarted = false;
}

SoftPwmChannel::~SoftPwmChannel(){
	
}

SoftPwmChannel::SoftPwmChannel(char port,int dutyInPercentage,int period){
	this->isStarted = false;
	this->state = false;
	this->port = port;
	this->duty = dutyInPercentage;
	this->period = period;
	this->localMillisCounter = 0;
}

void SoftPwmChannel::updateSoftPwmStatus(int outerMilliseconds){
	if(this->isStarted){
		if(!this->state){ //state == false - ������ ��������� � ��������������� ������������ ������ ����� ������� �������
			this->state = true;
			
			PORTD |= (1<<this->port); //�������� ����������� ����
			
		}
		this->localMillisCounter+=1;
		if(this->duty <= this->localMillisCounter){
			//��������� ��� �� �����
		}
		if(this->period <= this->localMillisCounter){ //����������� ������.�������� ��������� ������
			this->localMillisCounter = 0;
			this->state = false;
		}
	}
}

void SoftPwmChannel::startSoftPwm(){

	this->stopSoftPwm();//�������� �� �� ����� ��������
	this->isStarted = true; //������ ����� �� �����������
}


void SoftPwmChannel::stopSoftPwm(){
	this->isStarted = false;
	this->localMillisCounter = 0;
	this->state = false;
	//�������� ��������� ����
	PORTD &= ~(1<< this->port); //��������� ����
}