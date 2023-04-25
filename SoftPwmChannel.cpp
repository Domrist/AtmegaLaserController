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
		if(!this->state){ //state == false - первое включение и соответствующее переключение флагов после каждого периода
			this->state = true;
			
			PORTD |= (1<<this->port); //включаем определённый порт
			
		}
		this->localMillisCounter+=1;
		if(this->duty <= this->localMillisCounter){
			//отключаем пин на порту
		}
		if(this->period <= this->localMillisCounter){ //заканчиваем период.обнуляем локальный таймер
			this->localMillisCounter = 0;
			this->state = false;
		}
	}
}

void SoftPwmChannel::startSoftPwm(){

	this->stopSoftPwm();//обнуляем всё от греха подальше
	this->isStarted = true; //меняем флаги на стандартные
}


void SoftPwmChannel::stopSoftPwm(){
	this->isStarted = false;
	this->localMillisCounter = 0;
	this->state = false;
	//отрубаем локальный порт
	PORTD &= ~(1<< this->port); //отключаем порт
}