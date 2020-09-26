/*
  server_showout
*/
#include "mbed.h"
#include "EthernetInterface.h"
#include "C12832.h"
//init lcd display
C12832 lcd(D11, D13, D12, D7, D10);
//init analougout point
AnalogOut  aout(DAC0_OUT);
static const char* SERVER_IP = "192.168.1.2";
static const char* MASK = "255.255.255.0";
static const char* GATEWAY = "192.168.1.1";
static const char* CLIENT_IP ="192.168.1.1";
//init ethernetinterface
EthernetInterface eth;

//init  digital ios
DigitalOut red(LED_RED);
DigitalOut green(LED_GREEN);
DigitalIn  sig1(PTC12);
DigitalIn  sig2(D9);
DigitalIn swinput1(SW2);
DigitalIn swinput2(SW3);

//init 2 timer
Timer t;
Timer tt;

//init 4 threads
Thread *thp_eth;
Thread *thp_clock;
Thread *transmitter;
Thread *receiver;

int n;
// The counter array is used to store the  received car  signal data
char counter[7] = {0};
// The counter1 array is used to store current traffic light signal data
char counter1[4] = {0};
//The info array is used to store the one-bit clock signal sent by the Smartmesh IP system
int info1[3]= {0};
//The info array is used to store the one-bit real signal sent by the Smartmesh IP system
int info2[3]= {0};

//Definition of functions
void show(void);
void init_eth(void const *args);
void transmit(void const *args);
void receive(void const *args);
int main(void);
void start(void);
int check(int x);
int getinfo(int x);
void init(void);
void sentinfo(void);
int stat(void);
void analogout(int);
void ask(void);
void connect_mash(void const *args);
void clock(void const *args);

//use sw1 and sw2 buttons to control lcd 
void show(void)
{
    int r,g,y;
    r=(swinput1==1&&swinput2==0)?0:1;
    g=(swinput1==0&&swinput2==1)?0:1;
    y=(swinput1==0&&swinput2==0)?0:1;
    wait(0.5f);
    if (r==0) {
        red =0;
        green =1;
        t.reset();
    } else if(g==0) {
        green =0;
        red =1;
        t.reset();
    } else if(y==0) {
        red =0;
        green =0;
        t.reset();
    }
}
//init eth port
void init_eth(void const *args)
{
    //lcd.cls();
    eth.init(SERVER_IP, MASK, GATEWAY);
    int num=eth.connect();
    if(num!=-1) {
        transmitter= new Thread(transmit);
        receiver=new Thread(receive);
    }
}
//send information to other OBU device from array "counter1"
void transmit(void const *args)
{
    Endpoint server;
    UDPSocket sock;
    sock.init();
    server.set_address(CLIENT_IP, 6503);
    while(1) {
        stat();
        counter1[3]=30-t.read();
        sock.sendTo(server, counter1, sizeof(counter1));
        //wait(0.5);
    }
}
//receive information from other OBU device to array "counter"
void receive(void const *args)
{
    UDPSocket server;
    Endpoint client;
    server.bind(6500);
    int num;
    while(1) {
        num=stat();
        client.set_address(SERVER_IP,6500);
        n = server.receiveFrom(client, counter, sizeof(counter));
        if(num==0) {
            lcd.cls();
            lcd.locate(0,3);
            lcd.printf("red");
            lcd.locate(0,15);
            lcd.printf("vehicle ID: %c%c%d%d %c%c%c",counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);
        } else if(num==1) {
            lcd.cls();
            lcd.locate(0,3);
            lcd.printf("green");
            lcd.locate(0,15);
            lcd.printf("vehicle ID: %c%c%d%d %c%c%c",counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);
        } else if(num==2) {
            lcd.cls();
            lcd.locate(0,3);
            lcd.printf("yellow");
            lcd.locate(0,15);
            lcd.printf("vehicle ID: %c%c%d%d %c%c%c",counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);
        }
        /*lcd.locate(0,15);
        lcd.printf("vehicle ID:%c,%c,%d,%d,%c,%c,%c",counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);
        wait(1);*/
        wait(1.5);
    }
}
//check receive information 01 or 10
int check(int x)
{
    if(x==2) {
        if (info2[0]==0&&info2[1]==1) {
            return 1;
        } else if(info2[0]==1&&info2[1]==0) {
            return 2;
        }
    }
    return 0;
}
//receive infomation 
int getinfo(int x)
{
    int p=x;
    int num;
    tt.reset();
    while(1) {
        //check digitalin port
        if(sig1==1) {
            for (num=0; num<p; num++) {
                //timer 25s to wait data
                if(tt.read()<25) {
                    info1[num]=sig1;
                    info2[num]=sig2;
                    if (num>0) {
                        if(info1[num]==info1[num-1]) {
                            num = num -1;
                        }
                    }
                } else {
                    tt.stop();
                    return -1;
                }
            }
            return num -1;
        }
    }
}
//check infomation 01 or 10. 01 means send real time stat.10 means receive control information
void init(void)
{
    int num;
    num=check(2);
    if(num==1) {
        sentinfo();
    } else if(num==2) {
        analogout(3);
        num=getinfo(3);
        if(num!=-1) {
            if(info2[0]==0&&info2[1]==0&&info2[2]==1) {
                red = 0;
                green = 1;
            } else if(info2[0]==0&&info2[1]==1&&info2[2]==0) {
                red = 1;
                green = 0;
            } else if(info2[0]==1&&info2[1]==0&&info2[2]==0) {
                red = 0;
                green = 0;
            }
            t.reset();
            sentinfo();
        }
    }
}
//sendinfomation function 660mv,2s wait,color stat,2s wait,660mv,2s wait,vehicle information,2s wait,660mv,2s wait,0mv 
void sentinfo(void)
{
    int num;
    num=stat();
    analogout(4);
    wait(2);
    analogout(num);
    wait(2);
    analogout(4);
    wait(2);
    if (counter[0]==1) {
        num =0;
    } else if (counter[0]==2) {
        num =1;
    } else if(counter[0]==3) {
        num =2;
    }
    analogout(num);
    wait(2);
    analogout(4);
    wait(2);
    analogout(5);
}
//get traffic light infomation set array "counter1"
int stat(void)
{
    int num;
    if(red==0&&green!=0) {
        num =0;
        counter1[0]=0;
        counter1[1]=0;
        counter1[2]=1;
    } else if(green==0&&red!=0) {
        num =1;
        counter1[0]=0;
        counter1[1]=1;
        counter1[2]=0;
    } else if(red==0&&green==0) {
        num =2;
        counter1[0]=1;
        counter1[1]=0;
        counter1[2]=0;
    }
    return num;
}
//init analogout
void analogout(int x)
{
    switch(x) {
        case 0:
            aout = 0.3f;
            break;
        case 1:
            aout = 0.4f;
            break;
        case 2:
            aout = 0.5f;
            break;
        case 3:
            aout = 0.1f;
            break;
        case 4:
            aout = 0.2f;
            break;
        case 5:
            aout= 0.0f;
            break;
    }
}

//init 2 timers,clock function and init_eth function
void start(void)
{
    lcd.cls();
    lcd.locate(0,15);
    lcd.printf("welcome to V2I system!");
    red = 1;
    green = 1;
    t.start();
    tt.start();
    thp_eth= new Thread(init_eth);
    thp_clock= new Thread(clock);

}
//receive infomation from Smartmesh IP system
//send infomation to Smartmesh IP system
void connect_mash(void)
{
    while(1) {
        /*if(counter[0]!=0&&sig1==0) {
            ask();
            if(getinfo(2)!=-1) {
                init();
            }*/
        if(sig1==1) {
            if(getinfo(2)!=-1) {
                init();
            }
        }
    }
}
//clock function every 30s light color changed from red to green and yellow
void clock(void const *args)
{
    red=0;
    while(1) {
        show();
        if(t.read()>30) {
            t.reset();
            if(red ==0&&green!=0) {
                red =1;
                green =0;
            } else if(green ==0&&red!=0) {
                red =0;
                green =0;
            } else if(red==0&&red==0) {
                red =0;
                green =1;
            }
        }
    }
}
//main function
int main(void)
{
    start();
    connect_mash();
}
