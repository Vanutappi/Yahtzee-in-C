//////////////////////////////////
//                              //
//  Network Programming         //
//  Assignment 4                //
//  yahtzeeclient.h             //
//                              //
//  Kimmo Pietiläinen           //
//                              //
//  See readme file for info!   //
//                              //
//////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <ifaddrs.h>


// Defined max buffer length
#define BUFFLEN 512
#define TCPBUFFLEN 1028

// Defined listening port for the client
#define clientlistenport 33333
#define tcp_clientlistenport 45645

// My awesome global variables to make this work.
int Global_Connected = 0;
uint8_t Global_Dice_1_Locked = 0;
uint8_t Global_Dice_2_Locked = 0;
uint8_t Global_Dice_3_Locked = 0;
uint8_t Global_Dice_4_Locked = 0;
uint8_t Global_Dice_5_Locked = 0;

int client(char *,int,int,char *,int); // so we wont get implicit declaration warning

// EOF
