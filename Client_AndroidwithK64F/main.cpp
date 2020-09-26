//client_showout
#include "mbed.h"
#include "EthernetInterface.h"
#include "C12832.h"
//The libary for LCD screen
C12832 lcd(D11, D13, D12, D7, D10);
DigitalOut red(LED_RED);
DigitalOut green(LED_GREEN);
 //blue.baud(9600);   
//The Pinout of TX&RX for UART services
Serial blue(PTB11, PTB10); //tx, rx

//The IP address for DSRC communication
static const char* SERVER_IP = "192.168.1.1";
static const char* CLIENT_IP = "192.168.1.2";
static const char* MASK = "255.255.255.0";
static const char* GATEWAY = "192.168.1.1";
EthernetInterface eth;
Thread *transmitter;
Thread *receiver;
Thread *init;
void transmit(void const *args); 
void receive(void const *args);
void init_eth(void const *args);
int n;
char counter[7]= {0};
char counter1[4]= {0};


//int currentLine;
 

    //The function for RED light 
    void data_transfer(){
    int i = 0 ; 
          
    blue.baud(9600);   //RATE BAUD FOR BLUETOOTH
         
    char command[8];
    command[0] = 8;    
    command[1] = 8;
    command[2] = 8;
    command[3] = 4;     //4 stands for red
    command[4] = counter1[3]/10;    
    command[5] = counter1[3]%10;
    command[6] = 9;
    command[7] = 9;
         
    for ( i = 0; i < 8; i++)  {

    blue.printf("%d",command[i]);
       
     }
            
     blue.printf("\n");
             
           //wait(0.5f);
    }
    
    //The function for Yellow light
    void data_transfer2(){
     
  
       int j = 0 ;   
    blue.baud(9600);        
         
    char command2[8];
    command2[0] = 8;    
    command2[1] = 8;
    command2[2] = 8;
    command2[3] = 5; //5 stands for yellow
    command2[4] = counter1[3]/10; 
    command2[5] = counter1[3]%10;
    command2[6] = 9;
    command2[7] = 9;
    

         
    for ( j = 0; j < 8; j++)  {

    blue.printf("%d",command2[j]);
       
     }
            
     blue.printf("\n");
             
           //wait(0.5f);
    }
   
   
   
   // The function for green light
    void data_transfer3(){
        int k = 0 ;     
    blue.baud(9600);        
         
    char command3[8];
    command3[0] = 8;    
    command3[1] = 8;
    command3[2] = 8;
    command3[3] = 6; //6 stands for green
        command3[4] = counter1[3]/10;   
        command3[5] = counter1[3]%10;
        command3[6] = 9;
        command3[7] = 9;
    

         
    for ( k = 0; k < 8; k++)  {

    blue.printf("%d",command3[k]);
       
     }
            
     blue.printf("\n");
             
           //wait(0.5f);
    }
    



//init eth point
void init_eth(void const *args)
{
    eth.init(CLIENT_IP, MASK, GATEWAY);
    int num=eth.connect();
    if(num!=-1) {
        transmitter= new Thread(transmit); //Thread ????
        receiver=new Thread(receive);
    }
}
//transmit function send information 
void transmit(void const *args)
{
    Endpoint server;
    UDPSocket sock;
    sock.init();
    server.set_address(SERVER_IP, 6503);
    while(1) {
        sock.sendTo(server, counter, sizeof(counter));
    }
}

//void printNext()    {
//     blue.baud(9600);   
//    
//    switch(currentLine) {
//        case 0:
//            blue.printf("777Slow down your speed66\n");
//          //  blue.printf("\n");
//            break;
//        case 1:
//            blue.printf("777Road work ahead66\n");
//         //   blue.printf("\n");
//            break;
//        case 2:
//            blue.printf("777Road accident ahead66\n");
//         //   blue.printf("\n");
//            break;
//        case 3:
//            blue.printf("777Traffic jam ahead66\n");
//          //    blue.printf("\n");
//            break;
//        case 4:
//            blue.printf("777Slow - Fog Use Lights66\n");
//        //      blue.printf("\n");
//            break;
//        case 5:
//            blue.printf("777Don't drink and Drive police enforced66\n");
//          //    blue.printf("\n");
//            break;
//        case 6:
//            blue.printf("777Please Drive safely66\n");
//         //     blue.printf("\n");
//            break;
//        case 7:
//            blue.printf("777Morning sir, have a good day66\n");
//         //     blue.printf("\n");
//            break;
//    }
//    
//    if (currentLine == 7)   {
//        currentLine = 0;
//    } else  {
//        currentLine++;
//    }
//    
//}





//receive function receive to array "counter1"
void receive(void const *args)
{
    
    UDPSocket server;
    Endpoint client;
    server.bind(6500);
    // blue.baud(9600);   
   // int m =1;
    Timer clock;
    //Timer timer;

     //currentLine = 0;
     //timer.start();
    
   clock.start();
   
    while(1) {
        client.set_address(CLIENT_IP,6500);
        n = server.receiveFrom(client, counter1, sizeof(counter1));
        if (counter1[0]==0&&counter1[1]==0&&counter1[2]==1) {
            red =0;
            green=1;
        data_transfer();  
        } else if (counter1[0]==0&&counter1[1]==1&&counter1[2]==0) {
            green =0;
            red =1;
            data_transfer3();
        } else if(counter1[0]==1&&counter1[1]==0&&counter1[2]==0) {
            red =0;
            green =0;
            data_transfer2();
        }
      //  wait(0.5);
        clock.reset();
       //if ((int)timer.read() % 30 < 1) {
//           printNext();
//       }
//      
//      wait(1); 
     
    }
}
//random 0-9 number
char random_num(void)
{
    return rand()%9;
}
//random 0-25 number
char random_1_ABC(void)
{
    return 1+rand()%25;
}
//num 0-25 <=>'A'-'Z'
char random_2_ABC(char x)
{
    if(x==1) {
        return 'A';
    } else if(x==2) {
        return 'B';
    } else if(x==3) {
        return 'C';
    } else if(x==4) {
        return 'D';
    } else if(x==5) {
        return 'E';
    } else if(x==6) {
        return 'F';
    } else if(x==7) {
        return 'G';
    } else if(x==8) {
        return 'H';
    } else if(x==9) {
        return 'I';
    } else if(x==10) {
        return 'J';
    } else if(x==11) {
        return 'K';
    } else if(x==12) {
        return 'L';
    } else if(x==13) {
        return 'M';
    } else if(x==14) {
        return 'N';
    } else if(x==15) {
        return 'O';
    } else if(x==16) {
        return 'P';
    } else if(x==17) {
        return 'Q';
    } else if(x==18) {
        return 'R';
    } else if(x==19) {
        return 'S';
    } else if(x==20) {
        return 'T';
    } else if(x==21) {
        return 'U';
    } else if(x==22) {
        return 'V';
    } else if(x==23) {
        return 'W';
    } else if(x==24) {
        return 'X';
    } else if(x==25) {
        return 'Y';
    } else {
        return 'Z';
    }
}
//first 2 vehicle number
char first_2_word(void)
{
    while(1) {
        char num =random_2_ABC(random_1_ABC());
        if(num!='I'&&num!='Q'&&num!='Z') {
            return num;
        }
    }
}
//last 3 vehicle number
char last_3_word(void)
{
    while(1) {
        char num =random_2_ABC(random_1_ABC());
        if(num!='I'&&num!='Q') {
            return num;
        }
    }
}
//send all vehicle number to array "counter"
void setcounter1(void)
{
    counter[0]=first_2_word();
    counter[1]=first_2_word();
    counter[2]=random_num();
    counter[3]=random_num();
    counter[4]=last_3_word();
    counter[5]=last_3_word();
    counter[6]=last_3_word();
    /*lcd.cls();
    lcd.locate(0,3);
    lcd.printf("%c%c%d%d%c%c%c",counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);*/
    wait(3);
}



int main()
{
    red =1;
    green =1;
  
    init=new Thread(init_eth); 
    while (true) {
        setcounter1();
           
  }  
}
