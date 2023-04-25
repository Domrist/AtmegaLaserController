/*
 * FuckingTimer.cpp
 *
 * Created: 18.08.2022 21:11:07
 * Author : domri
 */ 
#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//block for display

#define e1 PORTB |= 0b00001000;
#define e0 PORTB &= 0b11110111;
#define rs1 PORTB |= 0b00000100;
#define rs0 PORTB &= 0b11111011;

void sendHalfByte(unsigned char c){
  c <<= 4;
  e1;
  _delay_us(50);
  PORTB &= 0b00001111;
  PORTB |= c;
  e0;
  _delay_us(50);
}

void sendByte(unsigned char c,unsigned char mode){
  if (mode == 0){
    rs0;
  }
  else{
    rs1;
  }
  unsigned char hc = 0;
  hc = c >> 4;
  sendHalfByte(hc);
  sendHalfByte(c);
}

void lcdIni(){
  DDRB = 0xFF;
  PORTB = 0x00;
  _delay_ms(15);
  sendHalfByte(0b00000011);
  _delay_ms(15);
  sendHalfByte(0b00000011);
  _delay_ms(100);
  sendHalfByte(0b00000011);
  _delay_ms(15);
  sendHalfByte(0b00000010);
  _delay_ms(15);
  sendByte(0b00101000,0); //����� ����������� ������ �� 4 ���� � ���������� ����� - 2
  _delay_ms(15);
  sendByte(0b00001100,0); // �������� ����������� �� �������, ������� �� ��������
  _delay_ms(15);
  sendByte(0b00000110,0); //��������� ������ ����� ��������� �����
  _delay_ms(15);
  
}

void sendChar(unsigned char c){
  sendByte(c,1);
}

void sendString(char str[]){
  wchar_t n;
  for(n = 0;str[n] != '\0';n++){
    sendChar(str[n]);
  }
}


void setPos(unsigned char x,unsigned char y){
  char address = 0;
  if(y == 0){
    address = (0x80 + x) | 0b10000000;
  }
  else if(y == 1){
    address = (0xC0 + x) | 0b10000000;
  }
  else if(y == 2){
    address = (0x94 + x) | 0b10000000; 
  }
  else if(y == 3){
    address = (0xD4 + x) | 0b10000000;
  }
  sendByte(address,0);
  
}

void displayReturnHome(){
  sendByte(0b00000010,0);
}

void clearDisplay(){
  sendByte(0b00000001,0);
}

//end display block

class SoftPwmChannel{
  public:
    //duty,period - in millis
    //duty - ��������
    
    int localMillis;
    float duty;
    float period;
    int port;
    bool state;
    bool isStarted;
    bool enabled;
    char edgeStatus; //0 - default,1 - rising edge,2 falling edge
	bool firstEnabled;
	bool fallingStateOnce;
	SoftPwmChannel *callbackObject;
  
    SoftPwmChannel(){
        
    }
	//� �������� ������� ���������� ������� ���� ����� - 
    SoftPwmChannel(int _port,int _duty,int _period){
      this->localMillis = 0;
      this->port = _port;
      this->period = _period;
      this->duty = period/_duty;
      this->enabled = true;
	  this->firstEnabled = false;
	  this->callbackObject = NULL;
    }
    ~SoftPwmChannel(){
    }
    
    bool getEnabled(){
      return this->enabled;
    }
    
    void startSoftPwm(){
      
      if(!this->enabled){ //���� ����� ��������,�� �� ������ ���� ��������� � ������� �� ������� �������
        return;
      }
	  this->stopSoftPwm(); //����� ���������� ������ ��� ����� �� �������� ��������� ���. ���������� ��� ���� ���������
      this->isStarted = true; // � ������ ���� �������
    }
    
    void stopSoftPwm(){
      this->isStarted = false;
      this->localMillis = 0;
      this->state = false;
      //�������� ��������� ����
      PORTD &= ~(1<< this->port); //��������� ����
    }
    
    int getDutyPercent(){
      return this->period/this->duty;
    }
    
    int getPeriod(){
      return this->period;
    }
    
    bool getEnabledState(){
      return this->state;
    }
    
    void changeDuty(){
      int t = this->period/this->duty;
	  if(t == 10){
		t = 2;
	  }
	  else if(t < 10){
		t++;  
	  }
	  
      this->duty = this->period/t;
    }
        
    void recalculateDuty(int newPeriod){ // ����� ������ ����������� ������ ��� ����� ��� ��������� ����������� �������
      this->period = newPeriod;
      this->duty = this->period/this->duty;
    }
    
    void updateSoftPwm(){
      if(this->isStarted){
        if(!this->state){ //state == false - ������ ��������� � ��������������� ������������ ������ ����� ������� �������
          this->state = true;
          PORTD |= (1<< (this->port) ); //�������� ����������� ����
        }
        this->localMillis +=1;
        if(this->duty <= this->localMillis){
          //��������� ��� �� �����
          PORTD &= ~(1 << (this->port) );
		  if(this->callbackObject != NULL){ //���� ��� ������ ����������  - �������� �������
			
			this->callbackObject->startSoftPwm();
			this->callbackObject = NULL;
		  }
        }
		//���� ��������� ������� ��������� �� ������� ������� - �������� ������� � ����������� ����
        if(this->period <= this->localMillis){ //����������� ������.�������� ��������� ������
          this->localMillis = 0;
          this->state = false;
        }
      }      
    }
};

SoftPwmChannel redChannel(5,2,2000);
SoftPwmChannel greenChannel(6,2,2000);
SoftPwmChannel blueChannel(7,2,2000);
SoftPwmChannel* channelMas[3] = {&redChannel,&greenChannel,&blueChannel};
SoftPwmChannel *currentChannel = channelMas[0];

char currentChannelIndex = 0;
int globalPeriod = 2000;

bool parallelMode = true;
bool isWorking = false;

void nextChannel(){
  currentChannelIndex +=1;
  if(currentChannelIndex == 3){ //���� ������������� � ���������� ������ �� ������
    currentChannel = 0;
  }
  else if(currentChannelIndex==4){
    currentChannelIndex = 0;
    currentChannel = &redChannel;
  }
  else if(currentChannelIndex==1){
    currentChannel = &greenChannel;
  }
  else if(currentChannelIndex==2){
    currentChannel = &blueChannel;
  }
}

void disableEnableCurrentChannel(){
  if( currentChannel == &redChannel || currentChannel == &greenChannel || currentChannel == &blueChannel){
    currentChannel->enabled = !currentChannel->enabled;
  }
}

void displayInfo(){
  //"435nm 100% Disabled<"- ������
  clearDisplay();
  _delay_ms(100);
  setPos(0,0);
  char line[20];
  char duty[3];
  sprintf(duty,"1/%d",redChannel.getDutyPercent());
  strcat(line,"435nm ");
  strcat(line,duty);
  strcat(line," ");
  strcat(line,redChannel.getEnabled() ? "Enabled":"Disabled");
  strcat(line,currentChannel == &redChannel ? "<":"\0" );
  sendString(line);
  memset(line,0,20);
  memset(duty,0,3);
  
  setPos(0,1);
  char line2[20];
  char duty2[3];
  strcat(line2,"532nm ");
  sprintf(duty2,"1/%d",greenChannel.getDutyPercent());
  strcat(line2,duty2);
  strcat(line2," ");
  strcat(line2,greenChannel.getEnabled() ? "Enabled":"Disabled");
  strcat(line2,currentChannel == &greenChannel ? "<":"\0" );
  sendString(line2);
  memset(line2,0,20);
  memset(duty2,0,3);
  
  
  setPos(0,2);
  char line3[20];
  char duty3[3];
  strcat(line3,"635nm ");
  sprintf(duty3,"1/%d",blueChannel.getDutyPercent());
  strcat(line3,duty3);
  strcat(line3," ");
  strcat(line3,blueChannel.getEnabled() ? "Enabled":"Disabled");
  strcat(line3,currentChannel == &blueChannel ? "<":"\0" );
  sendString(line3);
  memset(line3,0,20);
  memset(duty3,0,3);
  
  setPos(0,3);
  
  char per[10];
  char line4[20];
  strcat(line4,"Period ");
  sprintf(per,"%d",globalPeriod);
  strcat(line4,per);
  strcat(line4,currentChannelIndex==3?"<":"\0");
  sendString(line4);
  memset(line4,0,20);
  memset(per,0,10);
  
  _delay_ms(50);
}

void initKeyboard(void){
  DDRC = 0x00; // ��������� ���� C ��� ������ ��� ����
  PORTC = 0b00111111;//����������� ���������
  //��� ��� ���� ������� ����� ��� �������� ����������
  /*
    0 - ����������� ������� �����
    1 - ��������/��������� �����
    2 - ����� ������ ������ : ������������/����������������
    3 - ������/���� ������ ���-�������
    4-5 - ���������/��������� �������� ��������
  */
}

void changeValue(char sign){
  if(currentChannel == &redChannel || currentChannel == &greenChannel || currentChannel == &blueChannel){
      currentChannel->changeDuty();
    }
    else if(currentChannel == 0){ //����� ������� ������������ ����������� ������� ����� ������������?
      globalPeriod+= (1*sign);
      redChannel.recalculateDuty(globalPeriod);
      greenChannel.recalculateDuty(globalPeriod);
      blueChannel.recalculateDuty(globalPeriod);
    }
}

void updateKeyboard(){
  //����������� �����
  if(!(PINC&0b00000001)){ 
    _delay_ms(250);
    nextChannel();
    displayInfo();
    _delay_ms(250);
  }
  //��������-��������� ������� �����
  if(!(PINC&0b00000010)){ 
    _delay_ms(250);
    disableEnableCurrentChannel();
    displayInfo();
    _delay_ms(250);
  }
  //����� ������
  if(!(PINC&0b00000100)){ 
    _delay_ms(250);
    clearDisplay();
    _delay_ms(150);
    char modeLine[20];
    char selectedMode[20];
    strcat(modeLine,"Selected mode:");
    parallelMode = !parallelMode;
    strcat(selectedMode,parallelMode?"Paralel mode":"Query mode");
    setPos(0,0);
    sendString(modeLine);
    memset(modeLine,0,20);
    setPos(0,1);
    sendString(selectedMode);
    memset(selectedMode,0,20);
    _delay_ms(1000);
    clearDisplay();
    _delay_ms(150);
    displayInfo();
    _delay_ms(150);
  }
  //������-���� ����������� ���
  if(!(PINC&0b00001000)){ 
    _delay_ms(100);
    if(isWorking){ //���� ������ ���-���� ��������, �� �� ��������
      isWorking = false;
	  for(int i = 0;i< 3;i++){
		  channelMas[i]->stopSoftPwm();
	  }
	  /*
	  
      redChannel.stopSoftPwm();
      greenChannel.stopSoftPwm();
      blueChannel.stopSoftPwm();
	  */
    }
    else if(!isWorking){ //���� �� ��������, �� ��������� � ����������� �� ���������� ������
      isWorking = true;
      if(parallelMode){ //���� ������������ ����� , �� �� ������
        redChannel.startSoftPwm();
        greenChannel.startSoftPwm();
        blueChannel.startSoftPwm();
      }
      else if(!parallelMode){ //���� ���������������� �����, �� �������� ����� � ����������� ���-�������
		
        //���� ���������� ��������� ��� �������
		SoftPwmChannel* enabledMas[3] = {NULL,NULL,NULL};
		int currentEnabledIndex = 0;
		int countOfEnabledChannels = 0;
		
		for(int i = 0;i<3;i++){
			channelMas[i]->callbackObject = NULL;
			if(channelMas[i]->enabled){
				countOfEnabledChannels++;
				enabledMas[currentEnabledIndex] = channelMas[i];
				currentEnabledIndex++;
			}
		}
		
		if(countOfEnabledChannels == 3){
			enabledMas[0]->callbackObject = enabledMas[1];
			enabledMas[1]->callbackObject = enabledMas[2];
			channelMas[0]->startSoftPwm();

		}
		if(countOfEnabledChannels == 2){
			enabledMas[0]->callbackObject = enabledMas[1];
			enabledMas[0]->startSoftPwm();
		}
		if(countOfEnabledChannels ==1){
			enabledMas[0]->startSoftPwm();
		}
      }
    }
    _delay_ms(100);
  }
  //����������� �������� ������ ��� ������ ������� �����
  if(!(PINC&0b00010000)){ 
    _delay_ms(100);
	clearDisplay();
    changeValue(1);
	_delay_ms(50);
	displayInfo();
    _delay_ms(100);
  }
  //��������� �������� ������� ��� �������
  if(!(PINC&0b00100000)){ 
    _delay_ms(100);
    clearDisplay();
    changeValue(1);
    _delay_ms(50);
    displayInfo();
    _delay_ms(100);
  }
}

int millis;

ISR(TIMER1_COMPA_vect){
  
  redChannel.updateSoftPwm();
  greenChannel.updateSoftPwm();
  blueChannel.updateSoftPwm();
  
  TCNT1=0; //�������� ������
}

int main(void)
{
  initKeyboard();
  lcdIni();
  
  millis = 0;
  TCCR1A=0x00; //��������� �������
  TCCR1B=0x03; // ������������ �� 64. ���� = 15 625
  TCNT1=0x00; //����� ������������� ����
  OCR1A = 0x0F; //1; //3D09 //9C - ����� ��� ���� ����� ������� ����������� � ���� ������ ��� � �������
  DDRD = 0xFF;
  PORTD = 0x00;
  TIMSK=0x10; //��������� ������
  
  sei();
   displayInfo();
  while (1) //������ ���� ���� �� ���, �� ����� ��� ����� �� ��������
  {
    updateKeyboard();
    
  };
}