//////////////////////////////////
//                              //
//  Network Programming         //
//  Assignment 4                //
//  yahteeserver.c              //
//                              //
//  Kimmo Pietiläinen           //
//                              //
//  See readme file for info!   //
//				                      //
//////////////////////////////////

// This is bugfix version

#include "yahtzeeserver.h"

// Main function
int main(int argc, char *argv[])
{
  // help variables for argument sorting
  int e=1;
  int helper1=0,helper2=0,helper3=0,helper4=0;
  int ok1=0,ok2=0,ok3=0,ok4=0;
  // Checking if we got the right amount of arguments
  if (argc == 9)
  {
    for (e=1;e<=7;e+=2){ // so the argument order doesnt matter to user
      if (strncmp(argv[e], "-up", 3) == 0){
        helper1=e+1;
        ok1=1;
      }
      if (strncmp(argv[e], "-tp", 3) == 0){
        helper2=e+1;
        ok2=1;
      }
      if (strncmp(argv[e], "-m\0", 3) == 0){
        helper3=e+1;
        ok3=1;
      }
      if (strncmp(argv[e], "-mp", 3) == 0){
        helper4=e+1;
        ok4=1;
      }
    }
    if (ok1 == 1 && ok2 == 1 && ok3 == 1){
      if (server(atoi(argv[helper1]),atoi(argv[helper2]),argv[helper3],atoi(argv[helper4])) == 0){
        printf("Server offline.\n"); // Starting the server.
      }
      else
      {
        printf("Server error.\n");
        return -1;
      }
    }
    else {
      printf("Argument error.\nReadme file for more information:\n\
      Server: %s -up <UDP listening port> -tp <TCP listening port> -m <multicast address> -mp <multicast port number>\n",argv[0]);
      return -1;
    }
  }
  // Argument errors
  else
  {
    printf("Invalid amount of arguments.\nReadme file for more information:\n\
    Server: %s -up <UDP listening port> -tp <TCP listening port> -m <multicast address> -mp <multicast port number>\n",argv[0]);
    return -1;
  }
  return 0;
}

// Server function
int server(int port, int tcp_port, char *mcip, int mcport)
{
  // Socket descriptors
  int socketfd; 
  int mcsocketfd;
  int tcp_socketfd;
  int new_socketfd;
  
  // tcp byte helpers
  int nbytes;
  int byte_count;
  
	initialize_players(4); // number of initialized g_players
  
  int dgramlen = 0; // datagram length

  char recvbuffer[BUFFLEN]; // Buffer to which recieved messages are collected

  srand((unsigned)time(NULL)); // so that rand() would be abit more random

  uint8_t message_id; // message id gathered from all incoming messages, always the 1st byte.
  
  int i,e,j; // help variables for for-loops

  // buffer and message id for GAME_FULL message.
  char fullbuffer[40];
  uint8_t fullint = 21;
  uint8_t fullerrorint = 1;

  // buffer and message id for INVALID_POSITION message.
  char invbuffer[25];
  uint8_t invint = 21;
  uint8_t inverrorint = 3;

  // buffer and message id for GAME_RUNNING message.
  char runningbuffer[53];
  uint8_t runningint = 21;
  uint8_t runningerrorint = 2;

  // buffer and message if for ACCEPTED message.
  char acceptedbuffer[11];
  uint8_t acceptedint = 2;

  // buffer and message id for DICES message.
  char dicesbuffer[6];
  uint8_t dicesint = 4;

  // buffer and message id for POSSIBLE message.
  char possiblebuffer[15];
  uint8_t possibleint = 10;
  
  // buffer and message id for POSITION_SET message.
  char positionsetbuffer[4];
  uint8_t positionsetint = 12;  
  
  // buffer and message id for QUIT message.
  // char quitbuffer[1];
  // uint8_t quitint = 20;
  
  // buffer and message id for STOP_GAME message.
  char stopbuffer[1];
  uint8_t stopint = 32;
  
  // buffer and message id for PLAYER_JOIN mc.
  char mc_joinbuffer[11];
  uint8_t mc_joinint = 34;
  
  // buffer and message id for PLAYER_LEFT mc.
  char leftbuffer[11];
  uint8_t leftint = 33;
  
  // buffer and message id for START_GAME mc.
  char startbuffer[1];
  uint8_t startint = 30;

  // buffer and message id for NEXT_ROUND mc.
  char nextbuffer[1];
  uint8_t nextint = 31;

  char message[6]; // for getting /quit command from STDIN
  
  char tcpbuffer[TCP_BUFFLEN]; // incoming tcp buffer
  char chatmessagebuffer[BUFFLEN]; // tcp chat message buffer
  char tcpmessage[BUFFLEN]; // buffer for tcp messages that are sent
  char tcppeekbuffer[4]; // this is used to see the next messages length in tcp socket

  char quitmessage[37] = "Game is full. Please try again later."; // quit message sent to client.

  char runningmessage[50] = "Game is currently running. Please try again later."; // game_running error message

  char poserrormessage[22] = "Position is not valid."; // invalid_position error message
  
  uint8_t scoresheetsint = 35; // message id for sending scoresheets mc

  // This is what is actually sent to client. It also has the totals.
  char scoresheets[86]; // 85 is the max lenght if we got 4 players, 1 more for \0

  int loop = 1; // for while loop

  int yes =1; // for setting reusable socket

  int id = 0; // id help variable so server knows which player its handling
  
  uint8_t pid; // player id helper

  fd_set RecFd,MasterFd; // file descriptorsets for select()

  int maxfd; // highest file descriptor

  struct sockaddr_in remote_address; // client addresses (TCP)
  //char remoteIP[INET6_ADDRSTRLEN];

  struct in_addr localInterface;
  struct sockaddr_in multicast_group_address, multicast_local_address, client_address, server_address, tcp_server_address; // structs for addresses
  struct ip_mreq group; // for multicast group management
  socklen_t tcp_server_addrlen = 0,server_addrlen = 0, multicast_group_addrlen = 0, multicast_local_addrlen = 0, addrlen = 0, client_addrlen = 0; // address lengths
  
  printf("Yahtzee server.\n");

  // Checking if port numbers are ok.
  if (port > 1024 && port < 65000 && mcport > 1024 && mcport < 65000 && mcport != port && port != tcp_port)
  {
    // UDP Socket creation
    if ((socketfd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
      return -1;
    }

	  // Initializing server/client variables and server/client address struct
	  server_addrlen=sizeof(server_address);
	  memset(&server_address,0,server_addrlen); // Clearing server address info
	  server_address.sin_family = AF_INET; // Initializing network family
	  server_address.sin_addr.s_addr = INADDR_ANY; // Using any address for server
	  server_address.sin_port=htons(port); // Assigning port in network byte order
	  
	  memset(&client_address,0,client_addrlen);
	  client_address.sin_family = AF_INET;
	  //client_address.sin_port=htons(clientsendport); // assigning a port for the client to reciece from
	  client_address.sin_addr.s_addr = INADDR_ANY;
	  // this is actually DEFINED in code for both client and server, all clients must be revieving messages from server at this port
	  // servers listening port is ofcourse given as an argument when client/server is started.

	  // Binding address and port
	  if (bind(socketfd,(struct sockaddr *)&server_address,server_addrlen) < 0)
	  {
	    printf("Bind failed.\n");
			close(socketfd);
			//close(mcsocketfd);
	    return -1;
	  }
	  else{
	    printf("UDP bind success.\n");
	  }
    
    // TCP socket creation  
    if ((tcp_socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
      return -1;
    }
    
    setsockopt(tcp_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // setting it as reuseable to avoid errors   

    // Initializing server/client variables and server/client address struct
    tcp_server_addrlen=sizeof(tcp_server_address);
    memset(&tcp_server_address,0,tcp_server_addrlen); // Clearing server address info
    tcp_server_address.sin_family = AF_INET; // Initializing network family
    tcp_server_address.sin_addr.s_addr = INADDR_ANY; // Using any address for server
    tcp_server_address.sin_port=htons(tcp_port); // Assigning port in network byte order

    // Binding address and port
    if (bind(tcp_socketfd,(struct sockaddr *)&tcp_server_address,tcp_server_addrlen) < 0)
    {
      printf("Bind failed.\n");
			close(socketfd);
		  close(tcp_socketfd);
      return -1;
    }
	  else{
	    printf("TCP bind success.\n");
	  }    
    
		////////// MULTICAST 

		// Socket creation
		if ((mcsocketfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
			return -1; // checks if socket creation goes ok
		}    

		// Enable SO_REUSEADDR to allow multiple instances of this 
		// application to receive copies of the multicast datagrams. 

		int reuse = 1;

		if(setsockopt(mcsocketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
		{
			perror("Setting SO_REUSEADDR error");
			close(socketfd);
			close(mcsocketfd);
			exit(1);
		}
		else 
		{
			//printf("Setting SO_REUSEADDR...OK.\n");
		}

		// Bind to the proper port number with the IP address 
		// specified as INADDR_ANY. 

		multicast_group_addrlen=sizeof(multicast_group_address);
		multicast_local_addrlen=sizeof(multicast_local_address);
		
		memset((char *) &multicast_local_address, 0, sizeof(multicast_local_address));
		multicast_local_address.sin_family = AF_INET;
		multicast_local_address.sin_port = htons(mcport);
		multicast_local_address.sin_addr.s_addr = INADDR_ANY;

		memset((char *) &multicast_group_address, 0, sizeof(multicast_group_address));
		multicast_group_address.sin_family = AF_INET;
		multicast_group_address.sin_addr.s_addr = INADDR_ANY;
		multicast_group_address.sin_port = htons(mcport);	

		// no need to bind this as we dont listen? ill bind it just incase
    // Bind mc socket (many clients can bind same port now on same computor)
		if(bind(mcsocketfd, (struct sockaddr*)&multicast_local_address, sizeof(multicast_local_address)))
		{
			perror("Binding datagram socket error");
			close(socketfd);
		  close(mcsocketfd);
			exit(1);
		}
	  else{
	    printf("MC bind success.\n");
	  }

		if(inet_pton(AF_INET,mcip,&(multicast_group_address.sin_addr)) <= 0) { // assign ip to address struct 
			perror("inet_pton()");
			return -1;
		}

		group.imr_multiaddr.s_addr = inet_addr(mcip); //multicast_address.sin_addr.s_addr;
		group.imr_interface.s_addr = INADDR_ANY;  //inet_addr("10.0.2.15"); //client_address.sin_addr.s_addr;

		addrlen = sizeof(group.imr_interface.s_addr);

    // no need to join as we dont care what happens, only tell em stuff
    
    // Adding to time to live (1 ---> 7)
    u_char ttl = 7;
	
		if(setsockopt(mcsocketfd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) < 0)
		{
			perror("Setting IP_MULTICAST_TTL error");
			close(socketfd);
			close(mcsocketfd);
			exit(1);
		}
		else
		{
			//printf("Increased TTL to 7...OK.\n");
		}
    
    // Setting local interface for outbound multicast datagrams. 
		localInterface.s_addr = INADDR_ANY;

		if(setsockopt(mcsocketfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
		{
			perror("Setting local interface error");
			close(socketfd);
		  close(mcsocketfd);
			exit(1);
		}
		else {
			//printf("Setting the local interface...OK\n");
		}
     
    // resetting filedestriptorsets
    FD_ZERO(&RecFd);
    FD_ZERO(&MasterFd);    
    
    // getting maximum filedescriptornumber 
    if(socketfd > tcp_socketfd){
      maxfd = socketfd;
    }
    else{
      maxfd = tcp_socketfd;
    }

    // Setting up filedescriptorsets with socket, mcsocket and STDIN
    FD_SET(socketfd, &MasterFd);
    FD_SET(tcp_socketfd, &MasterFd);
    FD_SET(STDIN_FILENO, &MasterFd);     
     
    // TCP listen for connections
    if (listen(tcp_socketfd, 10) == -1) {
        perror("listen: ");
        exit(1);
    }     
        
    printf("Server is ready for clients.\n");   
      
    // Main Server loop
    while(loop == 1){
    
			// copying MasterFd, as RecFd changes
			RecFd = MasterFd;
			 
			// Select()
			if(select(maxfd+1, &RecFd, NULL, NULL, NULL) == -1)
			{
			  perror("Server select() error: ");
			  exit(1);
			}
			//printf("Server-select() pinged\n");
			 
			// run through the existing connections looking for data to be read
			// listening TCP, UDP, STDIN, new TCPs
			for(i = 0; i <= maxfd; i++)
			{
			  if(FD_ISSET(i, &RecFd)){ // Something triggered
			    //printf("We got one: %d\n",i);
					if(i == tcp_socketfd) {
					  // handles new connections
					  addrlen = sizeof(client_addrlen);
						if((new_socketfd = accept(tcp_socketfd, (struct sockaddr *)&remote_address, &addrlen)) == -1){
							perror("Server accept() error:");
						}
						else{
							//printf("Server accept() is ok\n");
				 
							FD_SET(new_socketfd, &MasterFd); // add to MasterFd set 
							if(new_socketfd > maxfd){ // keeping track of the maximum fdnumber
								maxfd = new_socketfd;
							}
					    printf("Server: New connection from on socket %d\n", new_socketfd);
					  }
					}
					
					// Keyboard triggered
			    else if (i == STDIN_FILENO){
			    
						printf("STDIN TRIGGERED\n");
						fgets (message , 6 , stdin);
						// Server quit option
						if (strncmp(message, "/quit", 5)== 0) {
						  printf("Server shutting down.\n");
						  // need to send quit to client
						  stopbuffer[0] = stopint;
						  // next send the data to clients.
						  
						  // this feels kinda not useful anymore as clients will detect TCP connection break and shut down
						  dgramlen = sendto(mcsocketfd,stopbuffer,1,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);
						  	
						  close(socketfd);
						  close(mcsocketfd);
						  close(tcp_socketfd);
						  exit(1);
						}
					}		    
			    			    
					/////// UDP TRIGGERED ///////////
					else if(i == socketfd) { // this checks if anything happens in socket

						printf("UDP TRIGGERED\n");
						addrlen = sizeof(client_address);
						// Waiting for a datagram from client
						dgramlen = recvfrom(socketfd,recvbuffer,BUFFLEN,0,(struct sockaddr*)&client_address,&addrlen);

						message_id = recvbuffer[0]; // getting the message id from 1st byte of message.

						printf("Got message: %d\n",message_id);
						printf("%s\n", (char *)inet_ntoa(client_address.sin_addr)); 

		
		        // For when we get a JOIN message from client 
						if (message_id == 1) {
		 
						  // lets store the clients IP so we can know if same player is connecting.
						  if (g_IS_CLIENT_CONNECTED <= 3) {
						    for (e=0;e<=3;e++){
						      if (g_player[e].is_available == 0){ // going through all players and checking if any available
						  		  g_player[e].player_address = client_address;
						  		  g_player[e].is_available = 1;
						  		  g_IS_CLIENT_CONNECTED++;
						  		  pid = e+1;
						  		  id=e;
						  		  g_player[e].player_id = pid;
						  		  e = 34;
						        g_PLAYERS++;
						  		  //printf("Hope this happens only once per JOIN! %d\n",id);
						  		}
						  	}
						  	           	
								// Lets form ACCEPTED message to be sent to client

								// 16bit integer for player_id so I will split it into 2 parts of 8bit integers to fit in into 1+1 byte of char space.
								// uint16_t player_id = 1;
								// uint8_t player_id_part_1 = (uint8_t)player_id >> 8; // bitshifting
								// uint8_t player_id_part_2 = (uint8_t)player_id;
						
								// This will only work for assignment 1, I know that, but this will work fine here as there arent more then 1 players.
								acceptedbuffer[0] = acceptedint;

								*(uint16_t*)&acceptedbuffer[1] = htons(g_player[id].player_id); // this now does the thing i used to do with bit shifts

								//acceptedbuffer[2] = player_id_part_2;

								acceptedbuffer[3] = g_player[id].player_name[0];
								acceptedbuffer[4] = g_player[id].player_name[1];
								acceptedbuffer[5] = g_player[id].player_name[2];
								acceptedbuffer[6] = g_player[id].player_name[3];
								acceptedbuffer[7] = g_player[id].player_name[4];
								acceptedbuffer[8] = g_player[id].player_name[5];
								acceptedbuffer[9] = g_player[id].player_name[6];

								acceptedbuffer[10] = '\0';

								printf("Sending UDP message: %d\n",acceptedint);
								
								client_addrlen = sizeof(g_player[id].player_address);
								//client_addrlen = sizeof(client_address);
								// Sending data to client.
								dgramlen = sendto(socketfd,acceptedbuffer,11,0,(struct sockaddr *)&g_player[id].player_address,client_addrlen);
								//sleep(2);
								//dgramlen = sendto(socketfd,acceptedbuffer,11,0,(struct sockaddr *)&client_address,client_addrlen);
								printf("Tried to send ACCEPTED to client.\n");
								
						    // Lets also send a multicast that a new player has joined
								
								mc_joinbuffer[0] = mc_joinint;
								
						  	*(uint16_t*)&mc_joinbuffer[1] = htons(g_player[id].player_id); // takes 2 spots
						  	
						  	mc_joinbuffer[3] = g_player[id].player_name[0];
								mc_joinbuffer[4] = g_player[id].player_name[1];
								mc_joinbuffer[5] = g_player[id].player_name[2];
								mc_joinbuffer[6] = g_player[id].player_name[3];
								mc_joinbuffer[7] = g_player[id].player_name[4];
								mc_joinbuffer[8] = g_player[id].player_name[5];
								mc_joinbuffer[9] = g_player[id].player_name[6];
								
								mc_joinbuffer[10] = '\0';
								
								printf("Sending MC message: %d\n",mc_joinint);
								
								// Sending data to MC.
								dgramlen = sendto(mcsocketfd,mc_joinbuffer,11,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);
						  	
						  	//printf("%d %s\n",dgramlen,mc_joinbuffer);
						  	
						  	if (dgramlen <0){
						  	  printf("PLAYER_JOIN mc error.\n");
						  	}
						  }
						  else{
						    printf("Someone else tried to connect.\n");
							  // We need to send a GAME_FULL message to the poor client who tried to join.
							  fullbuffer[0] = fullint;
							  fullbuffer[1] = fullerrorint;

							  //adding error message
							  for (i=0;i<=37;i++){
							    strcpy(&fullbuffer[i+2],&quitmessage[i]);
							  }

							  // adding end character
							  fullbuffer[39] = '\0';
							
							  printf("Sending message: %d\n",fullint);
							  // sending GAME_FULL message to client
							  dgramlen = sendto(socketfd,fullbuffer,40,0,(struct sockaddr *)&client_address,addrlen);
							  // also changing the message_id so it wont mess up my server
							  message_id = 25;
							  recvbuffer[0] = 25;  
							}    	          	    	
						}
					 
		        // Connection handling
            if (g_IS_CLIENT_CONNECTED == 4){
							g_STATE_GAME_IS_ON = 1;
						
							// must also send START_GAME to clients

						  if (g_START_GAME_SENT == 0){
								// program was too fast, CHEERS didnt have time to come.
								sleep(2);
								startbuffer[0] = startint;
								dgramlen = sendto(mcsocketfd,startbuffer,1,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);
								g_START_GAME_SENT = 1;
							}
						} 
						
						if (g_STATE_GAME_IS_ON == 1) { // We dont want any new players now                      

						  //printf("%s %s\n",client_ip, (char *)inet_ntoa(client_address.sin_addr)); 
						  
						  // We need to check who just sent a udp packet 

						  //printf("Player1: %u\n",g_player[0].is_available);      
						  //printf("Player2: %u\n",g_player[0].is_available);   
						  //printf("Player3: %u\n",g_player[0].is_available);   
						  //printf("Player4: %u\n",g_player[0].is_available); 

						  int supadupahelp = 0;     
						  
						  // player1
						  if (g_player[0].is_available == 1 && check_client_address(0, &client_address) == 0 && check_client_port(0, &client_address) == 0){
						    id = 0;
						    supadupahelp = 1;
						    //printf("We got ID1\n");
						  }

              // player2
						  if (g_player[1].is_available == 1 && check_client_address(1, &client_address) == 0 && check_client_port(1, &client_address) == 0 && supadupahelp != 1){
						    id = 1;
						    supadupahelp = 1;
						    //printf("We got ID2\n");
						  }

              // player3
						  if (g_player[2].is_available == 1 && check_client_address(2, &client_address) == 0 && check_client_port(2, &client_address) == 0 && supadupahelp != 1){
						    id = 2;
						    supadupahelp = 1;
						    //printf("We got ID3\n");
						  }

              // player4
						  if (g_player[3].is_available == 1 && check_client_address(3, &client_address) == 0 && check_client_port(3, &client_address) == 0 && supadupahelp != 1){
						    id = 3;
						    supadupahelp = 1;
						    //printf("We got ID4\n");
						  }

						  // none of the above triggered, means its someone we dont want!
						  if (supadupahelp == 0){ 
						  	printf("Someone else tried to connect.\n");

							  // We need to send a GAME_RUNNING message to the poor client who tried to join.
							  runningbuffer[0] = runningint;
							  runningbuffer[1] = runningerrorint;

							  //adding error message
							  for (i=0;i<=50;i++){
							    strcpy(&runningbuffer[i+2],&runningmessage[i]);
							  }

							  // adding end character
							  runningbuffer[52] = '\0';
							
							  printf("Sending message: %d\n",runningint);
							  // sending GAME_running message to client
							  dgramlen = sendto(socketfd,runningbuffer,40,0,(struct sockaddr *)&client_address,addrlen);
							  message_id = 25;
							  recvbuffer[0] = 25;
						  }
						}

		
		        // For when we get a ROLL_DICE message from client 
						if (message_id == 3) {

						  if (g_player[id].throws < 3){

								//printf("Got DICES message from client.\n");
						 
								// Lets roll the dices using random() function.
								g_player[id].dice1 = rand() % 6 + 1;
								g_player[id].dice2 = rand() % 6 + 1;
								g_player[id].dice3 = rand() % 6 + 1;
								g_player[id].dice4 = rand() % 6 + 1;
								g_player[id].dice5 = rand() % 6 + 1;
								// and then send them to client with DICES message.
								dicesbuffer[0] = dicesint;
								dicesbuffer[1] = g_player[id].dice1;
								dicesbuffer[2] = g_player[id].dice2;
								dicesbuffer[3] = g_player[id].dice3;
								dicesbuffer[4] = g_player[id].dice4;
								dicesbuffer[5] = g_player[id].dice5;

								// sleep(2);

								printf("Sending message: %d\n",dicesint); 
								// next send the sata to client.
								dgramlen = sendto(socketfd,dicesbuffer,6,0,(struct sockaddr *)&client_address,addrlen);
								
								message_id = 0;
						    
						    g_player[id].throws++;

						  }
						  else{
						    if (g_player[0].scored == 1 && g_player[1].scored == 1 && g_player[2].scored == 1 && g_player[3].scored == 1){
						      g_STATE_SCORE = 1;
						    }
						  }
						  //printf("Rolled em dices: %d %d %d %d %d , trying to send them to client %d ... %d.\n",g_player[id].dice1,dice2,g_player[id].dice3,g_player[id].dice4,g_player[id].dice5,dicesint,dgramlen);
						}

		
		        // For when we get a LOCK message from client 
						if (message_id == 5) {

						  // First we need to check howmany rolls has the player already had.
						  if (g_player[id].throws == 3){ // has already rolled 3 times

						    //score(id);
						    
						    // need to send possible msg to client
						    // so lets change message_id to 6 so it will be the same as getting a /score msg
						    
						    message_id = 6;
						    

						  }
						  else { // has still rolls left
						       
						    //printf("Got these for locking: %d %d %d %d %d\n",recvbuffer[1],recvbuffer[2],recvbuffer[3],recvbuffer[4],recvbuffer[5]);
						  
						    // Lets roll the unlocked dices using random() function.
						    if (recvbuffer[1] == 0){
						      g_player[id].dice1 = rand() % 6 + 1;
						    }
						    if (recvbuffer[2] == 0){
						      g_player[id].dice2 = rand() % 6 + 1;
						    }
						    if (recvbuffer[3] == 0){
						      g_player[id].dice3 = rand() % 6 + 1;
						    }
						    if (recvbuffer[4] == 0){
						      g_player[id].dice4 = rand() % 6 + 1;
						    }
						    if (recvbuffer[5] == 0){
						      g_player[id].dice5 = rand() % 6 + 1;
						    }
		 
						    // and then send them to client with DICES message.
						    dicesbuffer[0] = dicesint;
						    dicesbuffer[1] = g_player[id].dice1;
						    dicesbuffer[2] = g_player[id].dice2;
						    dicesbuffer[3] = g_player[id].dice3;
						    dicesbuffer[4] = g_player[id].dice4;
						    dicesbuffer[5] = g_player[id].dice5;
						   
						    printf("Sending message: %d\n",dicesint);
						    // next send the data to client.
						    dgramlen = sendto(socketfd,dicesbuffer,6,0,(struct sockaddr *)&client_address,addrlen);
						    //have to remember to add 1 to roll_count
						    //roll_count++;

						    g_player[id].throws++;
						  }
						  if (g_player[0].scored == 1 && g_player[1].scored == 1 && g_player[2].scored == 1 && g_player[3].scored == 1){
						    g_STATE_SCORE = 1;
						  }
						}

		        // For when we get an SCORE message from client 
						if (message_id == 6) {

						  // Before anything lets reset roll_count
						  // roll_count = 1;
						  
						  // score(id); // will be used once set_score is ok
						  
						  // Need to send possible positions to client so it can choose
						  
						  possiblebuffer[0] = possibleint;
						  possiblebuffer[1] = g_GAME_ROUNDS+1;
						  g_player[id].round = g_GAME_ROUNDS+1;
						  
						  uint8_t apu1 = 1;
						  uint8_t apu2 = 2;
						  uint8_t apu3 = 3;
						  uint8_t apu4 = 4;
						  uint8_t apu5 = 5;
						  uint8_t apu6 = 6;
						  uint8_t apu7 = 7;
						  uint8_t apu8 = 8;
						  uint8_t apu9 = 9;
						  uint8_t apu10 = 10;
						  uint8_t apu11 = 11;
						  uint8_t apu12 = 12;
						  uint8_t apu13 = 13;
						  
						  uint8_t spot=2;
						  
						  // going thru and marking all positions that are available for scoring
						  if (g_player[id].ones == 'a'){
						    possiblebuffer[spot] = apu1;
						    spot++;
						  }
							if (g_player[id].twos == 'a'){
						    possiblebuffer[spot] = apu2;
						    spot++;						    
						  }
							if (g_player[id].threes == 'a'){
						    possiblebuffer[spot] = apu3;
						    spot++;						    
						  }
							if (g_player[id].fours == 'a'){
						    possiblebuffer[spot] = apu4;
						    spot++;						    
						  }
							if (g_player[id].fives == 'a'){
						    possiblebuffer[spot] = apu5;
						    spot++;						    
						  }
							if (g_player[id].sixes == 'a'){
						    possiblebuffer[spot] = apu6;
						    spot++;						    
						  }
							if (g_player[id].threeofakind == 'a'){
						    possiblebuffer[spot] = apu7;
						    spot++;						    
						  }
						 	if (g_player[id].fourofakind == 'a'){
						    possiblebuffer[spot] = apu8;
						    spot++;						    
						  }
							if (g_player[id].fullhouse == 'a'){
						    possiblebuffer[spot] = apu9;
						    spot++;						    
						  }
		          if (g_player[id].lowstraight == 'a'){
						    possiblebuffer[spot] = apu10;
						    spot++;						    
						  }
	          	if (g_player[id].highstraight == 'a'){
						    possiblebuffer[spot] = apu11;
						    spot++;						    
						  }
		          if (g_player[id].yahtzee == 'a'){
						    possiblebuffer[spot] = apu12;
						    spot++;						    
						  }
		          if (g_player[id].chance == 'a'){
						    possiblebuffer[spot] = apu13;
						    spot++;						    
						  }
						  
						  // adding \0 to the end
						  possiblebuffer[spot] = '\0';
						  spot++;
						  
						  printf("Sending message: %d\n",possibleint);
						  // next send the data to client.
						  dgramlen = sendto(socketfd,possiblebuffer,spot,0,(struct sockaddr *)&g_player[id].player_address,addrlen);  
						  
						  g_player[id].throws = 3;
						  
						  //g_player[id].scored = 1;
						  //printf("Player%d has scored! Sending possible message\n",id);

              // checks if its time to go to score state
						  if (g_player[0].scored == 1 && g_player[1].scored == 1 && g_player[2].scored == 1 && g_player[3].scored == 1){
						    g_STATE_SCORE = 1;
						  }
						}

		        // For when we get an SET_SCORE message from client 
						if (message_id == 11) {
              
              uint8_t field_id = recvbuffer[2];
              int sendposerror = 0;
              
              // We have to check if that position has been scored into earlier
              
              if (g_player[id].ones != 'a' && field_id == 1){
                sendposerror = 1;
              }
              else if (g_player[id].twos != 'a' && field_id == 2){
                sendposerror = 1;
              }
              else if (g_player[id].threes != 'a' && field_id == 3){
                sendposerror = 1;
              }
              else if (g_player[id].fours != 'a' && field_id == 4){
                sendposerror = 1;
              }
              else if (g_player[id].fives != 'a' && field_id == 5){
                sendposerror = 1;
              }
              else if (g_player[id].sixes != 'a' && field_id == 6){
                sendposerror = 1;
              }
              else if (g_player[id].threeofakind != 'a' && field_id == 7){
                sendposerror = 1;
              }
              else if (g_player[id].fourofakind != 'a' && field_id == 8){
                sendposerror = 1;
              }
              else if (g_player[id].fullhouse != 'a' && field_id == 9){
                sendposerror = 1;
              }
              else if (g_player[id].lowstraight != 'a' && field_id == 10){
                sendposerror = 1;
              }
              else if (g_player[id].highstraight != 'a' && field_id == 11){
                sendposerror = 1;
              }
              else if (g_player[id].yahtzee != 'a' && field_id == 12){
                sendposerror = 1;
              }
              else if (g_player[id].chance != 'a' && field_id == 13){
                sendposerror = 1;
              }
              else { // its free jou
                sendposerror = 0;
              }
              
              if (sendposerror == 1){
                            
                // errortime
							  // We need to send a INVALID_POSITION message to client
							  invbuffer[0] = invint;
							  invbuffer[1] = inverrorint;

							  //adding error message
							  for (i=0;i<=22;i++){
							    strcpy(&invbuffer[i+2],&poserrormessage[i]);
							  }

							  // adding end character
							  invbuffer[24] = '\0';
							
							  printf("Sending message: %d\n",invint);
							  // sending INVALID_POSITION message to client
							  dgramlen = sendto(socketfd,invbuffer,25,0,(struct sockaddr *)&client_address,addrlen);
							  // also changing the message_id so it wont mess up my server
							  message_id = 25;                
                
              }
              else{ // no errors so the position was OK
              
		            int do_it_right = 1;
		            int do_it_wrong = 0;
		            int wherewescored = 0;
		            
		            //printf("Field id: %d Round: %d PRound: %d\n",recvbuffer[2],recvbuffer[1],g_player[id].round);
		            
		            if (recvbuffer[1] == g_player[id].round){ // means that the rounds client sent matches the rounds server sent the POSSIBLE msg
		              wherewescored = score(id,field_id,do_it_right);
		            }
		            else{ // if its invalid, just score 1st possible field
		              wherewescored = score(id,field_id,do_it_wrong);
		            }  
		            
		            g_player[id].scored = 1;
		            
		            // Need to send POSITION_SET message to client
		            
		            positionsetbuffer[0] = positionsetint;
		            positionsetbuffer[1] = g_GAME_ROUNDS+1;
		            positionsetbuffer[2] = wherewescored;
		            positionsetbuffer[3] = g_player[id].latestscore;
		            
		            // and lets send it to client
		            dgramlen = sendto(socketfd,positionsetbuffer,4,0,(struct sockaddr *)&g_player[id].player_address,addrlen);
		            
		            //printf("Tried to send: %d %d %d %d\n",positionsetbuffer[0],positionsetbuffer[1],positionsetbuffer[2],positionsetbuffer[3]);
		            
		            // checks if its time to go to score state        
		            if (g_player[0].scored == 1 && g_player[1].scored == 1 && g_player[2].scored == 1 && g_player[3].scored == 1){
								  g_STATE_SCORE = 1;
								}
              }
            }

		
		        // For when we get an QUIT message from client 

						if (message_id == 20) {
						  
						  printf("Recieved QUIT message from client number: %u\n",g_player[id].player_id);   
						  	  
						  // sending MC message that a player had left.
						  
						  leftbuffer[0] = leftint;
								
							*(uint16_t*)&leftbuffer[1] = htons(g_player[id].player_id); // takes 2 spots
							
							leftbuffer[3] = g_player[id].player_name[0];
							leftbuffer[4] = g_player[id].player_name[1];
							leftbuffer[5] = g_player[id].player_name[2];
							leftbuffer[6] = g_player[id].player_name[3];
							leftbuffer[7] = g_player[id].player_name[4];
							leftbuffer[8] = g_player[id].player_name[5];
							leftbuffer[9] = g_player[id].player_name[6];
							
							leftbuffer[10] = '\0';
							
							printf("Sending message: %d\n",mc_joinint);
							
							// Sending data to MC.
							dgramlen = sendto(mcsocketfd,leftbuffer,11,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen); 
						  
						  // need to work some way to let the gane jeeo going even though someone is gone
						  // but would need too much server logic fixing that I dont have time at 3am 28.10.    
						}

						if (g_STATE_SCORE == 1){
						  
						  // now we need to send a massive scoresheet
						  scoresheets[0] = scoresheetsint;

						  int zbang = 0; // just a help variable

						  for (e=0;e<=3;e++){
						    if (g_player[e].is_available == 1){
						      *(uint16_t*)&scoresheets[zbang+1] = htons(g_player[id].player_id);
									scoresheets[zbang+3] = g_player[e].ones;
									scoresheets[zbang+4] = g_player[e].twos;
									scoresheets[zbang+5] = g_player[e].threes;
									scoresheets[zbang+6] = g_player[e].fours;
									scoresheets[zbang+7] = g_player[e].fives;
									scoresheets[zbang+8] = g_player[e].sixes;
									scoresheets[zbang+9] = g_player[e].threeofakind;
									scoresheets[zbang+10] = g_player[e].fourofakind;
									scoresheets[zbang+11] = g_player[e].fullhouse;
									scoresheets[zbang+12] = g_player[e].lowstraight;
									scoresheets[zbang+13] = g_player[e].highstraight;
									scoresheets[zbang+14] = g_player[e].yahtzee;
									scoresheets[zbang+15] = g_player[e].chance;
									*(uint16_t*)&scoresheets[zbang+16] = htons(g_player[e].uppertotal);
									*(uint16_t*)&scoresheets[zbang+18] = htons(g_player[e].lowertotal);
									*(uint16_t*)&scoresheets[zbang+20] = htons(g_player[e].totaltotal);
						      zbang = zbang + 22;
						    }
						  }
						  
						  scoresheets[zbang+1] = '\0'; // this lets clients know where it ends

						  dgramlen = sendto(mcsocketfd,scoresheets,zbang+2,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);

						  g_STATE_SCORE = 0;
						  g_GAME_ROUNDS++;

						  // lets sleep abit
						  sleep(1);

						  // as we have now scored, we must send NEXT_ROUND to clients
						  nextbuffer[0] = nextint;
							dgramlen = sendto(mcsocketfd,nextbuffer,1,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);

              g_STATE_SCORE = 0;
              g_player[0].throws = 0;
              g_player[1].throws = 0;
              g_player[2].throws = 0;
              g_player[3].throws = 0;
              g_player[0].scored = 0;
              g_player[1].scored = 0;
              g_player[2].scored = 0;
              g_player[3].scored = 0;

						  if (g_GAME_ROUNDS == 13){
						  
						  
						    // Here is where TOP10 would come but TCP names arent working so unless that is fixed, this wont happen
						  
						  
						    // game ends here, will send mc to clients and reset all data
						    stopbuffer[0] = stopint;
						    // next send the GAME_STOP to clients.
						    dgramlen = sendto(mcsocketfd,stopbuffer,1,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);
						    initialize_players(4);
						    g_GAME_ROUNDS = 0;
						    g_STATE_GAME_IS_ON = 0;
						    g_IS_CLIENT_CONNECTED = 0;
						  }
						}     
			    
			    message_id = -1; // so the loop wont go haywire
			    
			    }
			    else{
			      
			      // This else means new TCP connections triggered, aka some client sends /msg or /name
			      
			      
			      memset(&tcppeekbuffer,0,sizeof(tcppeekbuffer));
		        // Lets peek inside so we get the length of the incoming message
				    byte_count = recv(i, tcppeekbuffer, sizeof(int), MSG_PEEK); 

            printf("We got: %d bytes in TCP.\n",byte_count); // we get 4 1st bytes so we can check pakcet lengths according to protocol

				    byte_count = ntohl(*(uint32_t*)&tcppeekbuffer[0]); // just dont understand why this doesnt work
				    
				    printf("Peeked length: %d \n",byte_count); // gives -234235236 types of numbers
							  
						// handle data from a client 
						if((nbytes = recv(i, tcpbuffer, byte_count, 0)) <= 0){
							// got error or connection closed by client 
							if(nbytes == 0){
								// connection closed, we got a FIN
								printf("Server: socket %d got closed\n", i);
							} 
							else{
								perror("recv() error");
							} 
							// close that socket
							close(i);
							// remove from MasterFd set 
							FD_CLR(i, &MasterFd);
							
							// ALSO send quits via mc and quit to client via udp
							
						}
						else {
						
						  // Lets see what kind of message that was
						  // Server only recieves two kinds
						
						  int tcpid;
						
						  // CHATMSG
						  if (tcpbuffer[4] == 40){
						    
						    // Need to figure out who it was
								if (g_player[0].is_available == 1 && check_client_address(0, &remote_address) == 0){
								  tcpid = 0;
								}
								if (g_player[1].is_available == 1 && check_client_address(1, &remote_address) == 0){
								  tcpid = 1;
								}
								if (g_player[2].is_available == 1 && check_client_address(2, &remote_address) == 0){
								  tcpid = 2;
								}
								if (g_player[3].is_available == 1 && check_client_address(3, &remote_address) == 0){
								  tcpid = 3;
								}
						    
						    printf("It was ID: %d\n",tcpid);
						    
						    // Need to somehow keep the message
						    for (e=5;e<byte_count;e++){
						      chatmessagebuffer[e-5] = tcpbuffer[e];
						      printf("%c",tcpbuffer[e]);
						      tcpmessage[e+6] = tcpbuffer[e];
						    }
						    
						    uint8_t neljayksi = 41;
						    uint8_t nimenpituus = 5;
						    byte_count = byte_count +6;
						    
						    *(uint32_t*)&tcpmessage[0] = htonl(byte_count); 
						    
						    tcpmessage[4] = neljayksi;
						    tcpmessage[5] = nimenpituus;
						    tcpmessage[6] = 'J';
						    tcpmessage[7] = 'e';
						    tcpmessage[8] = 'b';
						    tcpmessage[9] = 'a';
						    tcpmessage[10] = '\0';
						
						    // This is a rough version, itll send back message to all, and the sender will be Jepa
						    // So any client can send /msg <anything> and itll show to all clients as Jepa: <anything>
						    
						    // Ran out of time to get the logic working
						    
						  }
						  
						  // NAME_SEL (clients proposal for playername)
						  if (tcpbuffer[4] == 43){
						    
						    // Need to figure out who it was
								if (g_player[0].is_available == 1 && check_client_address(0, &remote_address) == 0){
								  tcpid = 0;
								}
								if (g_player[1].is_available == 1 && check_client_address(1, &remote_address) == 0){
								  tcpid = 1;
								}
								if (g_player[2].is_available == 1 && check_client_address(2, &remote_address) == 0){
								  tcpid = 2;
								}
								if (g_player[3].is_available == 1 && check_client_address(3, &remote_address) == 0){
								  tcpid = 3;
								}						    
						    // Dont have time to finish this, its 4am :/
						  }						  			  
						
							// we got some data from a client
							for(j = 0; j <= maxfd; j++) {
								// send to everyone
								if(FD_ISSET(j, &MasterFd))
								{
								  
								  // Need to chance chatmsg to playermsg
								  
								  //printf("%d\n",nbytes);
								  
								  //printf("%c%c%c%c%c",tcpbuffer[5],tcpbuffer[6],tcpbuffer[7],tcpbuffer[8],tcpbuffer[9]);
								  
									// except the tcp_socketfd and ourselves and UDP socket and keyboard
									if(j != tcp_socketfd && j != socketfd && j != STDIN_FILENO){
							      if(send(j, tcpmessage, byte_count, 0) == -1)
											perror("send() error");
				 				    }
									}
							  }
			 	      }
						}
					}
		    }
		  }
		}
  return 0;
}

// lets make scoring to its own function 
int score(int id,uint8_t field_id,int how_to_do_it) // player_id will be from 1-4 in assignment 2
{

  // 1st checks if we do this by 1st possible score OR by how client wanted
  
  int scoredhere = 0;
  int points = 0;
  
  if (how_to_do_it == 0){ // 1st possible

    if (g_player[id].ones == 'a'){ // aces
      g_player[id].ones = score_simple(id,1);
      printf("Player %d got score ones: %d\n",id,g_player[id].ones);
      scoredhere = 1;
      points = g_player[id].ones;
    }
    else if (g_player[id].twos == 'a'){ // twos
      g_player[id].twos = score_simple(id,2);
      printf("Player %d got score twos: %d\n",id,g_player[id].twos);
      scoredhere = 2;
      points = g_player[id].twos;
    }
    else if (g_player[id].threes == 'a'){ // threes
      g_player[id].threes = score_simple(id,3);
      printf("Player %d got score threes: %d\n",id,g_player[id].threes);
      scoredhere = 3;
      points = g_player[id].threes;
    }
    else if (g_player[id].fours == 'a'){ // fours
      g_player[id].fours = score_simple(id,4);
      printf("Player %d got score fours: %d\n",id,g_player[id].fours);
      scoredhere = 4;
      points = g_player[id].fours;
    }
    else if (g_player[id].fives == 'a'){ // fives
      g_player[id].fives = score_simple(id,5);
      printf("Player %d got score fives: %d\n",id,g_player[id].fives);
      scoredhere = 5;
      points = g_player[id].fives;
    }
    else if (g_player[id].sixes == 'a'){ // sixes
      g_player[id].sixes = score_simple(id,6);
      printf("Player %d got score sixes: %d\n",id,g_player[id].sixes);
      scoredhere = 6;
      points = g_player[id].sixes;
    }
    else if (g_player[id].threeofakind == 'a'){ // 3-of-a-kind
      g_player[id].threeofakind = score_ofakind(id,3);
      printf("Player %d got score threeofakind: %d\n",id,g_player[id].threeofakind);
      scoredhere = 7;
      points = g_player[id].threeofakind;
    }
    else if (g_player[id].fourofakind == 'a'){ // 4-of-a-kind
      g_player[id].fourofakind = score_ofakind(id,4);
      printf("Player %d got score fourofakind: %d\n",id,g_player[id].fourofakind);
      scoredhere = 8;
      points = g_player[id].fourofakind;
    }
    else if (g_player[id].fullhouse == 'a'){ // full house
      g_player[id].fullhouse = score_full_house(id);
      printf("Player %d got score full house: %d\n",id,g_player[id].fullhouse);
      scoredhere = 9;
      points = g_player[id].fullhouse;
    }
    else if (g_player[id].lowstraight == 'a'){ // low straigth
      g_player[id].lowstraight = score_low_straigth(id);
      printf("Player %d got score low straigth: %d\n",id,g_player[id].lowstraight);
      scoredhere = 10;
      points = g_player[id].lowstraight;
    }
    else if (g_player[id].highstraight == 'a'){ // high straigth
      g_player[id].highstraight = score_high_straigth(id);
      printf("Player %d got score high straigth: %d\n",id,g_player[id].highstraight);
      scoredhere = 11;
      points = g_player[id].highstraight;
    }
    else if (g_player[id].yahtzee == 'a'){ // yahtzee
      g_player[id].yahtzee = score_yahtzee(id);
      printf("Player %d got score yahtzee: %d\n",id,g_player[id].yahtzee);
      scoredhere = 12;
      points = g_player[id].yahtzee;
    }
    else if (g_player[id].chance == 'a'){ // chance
      g_player[id].chance = score_chance(id);
      printf("Player %d got score chance: %d\n",id,g_player[id].chance);
      scoredhere = 13;
      points = g_player[id].chance;
    }
    else{
      printf("This should never happen: right_way score error\n");
    }

  }
  if (how_to_do_it == 1){ // specific spot
  
    if (field_id == 1){ // aces
      g_player[id].ones = score_simple(id,1);
      printf("Player %d got score ones: %d\n",id,g_player[id].ones);
      scoredhere = 1;
      points = g_player[id].ones;
    }
    else if (field_id == 2){ // twos
      g_player[id].twos = score_simple(id,2);
      printf("Player %d got score twos: %d\n",id,g_player[id].twos);
      scoredhere = 2;
      points = g_player[id].twos;
    }
    else if (field_id == 3){ // threes
      g_player[id].threes = score_simple(id,3);
      printf("Player %d got score threes: %d\n",id,g_player[id].threes);
      scoredhere = 3;
      points = g_player[id].threes;
    }
    else if (field_id == 4){ // fours
      g_player[id].fours = score_simple(id,4);
      printf("Player %d got score fours: %d\n",id,g_player[id].fours);
      scoredhere = 4;
      points = g_player[id].sixes;
    }
    else if (field_id == 5){ // fives
      g_player[id].fives = score_simple(id,5);
      printf("Player %d got score fives: %d\n",id,g_player[id].fives);
      scoredhere = 5;
      points = g_player[id].fives;
    }
    else if (field_id == 6){ // sixes
      g_player[id].sixes = score_simple(id,6);
      printf("Player %d got score sixes: %d\n",id,g_player[id].sixes);
      scoredhere = 6;
      points = g_player[id].sixes;
    }
    else if (field_id == 7){ // 3-of-a-kind
      g_player[id].threeofakind = score_ofakind(id,3);
      printf("Player %d got score threeofakind: %d\n",id,g_player[id].threeofakind);
      scoredhere = 7;
      points = g_player[id].threeofakind;
    }
    else if (field_id == 8){ // 4-of-a-kind
      g_player[id].fourofakind = score_ofakind(id,4);
      printf("Player %d got score fourofakind: %d\n",id,g_player[id].fourofakind);
      scoredhere = 8;
      points = g_player[id].fourofakind;
    }
    else if (field_id == 9){ // full house
      g_player[id].fullhouse = score_full_house(id);
      printf("Player %d got score full house: %d\n",id,g_player[id].fullhouse);
      scoredhere = 9;
      points = g_player[id].fullhouse;
    }
    else if (field_id == 10){ // low straigth
      g_player[id].lowstraight = score_low_straigth(id);
      printf("Player %d got score low straigth: %d\n",id,g_player[id].lowstraight);
      scoredhere = 10;
      points = g_player[id].lowstraight;
    }
    else if (field_id == 11){ // high straigth
      g_player[id].highstraight = score_high_straigth(id);
      printf("Player %d got score high straigth: %d\n",id,g_player[id].highstraight);
      scoredhere = 11;
      points = g_player[id].highstraight;
    }
    else if (field_id == 12){ // yahtzee
      g_player[id].yahtzee = score_yahtzee(id);
      printf("Player %d got score yahtzee: %d\n",id,g_player[id].yahtzee);
      scoredhere = 12;
      points = g_player[id].yahtzee;
    }
    else if (field_id == 13){ // chance
      g_player[id].chance = score_chance(id);
      printf("Player %d got score chance: %d\n",id,g_player[id].chance);
      scoredhere = 13;
      points = g_player[id].chance;
    }
    else{
      printf("This should never happen: right_way score error\n");
    }
  }
  
  g_player[id].totaltotal = 0;
	
	if (g_player[id].uppertotal >= 63 && g_player[id].has_bonus_been_given == 0){
		g_player[id].uppertotal = g_player[id].uppertotal + 35;
		g_player[id].has_bonus_been_given = 1;
	}

	g_player[id].totaltotal = g_player[id].uppertotal + g_player[id].lowertotal;
	
	g_player[id].latestscore = points; // need to use this as functions cant return many values

// Score return the field id of where it was scored
return scoredhere;
}

// scoring ones/twos/threes(fours/fives/sixes
uint8_t score_simple(int id,uint8_t k) 
{
  uint8_t scoretoadd = 0;

  if (g_player[id].dice1 == k){
    scoretoadd = scoretoadd + k;
  }
  if (g_player[id].dice2 == k){
    scoretoadd = scoretoadd + k;
  }
  if (g_player[id].dice3 == k){
    scoretoadd = scoretoadd + k;
  }
  if (g_player[id].dice4 == k){
    scoretoadd = scoretoadd + k;
  }
  if (g_player[id].dice5 == k){
    scoretoadd = scoretoadd + k;
  }  

g_player[id].uppertotal = g_player[id].uppertotal + scoretoadd;
return scoretoadd;
}

// scoring 3 or 4 -ofakind
uint8_t score_ofakind(int id, int kind)
{
	int ofakind = 0;
	int e=0;
	uint8_t scoretoadd = 0;
	
	for (e=1;e<=6;e++){
	
	  ofakind = 0;
	  
		if (g_player[id].dice1 == e){
		  ofakind++;
		}
		if (g_player[id].dice2 == e){
		  ofakind++;
		}
		if (g_player[id].dice3 == e){
		  ofakind++;
		}
		if (g_player[id].dice4 == e){
		  ofakind++;
		}
		if (g_player[id].dice5 == e){
		  ofakind++;
		}
  
		if (ofakind >= kind) {
		  scoretoadd = e * kind;
		  if (kind == 3){
				g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
		    return scoretoadd;
		  }
		  if (kind == 4){
				g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
		    return scoretoadd;
		  }
		}
  }
  // there was no 3ofakind or 4ofakind so score is 0
  scoretoadd = 0;
  return scoretoadd;
}

// scoring full house
uint8_t score_full_house(int id)
{	
	uint8_t scoretoadd = 25;
  int trigger = 0;
	if (g_player[id].dice1 == g_player[id].dice2 && (g_player[id].dice3 == g_player[id].dice4 && g_player[id].dice3 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice1 == g_player[id].dice3 && (g_player[id].dice2 == g_player[id].dice4 && g_player[id].dice2 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice1 == g_player[id].dice4 && (g_player[id].dice2 == g_player[id].dice3 && g_player[id].dice2 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice1 == g_player[id].dice5 && (g_player[id].dice2 == g_player[id].dice3 && g_player[id].dice2 == g_player[id].dice4)){
		trigger = 1;
	}
	if (g_player[id].dice2 == g_player[id].dice3 && (g_player[id].dice1 == g_player[id].dice4 && g_player[id].dice1 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice2 == g_player[id].dice4 && (g_player[id].dice1 == g_player[id].dice3 && g_player[id].dice1 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice2 == g_player[id].dice5 && (g_player[id].dice1 == g_player[id].dice3 && g_player[id].dice1 == g_player[id].dice4)){
		trigger = 1;
	}
	if (g_player[id].dice3 == g_player[id].dice4 && (g_player[id].dice1 == g_player[id].dice2 && g_player[id].dice1 == g_player[id].dice5)){
		trigger = 1;
	}
	if (g_player[id].dice3 == g_player[id].dice5 && (g_player[id].dice1 == g_player[id].dice2 && g_player[id].dice1 == g_player[id].dice4)){
		trigger = 1;
	}
	if (g_player[id].dice4 == g_player[id].dice5 && (g_player[id].dice1 == g_player[id].dice2 && g_player[id].dice1 == g_player[id].dice3)){
		trigger = 1;
	}
  if (trigger == 1){
		g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
    return scoretoadd;
  }
  // if no full house, returning 0
  return 0;
}

// scoring low straigth
uint8_t score_low_straigth(int id)
{
	uint8_t scoretoadd = 0;
	if (g_player[id].dice1 != 6 && g_player[id].dice2 != 6 && g_player[id].dice3 != 6 && g_player[id].dice4 != 6 && g_player[id].dice5 != 6 && g_player[id].dice1 != g_player[id].dice2 && g_player[id].dice1 != g_player[id].dice3 && g_player[id].dice1 != g_player[id].dice4 && g_player[id].dice1 != g_player[id].dice5 && g_player[id].dice2 != g_player[id].dice3 && g_player[id].dice2 != g_player[id].dice4 && g_player[id].dice2 != g_player[id].dice5 && g_player[id].dice3 != g_player[id].dice4 && g_player[id].dice3 != g_player[id].dice5 && g_player[id].dice4 != g_player[id].dice5) {
		scoretoadd = 30;
		g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
		return scoretoadd;
	}
  // if no low straigth, returning 0
  return 0;
}

// scoring high straigth
uint8_t score_high_straigth(int id)
{

	uint8_t scoretoadd = 0;
	if (g_player[id].dice1 != 1 && g_player[id].dice2 != 1 && g_player[id].dice3 != 1 && g_player[id].dice4 != 1 && g_player[id].dice5 != 1 && g_player[id].dice1 != g_player[id].dice2 && g_player[id].dice1 != g_player[id].dice3 && g_player[id].dice1 != g_player[id].dice4 && g_player[id].dice1 != g_player[id].dice5 && g_player[id].dice2 != g_player[id].dice3 && g_player[id].dice2 != g_player[id].dice4 && g_player[id].dice2 != g_player[id].dice5 && g_player[id].dice3 != g_player[id].dice4 && g_player[id].dice3 != g_player[id].dice5 && g_player[id].dice4 != g_player[id].dice5) {
		scoretoadd = 40;
		g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
		return scoretoadd;
	}
  // if no high straigth, returning 0
  return 0;
}

// scoring yahtzee
uint8_t score_yahtzee(int id)
{  
  uint8_t scoretoadd = 50;
  
  if (g_player[id].dice1 == g_player[id].dice2 && g_player[id].dice1 == g_player[id].dice3 && g_player[id].dice1 == g_player[id].dice4 && g_player[id].dice1 == g_player[id].dice5) {
    g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
    return scoretoadd;
  }
  // if no yahtzee, return 0
  return 0;
}

// scoring chance
uint8_t score_chance(int id)
{
  uint8_t scoretoadd = 0;
  
	scoretoadd = g_player[id].dice1 + g_player[id].dice2 + g_player[id].dice3 + g_player[id].dice4 + g_player[id].dice5;
	
	g_player[id].lowertotal = g_player[id].lowertotal + scoretoadd;
	
  return scoretoadd;
}

// Resetting/Initializing Player global structs
int initialize_players(int id)
{
	id = id -1;
	int e = 0;

	for (e=0;e<=id;e++){

		if (e == 0){
		  g_player[e].player_name = "Player1"; 
		}
		if (e == 1){
		  g_player[e].player_name = "Player2"; 
		}
		if (e == 2){
		  g_player[e].player_name = "Player3"; 
		}
		if (e == 3){
		  g_player[e].player_name = "Player4"; 
		}
		//g_player[e].nick_chosen = 0;
		g_player[e].player_id = e+1;
		//g_player[e].nick_chosen = 0;
		g_player[e].dice1 = 0;
		g_player[e].dice2 = 0;
		g_player[e].dice3 = 0;
		g_player[e].dice4 = 0;
		g_player[e].dice5 = 0;
		g_player[e].is_available = 0; // 0 means its free, 1 means its taken
		g_player[e].ones = 'a';
		g_player[e].twos = 'a';
		g_player[e].threes = 'a';
		g_player[e].fours = 'a';
		g_player[e].fives = 'a';
		g_player[e].sixes = 'a';
		g_player[e].threeofakind = 'a';
	 	g_player[e].fourofakind = 'a';
		g_player[e].fullhouse = 'a';
		g_player[e].lowstraight = 'a';
		g_player[e].highstraight = 'a';
		g_player[e].yahtzee = 'a';
		g_player[e].chance = 'a';
		g_player[e].uppertotal = 0;
		g_player[e].lowertotal = 0;
		g_player[e].totaltotal = 0;
		g_player[e].has_bonus_been_given = 0;
    g_player[e].throws = 0;
    memset((char *) &g_player[e].player_address, 0, sizeof(g_player[e].player_address));
    g_player[e].round = 1;
    g_player[e].scored = 0; // 1 means has scored that round
    g_player[e].latestscore = 0;
    g_player[e].nickchosen = 0;
	}

return 0;
}

// checks if address is same as given IDs and new ones, based from assignment 1 notes
int check_client_address(int id, struct sockaddr_in * newcomer)
{
  printf("Comparing ip: %s vs %s\n",(char *)inet_ntoa(g_player[id].player_address.sin_addr),(char *)inet_ntoa(newcomer->sin_addr));
  // To check that the address is the same
  if(g_player[id].player_address.sin_addr.s_addr != newcomer->sin_addr.s_addr){   
    //printf("Returned -1 :(\n");
    return -1;
  }
  //printf("Returned 0 :)\n");
  return 0;
}

// checks if port is same as given IDs and new ones, based from assignment 1 notes
int check_client_port(int id, struct sockaddr_in * newcomer)
{
  printf("Comparing port: %d vs %d\n",g_player[id].player_address.sin_port,newcomer->sin_port);
  // To check that the port is the same
  if(g_player[id].player_address.sin_port != newcomer->sin_port){ 
    return -1; 
  }
return 0;
}

//EOF
