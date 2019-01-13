//////////////////////////////////
//                              //
//  Network Programming         //
//  Assignment 4                //
//  yahteeserver.h              //
//                              //
//  Kimmo Pietiläinen           //
//                              //
//  See readme file for info!   //
//				                      //
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

// Defined max buffer length
#define BUFFLEN 512
#define TCP_BUFFLEN 1028

// Defined listening port for the client
#define clientsendport 33333 // this is no longer valid
// more then 1 client from same computooor

// Global variables
int g_IS_CLIENT_CONNECTED = 0; 

int g_STATE_GAME_IS_ON = 0;

uint8_t g_GAME_ROUNDS = 0;

int g_START_GAME_SENT = 0;

int g_STATE_SCORE = 0;

int g_PLAYERS = 0;

// global struct for players and their info //
struct PLAYER {
	char *player_name;
	uint16_t player_id;
	uint8_t dice1;
	uint8_t dice2;
	uint8_t dice3;
	uint8_t dice4;
	uint8_t dice5;
	int is_available; // 0 means its free, 1 means its taken
  uint8_t ones;
  uint8_t twos;
  uint8_t threes;
  uint8_t fours;
  uint8_t fives;
  uint8_t sixes;
  uint8_t threeofakind;
  uint8_t fourofakind;
  uint8_t fullhouse;
  uint8_t lowstraight;
  uint8_t highstraight;
  uint8_t yahtzee;
  uint8_t chance;
  uint16_t uppertotal;
  uint16_t lowertotal;
  uint16_t totaltotal;
  uint8_t has_bonus_been_given; // 0 if not 1 if is
  uint8_t throws;
  struct sockaddr_in player_address;
  uint8_t round;
  uint8_t scored;
  uint8_t latestscore;
  uint8_t nickchosen; // 1 means client has chosen it via TCP	
} g_player[4];

// Funktion prototypes, so no implicit declaration warning
int how_many_players = 0;
int server(int,int,char *,int); 
int score(int,uint8_t,int);
uint8_t score_ofakind(int,int);
uint8_t score_simple(int,uint8_t);
uint8_t score_full_house(int);
uint8_t score_low_straigth(int);
uint8_t score_high_straigth(int);
uint8_t score_yahtzee(int);
uint8_t score_chance(int);
int initialize_players(int);
int check_client_address(int, struct sockaddr_in *);
int check_client_port(int, struct sockaddr_in *);

// eof
