//////////////////////////////////
//                              //
//  Network Programming         //
//  Assignment 4                //
//  yahtzeeclient.c             //
//                              //
//  Kimmo Pietiläinen           //
//                              //
//  See readme file for info!   //
//                              //
//////////////////////////////////

// This is bugfix version

#include "yahtzeeclient.h"

// Main function
int main(int argc, char *argv[])
{
  // help variables for argument sorting
  int e=1;
  int helper1=0,helper2=0,helper3=0,helper4=0,helper5=0;
  int ok1=0,ok2=0,ok3=0,ok4=0,ok5=0;
  // Checking if we got the right amount of arguments
  if (argc == 11)
  {
    for (e=1;e<=9;e+=2){ // so the argument order doesnt matter to user
      if (strncmp(argv[e], "-h", 2) == 0){
        helper1=e+1;
        ok1=1;
      }
      if (strncmp(argv[e], "-up", 3) == 0){
        helper2=e+1;
        ok2=1;
      }
      if (strncmp(argv[e], "-tp", 3) == 0){
        helper3=e+1;
        ok3=1;
      }
      if (strncmp(argv[e], "-m\0", 3) == 0){
        helper4=e+1;
        ok4=1;
      }
      if (strncmp(argv[e], "-mp", 3) == 0){
        helper5=e+1;
        ok5=1;
      }
    }
    if (ok1 == 1 && ok2 == 1 && ok3 == 1 && ok4 == 1 && ok5 == 1){
      if (client(argv[helper1],atoi(argv[helper2]),atoi(argv[helper3]),argv[helper4],atoi(argv[helper5])) == 0){ // starting client
        printf("Client turned off.\n");
      }
      else
      {
        printf("Client error.\n");
        return -1;
      }
    }
    else { // this happens if user makes mistake with -h or -up or -tp or m- or -mp.   
      printf("Argument error.\nReadme file for more information:\n\
      Client: %s -h <server address> -up <UDP port number> -tp <TCP port number> -m <multicast address> -mp <multicast port number>\n",argv[0]);
      return -1;
    }
  }
  // Argument errors
  else {
    printf("Invalid amount of arguments.\nReadme file for more information:\n\
    Client: %s -h <server address> -up <UDP port number> -tp <TCP port number> -m <multicast address> -mp <multicast port number>\n",argv[0]);
    return -1;
  }
  return 0;
}

// Client function
int client(char *serverip,int port,int tcp_port,char *mcip, int mcport)
{
  int socketfd; // sockets file descriptors
  int mcsocketfd;
  int tcp_socketfd;

  int selecterror;

  int msglength = 0; // tcp helper
  
  uint32_t packetsize; // tcp packetsize

  int byte_count; // tcp helpers
  int nbytes;
  
  char myscore[19]; // helper with score

  int round = 0;

  int length = 0; // length of sent/recieved datagram

  int loopdeloop = 1; // main loop variable

  int i,e,k,p,n; // for-clauses

  int whereistheend; // checking the '\0' in player typed messages.

  uint8_t what_message; // for checking message id
  uint8_t mc_what_message;

  // help variables for JOIN message
  uint8_t joinint = 1;
  char joinbuffer[1];
  // help variables for ROLL_DICE message
  uint8_t rollint = 3;
  char rollbuffer[1];
  // help variables for LOCK message
  uint8_t lockint = 5;
  char lockbuffer[6];
  // help variables for SCORE message
  uint8_t scoreint = 6;
  char scorebuffer[1];
  // help variables for QUIT message
  uint8_t quitint = 20;
  char quitbuffer[1];
  // help variables for SET_SCORE message
  uint8_t setscoreint = 11;
  char setscorebuffer[3];
  
  // help variables for HELLO message
  uint8_t helloint = 36;
  char hellobuffer[7];
  // help variables for CHEERS message
  uint8_t cheersint = 37;
  char cheersbuffer[8]; 

  // help variables for scoring the totals. (16bit)
  uint16_t uppertotal = 0;
  uint16_t lowertotal = 0;
  uint16_t totaltotal = 0;

  uint8_t messageresetter = 0; // for resetting message

  // help variables for scoring the other scores (8bit)
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

  uint16_t player_id; 

  char player_name[50]; 

  uint8_t dice1;
  uint8_t dice2;
  uint8_t dice3;
  uint8_t dice4;
  uint8_t dice5;

  socklen_t addrlen, server_addrlen,tcp_server_addrlen, multicast_group_addrlen, multicast_local_addrlen, client_addrlen, tcp_client_addrlen; // address length

  struct ip_mreq group; // for multicast group management
  struct in_addr localInterface;
  struct sockaddr_in server_address,tcp_server_address, client_address, tcp_client_address, multicast_group_address, multicast_local_address; // Struct for internet address info 

  // from example
  struct in_addr iaddr;
  memset(&iaddr, 0, sizeof(struct in_addr));

  // Info for user.
  printf("Yahtzee client.\n");
  //printf("Server: %s Port: %i\n", serverip,port);

  int binderror = 0;
  int client_port = 0; // helpers with bind
  int tcp_client_port = 0;

  char message[BUFFLEN];  // Buffer for STDIN
  char chatmessage[BUFFLEN];

  char recmessage[BUFFLEN];  // Buffer for the server reply
  char tcpbuffer[TCPBUFFLEN]; // buffer for incoming tcp messages
  char tcppeekbuffer[4]; // this is used to see the next messages length in tcp socket
  
  // Variables for select() timeout timer. Beej's.
  fd_set RecFd,MasterFd; // Filedescriptorsets
  int maxfd; // highest file descriptor
  //struct timeval timeout; // For timeout, NOT USED IN ASSIGNMENT 4

  // Select timeout values
  //timeout.tv_sec=10;  // It is set to timeout after 10 seconds.
  //timeout.tv_usec=0;  // NOT USED IN ASSIGNMENT 1.

  //server_addrlen = sizeof(server_address); // For getting the length of struct for internet address
  
  // Checking if port number is ok
  if (port > 1024 && port < 65000) {
  
    // UDP Socket creation
    if ((socketfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
      perror("Socket: ");
      return -1; // checks if socket creation goes ok
    }

    server_addrlen=sizeof(server_address);
    memset(&server_address,0,server_addrlen); // Clearing server address info
    // based on udpexample
    server_address.sin_family = AF_INET; // initialize network family type (IPv4 or IPv6)
    server_address.sin_port = htons(port); // Setting port number in network byte order 
  
    if(inet_pton(AF_INET,serverip,&(server_address.sin_addr)) <= 0) { // assign ip to address struct 
      perror("inet_pton()");
      return -1;
    }

    // Initializing client variables and client address struct
    client_addrlen=sizeof(client_address);
    memset(&client_address,0,client_addrlen); // Clearing client address info
    client_address.sin_family = AF_INET; // Initializing network family (IPv4 or IPv6)
    client_address.sin_addr.s_addr = INADDR_ANY; // Using any address for client
    client_address.sin_port=htons(clientlistenport); // Assigning port in network byte order

    // binding to a port, so we know if there are other clients on this computar  
		if(bind(socketfd, (struct sockaddr*)&client_address, sizeof(client_address)))
		{
			perror("Binding datagram socket error");
      binderror = 1;
		}
    else { 
      binderror = 0;
      printf("Bind to port %d\n", clientlistenport);
	  }
    
    client_port = clientlistenport;

    while (binderror == 1){
		  if (binderror == 1){ // need to change client listen port
		    client_port = client_port +1;
		    client_address.sin_port=htons(client_port);
		  }
			if(bind(socketfd, (struct sockaddr*)&client_address, sizeof(client_address)))
			{
				perror("Binding datagram socket error");
		    binderror = 1;
			}
      else { 
        binderror = 0;
        printf("Bind to port %d\n", client_port);
			}
    }

		// TCP Socket creation
		if ((tcp_socketfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
      perror("TCPSOCKET: ");
			return -1; // checks if socket creation goes ok
		}

		tcp_server_addrlen=sizeof(tcp_server_address);
		memset(&tcp_server_address,0,tcp_server_addrlen); // Clearing server address info
		tcp_server_address.sin_family = AF_INET; // initialize network family type 
		tcp_server_address.sin_port = htons(tcp_port); // Setting port number in network byte order 

		if(inet_pton(AF_INET,serverip,&(tcp_server_address.sin_addr)) <= 0) { // assign ip to address struct 
			perror("inet_pton()");
			return -1;
		}

		// Initializing client variables and client address struct
		tcp_client_addrlen=sizeof(tcp_client_address);
		memset(&tcp_client_address,0,tcp_client_addrlen); // Clearing client address info
		tcp_client_address.sin_family = AF_INET; // Initializing network family
		tcp_client_address.sin_addr.s_addr = INADDR_ANY; // Using any address for client
		tcp_client_address.sin_port=htons(tcp_clientlistenport); // Assigning port in network byte order

		// binding to a port, so we know if there are other clients on this computar  
		if(bind(tcp_socketfd, (struct sockaddr*)&tcp_client_address, sizeof(tcp_client_address)))
		{
			perror("Binding datagram socket error");
			binderror = 1;
		}
		else { 
			binderror = 0;
			printf("Bind to port %d\n", tcp_clientlistenport);
		}
	
		tcp_client_port = tcp_clientlistenport;

		while (binderror == 1){
			if (binderror == 1){ // need to change client listen port
				tcp_client_port = tcp_client_port +1;
				tcp_client_address.sin_port=htons(tcp_client_port);
			}
			if(bind(tcp_socketfd, (struct sockaddr*)&tcp_client_address, sizeof(tcp_client_address)))
			{
				perror("Binding datagram socket error");
				binderror = 1;
			}
			else { 
			  binderror = 0;
			  printf("Bind to port %d\n", tcp_client_port);
			}
		}


////////////////////// MULTICAST

    // Socket creation
    if ((mcsocketfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
      return -1; // checks if socket creation goes ok
    }    

		// Enable SO_REUSEADDR to allow multiple instances of this 
		// application to receive copies of the multicast datagrams

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
			printf("Setting SO_REUSEADDR...OK.\n");
		}

		// setting up addresses

		multicast_group_addrlen=sizeof(multicast_group_address);
		multicast_local_addrlen=sizeof(multicast_local_address);
		
		memset((char *) &multicast_group_address, 0, sizeof(multicast_group_address));
		multicast_group_address.sin_family = AF_INET;
		multicast_group_address.sin_addr.s_addr = INADDR_ANY;
		multicast_group_address.sin_port = htons(mcport);

		memset((char *) &multicast_local_address, 0, sizeof(multicast_local_address));
		multicast_local_address.sin_family = AF_INET;
		multicast_local_address.sin_port = htons(mcport);
		multicast_local_address.sin_addr.s_addr = INADDR_ANY;

    // Bind mc socket (many clients can bind same port now on same computor)
		if(bind(mcsocketfd, (struct sockaddr*)&multicast_local_address, sizeof(multicast_local_address)))
		{
			perror("Binding datagram socket error");
			close(socketfd);
		  close(mcsocketfd);
			exit(1);
		}
		else {
			printf("Binding datagram socket...OK.\n");
		}

    if(inet_pton(AF_INET,mcip,&(multicast_group_address.sin_addr)) <= 0) { // assign ip to address struct 
      perror("inet_pton()");
      return -1;
    }

    //printf("%s\n",(char *)inet_ntoa(multicast_address.sin_addr));
    //printf("%s\n",(char *)inet_ntoa(client_address.sin_addr));

    // from example 
    //iaddr.s_addr = INADDR_ANY; // use DEFAULT interface
    
    // Set the outgoing interface to DEFAULT
    //setsockopt(mcsocketfd, IPPROTO_IP, IP_MULTICAST_IF, &iaddr, sizeof(struct in_addr));

  	localInterface.s_addr = INADDR_ANY;

		if(setsockopt(mcsocketfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
		{
			perror("Setting local interface error");
			close(socketfd);
		  close(mcsocketfd);
			exit(1);
		}
		else {
			printf("Setting the local interface...OK\n");
		}

		group.imr_multiaddr.s_addr = inet_addr(mcip); //multicast_address.sin_addr.s_addr;
		group.imr_interface.s_addr = INADDR_ANY;  //inet_addr("xxx.xxx.xxx.xxx"); //client_address.sin_addr.s_addr;
	
		addrlen = sizeof(group.imr_interface.s_addr);

    // joining MC group
		if(setsockopt(mcsocketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0)
		{
			perror("Adding multicast group error");
			close(socketfd);
      close(mcsocketfd);
			exit(1);
		}
		else
		{
			printf("Adding multicast group...OK.\n");
		}

    // Adding TTL from 1 to 7
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
			printf("Increased TTL to 7...OK.\n");
		}

    // Im having loop enabled for clients, because im testing this on loopback address and 
    // MC messages wont work at all unless this is enabled

    // Setting loop
    unsigned char one = 1; // 0 to disable loop, 1 to enable loop

		if(setsockopt(mcsocketfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&one, sizeof(unsigned char)) < 0)
		{
			perror("Setting IP_MULTICAST_LOOP error");
			close(socketfd);
			close(mcsocketfd);
			exit(1);
		}
		else
		{
			printf("Looping is enabled.\n");
		}
    
    if (Global_Connected == 0){ // checks if we are connected, if not then automatically joins

      joinbuffer[0] = joinint;  
  
      // Sending data to server
      length = sendto(socketfd,joinbuffer,1,0,(struct sockaddr *)&server_address,server_addrlen);

      printf("Connecting to server...%d\n",length);

      // Changing global variable Global_Connected to 1, so client will not ask for /connect again.
      Global_Connected = 1;
      
      // getting a message
      length = recvfrom(socketfd,recmessage,BUFFLEN,0,(struct sockaddr *)&client_address,&addrlen);
      
      //printf("Did we get here?\n");
      
      if (length <0) { printf("Recvfrom error.\n");}

      // This is used to see the 1st byte of the message which always contains the message type id according to protocol
      what_message = recmessage[0]; 
     
      // We got an ACCEPT message from server.
      if (what_message == 2){

        // Need to gather player id and player name info 1st
      
        // Because player_id is 16bit integer i had to pack it as 2x 8bit and now unpack it to 16bit
        player_id = ((uint16_t) recmessage[1] << 8) | recmessage[2]; // bitshifting and casting to 16bit

        //memcpy(&recmessage[1],player_id,200);
      
        for (k=3;k<=10;k++){
          player_name[k-3] = recmessage[k];
        }

        printf("We got accepted! \nPlayer name: %s \nPlayer ID: %d \n", player_name, player_id);
        
        // INITIALIZING TCP CONNECTION AFTER ACCEPT MESSAGE
        // connecting to server tcp listener
        if (connect(tcp_socketfd, (struct sockaddr*)&tcp_server_address, sizeof(tcp_server_address)) == -1){
          perror("Connect() error:");
        }

        // we are now connected, but we wont be doing anything with it untill we get chat messages or try to send some ourselves.
        
        hellobuffer[0] = helloint;
        
        hellobuffer[1] = 'H';
        hellobuffer[2] = 'E';
        hellobuffer[3] = 'L';
        hellobuffer[4] = 'L';
        hellobuffer[5] = 'O';
        hellobuffer[6] = '\0';

        length = sendto(mcsocketfd,hellobuffer,7,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);

        what_message = 0;
      }
    }

    // resetting filedestriptorsets
    FD_ZERO(&RecFd);
    FD_ZERO(&MasterFd);

	  if (socketfd > mcsocketfd && socketfd > tcp_socketfd){
      maxfd = socketfd;
    }
	  if (mcsocketfd > socketfd && mcsocketfd > tcp_socketfd){
      maxfd = mcsocketfd;
    }
	  if (tcp_socketfd > socketfd && tcp_socketfd > mcsocketfd){
      maxfd = tcp_socketfd;
    }

    // Setting up filedescriptorsets with socket, mcsocket and STDIN
    FD_SET(STDIN_FILENO, &MasterFd);
    FD_SET(socketfd, &MasterFd);
    FD_SET(mcsocketfd, &MasterFd);
    FD_SET(tcp_socketfd, &MasterFd);
    
/////////// Main loop for client ////////////////////

    while (loopdeloop == 1){

      FD_ZERO(&RecFd);
      RecFd = MasterFd; // because select changed the RecFd 
      
      // memcpy(&RecFd, &MasterFd, sizeof(MasterFd)); // another way to do the above
    
      // Timeout alarm, after 10 seconds it shuts down the client
      //alarm(10); not used, left for documentation or probably future use
	
      // Timeout using select from Beej's
      selecterror = select (maxfd+1, &RecFd,NULL,NULL,NULL);  // Could add &timeout if put in the last NULL

      if (selecterror < 0){
        perror("Select error");
      }

/////////////////////KEYBOARD
      
      if(FD_ISSET(STDIN_FILENO, &RecFd)) { // this checks if anything happens in keyboard
      
        memset(&message,'\0',BUFFLEN);
        
        fgets (message , BUFFLEN , stdin);

        // for name selection
        if (strncmp(message, "/name ", 6) == 0) {
          
          // Need to find the end of the message
          for (e=0;e<BUFFLEN;e++){
            if (message[e] == '\0'){
              whereistheend = e;
              e = 2000;
            }
          }

          // We have the end now so we need to send it to server
          // Whereistheend also works as strlen just need to take /name_ aka 6 off it.
          memset(&chatmessage,'\0',BUFFLEN);
       
          // Lets also store name in client
          memset(&player_name,'\0',50); // resetting name   

          // need to copy the contents
          for (e=0;e<(whereistheend-6);e++){
            chatmessage[e+5] = message[e+6]; // +5 to leave room for msglength and message id
            player_name[e] = message[e];
          }

          //printf("You typed: %s!\n",chatmessage);

          // NAME_SEL to server

          msglength = 0;
          msglength = whereistheend -1; // 5-6 = -1 from header - /name_

          packetsize = msglength;

          //chatmessage[0] = msglength; 
          *(uint32_t*)&chatmessage[0] = htonl(packetsize);      

          printf("Packetsize: %d\n",packetsize);

          uint8_t nameselint = 43;
          chatmessage[4] = nameselint;

          send(tcp_socketfd,chatmessage,msglength,0);

          whereistheend = 0;
          memset(&message,'\0',BUFFLEN);
          memset(&chatmessage,'\0',BUFFLEN);
        }

        // for chat messages
        else if (strncmp(message, "/msg ", 5) == 0) {
          
          // Need to find the end of the message
          for (e=0;e<BUFFLEN;e++){
            if (message[e] == '\0'){
              whereistheend = e;
            }
          }

          // We have the end now so we need to send it to server
          // Whereistheend also works as strlen just need to take /msg_ aka 5 off it.
          memset(&chatmessage,'\0',BUFFLEN);
          
          // need to copy the contents
          for (e=0;e<(whereistheend-5);e++){
            chatmessage[e+5] = message[e+5];
          }

          //printf("You typed: %s!\n",chatmessage);

          msglength = 0;
          msglength = whereistheend -1; // 5-6 = -1 from header - /name_
          
          packetsize = msglength;

          //chatmessage[0] = msglength; 
          *(uint32_t*)&chatmessage[0] = htonl(packetsize); 

          uint8_t chatmsgint = 40;
          chatmessage[4] = chatmsgint;

          send(tcp_socketfd,chatmessage,msglength,0); 

          whereistheend = 0;
          memset(&message,'\0',BUFFLEN);
          memset(&chatmessage,'\0',BUFFLEN);
        }

        // to see newest scoresheet
        else if (strncmp(message, "/scoresheet", 11) == 0) {

          // now we must printf our scoresheet
	        printf("We got score: \n");
	        
	        // We have a problem here as we got those chars mixed up with 8bit integers
	        if (myscore[0] == 'a') { // Ones
	          printf("Ones: a ");
	        }
	        else {
	          ones = myscore[0];
	          printf("Ones: %u ",ones);
	        }
	        if (myscore[1] == 'a') { // Twos
	          printf("Twos: a ");
	        }
	        else {
	          twos = myscore[1];
	          printf("Twos: %u ",twos);
	        }
	        if (myscore[2] == 'a') { // Threes
	          printf("Threes: a ");
	        }
	        else {
	          threes = myscore[2];
	          printf("Threes: %u ",threes);
	        }
	        if (myscore[3] == 'a') { // Fours
	          printf("Fours: a ");
	        }
	        else {
	          fours = myscore[3];
	          printf("Fours: %u ",fours);
	        }
	        if (myscore[4] == 'a') { // Fives
	          printf("Fives: a ");
	        }
	        else {
	          fives = myscore[4];
	          printf("Fives: %u ",fives);
	        }
	        if (myscore[5] == 'a') { // Sixes
	          printf("Sixes: a ");
	        }
	        else {
	          sixes = myscore[5];
	          printf("Sixes: %u ",sixes);
	        }
	        if (myscore[6] == 'a') { // Threeofakind
	          printf("Three-of-a-kind: a ");
	        }
	        else {
	          threeofakind = myscore[6];
	          printf("Three-of-a-kind: %u ",threeofakind);
	        }
	        if (myscore[7] == 'a') { // Fourofakind
	          printf("Four-of-a-kind: a ");
	        }
	        else {
	          fourofakind = myscore[7];
	          printf("Four-of-a-kind: %u ",fourofakind);
	        }
	        if (myscore[8] == 'a') { // Fullhouse
	          printf("Full House: a ");
	        }
	        else {
	          fullhouse = myscore[8];
	          printf("Full House: %u ",fullhouse);
	        }
	        if (myscore[9] == 'a') { // Smallstraight
	          printf("Small straight: a ");
	        }
	        else {
	          lowstraight = myscore[9];
	          printf("Small straight: %u ",lowstraight);
	        }
	        if (myscore[10] == 'a') { // Largestraight
	          printf("Large straight: a ");
	        }
	        else {
	          highstraight = myscore[10];
	          printf("Large straight: %u ",highstraight);
	        }
	        if (myscore[11] == 'a') { // Yahtzee
	          printf("Yahtzee: a ");
	        }
	        else {
	          yahtzee = myscore[11];
	          printf("Yahtzee: %u ",yahtzee);
	        }
	        if (myscore[12] == 'a') { // Chance
	          printf("Chance: a ");
	        }
	        else {
	          chance = myscore[12];
	          printf("Chance: %u ",chance);
	        }

	        printf("Upper total: %u Lower total: %u Total: %u \n\n",myscore[13],myscore[15],myscore[17]);

        }

        // if player wanted to lock something
        else if (strncmp(message, "/lock", 5) == 0) {
          
          printf("Locking dices: ");
          // Lets see where we have '\0̈́'
          for (i = 7; i <= 16; i++){
            if (message[i]) { // this looks up where NULL is so i know the end        
              whereistheend = i; // Now we know where message ends.
            }
          }
          
          // Need to reset the locks
          Global_Dice_1_Locked = 0;
          Global_Dice_2_Locked = 0;
          Global_Dice_3_Locked = 0;
          Global_Dice_4_Locked = 0;
          Global_Dice_5_Locked = 0;
          
          for (e = 6; e < whereistheend; e+=2){
            //printf("%d:  %c    ",e,message[e]);
            if (atoi(&message[e]) == 1){
              Global_Dice_1_Locked = 1;
              printf("1 ");
            }
            else if (atoi(&message[e]) == 2){ // strncmp(&message[e], "2", 1) == 0 <- what i used to have and it didnt work
              Global_Dice_2_Locked = 1;
              printf("2 ");
            }
            else if (atoi(&message[e]) == 3){
              Global_Dice_3_Locked = 1;
              printf("3 ");
            }
            else if (atoi(&message[e]) == 4){
              Global_Dice_4_Locked = 1;
              printf("4 ");
            }
            else if (atoi(&message[e]) == 5){
              Global_Dice_5_Locked = 1;
              printf("5 ");
            }
          }
          printf("\n");
          // Need to reset the message buffer as it might pass some variables over.
          // if the next /lock message is shorter than the earlier
          for (p=0;p<20;p++){
            message[p] = messageresetter;
          }
          // Now we know which dices we want to lock
          // Next we must send that information to the server

          lockbuffer[0] = lockint;
          lockbuffer[1] = Global_Dice_1_Locked;
          lockbuffer[2] = Global_Dice_2_Locked;
          lockbuffer[3] = Global_Dice_3_Locked;
          lockbuffer[4] = Global_Dice_4_Locked;
          lockbuffer[5] = Global_Dice_5_Locked;

          // Sending data to server
          length = sendto(socketfd,lockbuffer,6,0,(struct sockaddr *)&server_address,addrlen);
          //printf("We tried to send locked message.\n");
          
        }
        // if player wanted to score
        else if (strncmp(message, "/score", 6) == 0) {
          // send SCORE command to server
          scorebuffer[0] = scoreint;
          // Sending data to server
          length = sendto(socketfd,scorebuffer,1,0,(struct sockaddr *)&server_address,addrlen);
        }
        // If player wanted to quit (protocol says we shouldnt quit here, but i like to give that option)
        else if (strncmp(message, "/quit", 5)== 0) {
          printf("Client shutting down.\n");
          // Need to send quit message to server
          quitbuffer[0] = quitint;
          length = sendto(socketfd,quitbuffer,1,0,(struct sockaddr *)&server_address,addrlen);
          setsockopt (mcsocketfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group, sizeof(group));
          close(mcsocketfd);
          close(socketfd);
          close(tcp_socketfd);
          exit(1);
        }
        else {
          printf("Please give a proper command. (/lock /score /quit /name /msg)\n");
        }  
        fflush(stdin);      
      }

/////////////////////// TCP

      if(FD_ISSET(tcp_socketfd, &RecFd)) { // this checks if anything happens in TCP socket
        
        sleep(2);
        
        //printf("TCP_SOCKET triggered.\n");
        
        // Lets peek inside so we get the length of the incoming message
        if ((nbytes = recv(tcp_socketfd, tcppeekbuffer, sizeof(int), MSG_PEEK)) <= 0){
        
					// got error or connection closed by server
					if(nbytes == 0){
						// connection closed 
						printf("Server closed TCP socket.\n");
					} 
					else{
						perror("recv() error");
					} 
					// might aswell shut down
					printf("Server has been shut down and/or connection has been severed.\nShutting down client.\n");
					close(socketfd);
					close(mcsocketfd);
					close(tcp_socketfd);
					exit(1);
        }
        else{
               
		      byte_count = ntohl(*(uint32_t*)&tcppeekbuffer[0]);

		      //printf("Peeked length: %d \n",byte_count);

		      recv(tcp_socketfd, tcpbuffer, byte_count, 0); // now lets take the amount we just peeked

	////////// PLAYERMESSAGE
		      if (tcpbuffer[4] == 41){ 
		        // lets print it
		        //printf("We has tcp msg: ");
		        for (e=6;e<(tcpbuffer[5]+6);e++){
		          printf("%c",tcpbuffer[e]);
		        }
		        printf(": ");
		        for (e=(6+tcpbuffer[5]);e<byte_count;e++){
		          printf("%c",tcpbuffer[e]);
		        }
		      }

	////////// SEL_NAME
		      if (tcpbuffer[4] == 42){ 
		               
		        printf("Server wants your name. Please type it WITHOUT any command (NO /name) and press enter.\n");

		        fgets (message , BUFFLEN , stdin);

		        // Need to find the end of the message
		        for (e=0;e<BUFFLEN;e++){
		          if (message[e] == '\0'){
		            whereistheend = e;
		            e = 2000;
		          }
		        }

		        // clearing buffer
		        memset(&chatmessage,'\0',BUFFLEN);

		        // Lets also store name in client
		        memset(&player_name,'\0',50); // resetting name

		        // need to copy the contents
		        for (e=0;e<(whereistheend);e++){
		          chatmessage[e+5] = message[e]; // +5 to leave room for msglength and message id
		          player_name[e] = message[e];
		        }

		        //printf("You typed: %s!\n",chatmessage);

		        // NAME_SEL to server

		        msglength = 0;
		        msglength = whereistheend +5; 

		        chatmessage[0] = msglength;          

		        uint8_t nameselint = 43;
		        chatmessage[4] = nameselint;

		        send(tcp_socketfd,chatmessage,msglength,0);

		        whereistheend = 0;
		        memset(&message,'\0',BUFFLEN);
		        memset(&chatmessage,'\0',BUFFLEN);
		           
		      }

	////////// NAME_OK
		      if (tcpbuffer[4] == 44){ 
		        printf("Name selection was successful.\nYour name is now %s.\n",player_name);
		      }

	////////// GOT_TO_TOP
		      if (tcpbuffer[4] == 45){ 
		        printf("You got a TOP10 score!\n");
		        printf("Your position is %d out of %d.\n",tcpbuffer[5],ntohs(*(uint16_t*)&tcpbuffer[6]));
		      }

	////////// TOP10
		      if (tcpbuffer[4] == 45){ 
		        printf("Here is the complete list of TOP10 players.\n");
		        n = 1;
		        
		        // parsing that TOP10 message is painful

		        uint8_t playernamelen = 5;

		        for (e=5;e<byte_count;e++){ 
		          if (e == (playernamelen)){
		            printf("\n%d: ",n);
		            n++;
		            playernamelen = playernamelen + tcpbuffer[e] +1;
		          }
		          else{
		            printf("%c",tcpbuffer[e]);
		          }
		        }
		        // TOP10 may as well be QUIT, no need to spam UDP sockets.
		        printf("\nGame has ended, shutting client down.\n");
		        // Maybe leave MC group?
		        close(socketfd);
		        close(tcp_socketfd);
		        close(mcsocketfd);
		        exit(1);
		      }
		    }
		  }

/////////////////////// MULTICAST

      if(FD_ISSET(mcsocketfd, &RecFd)) {

        //printf("MC triggered.\n");
        
        // so we wont have anything lingering there
        for (e=0;e<BUFFLEN;e++){
          recmessage[e] = '\0';
        }

        length = recvfrom(mcsocketfd,recmessage,BUFFLEN,0,(struct sockaddr *)&multicast_local_address,&multicast_local_addrlen);
        
        
        mc_what_message = recmessage[0];
        
        //printf("We got MC message: %u\n",mc_what_message);
        
////////////// START_GAME    
        if (mc_what_message == 30){
        
          // then lets ask for some dices
          
          rollbuffer[0] = rollint;  
      
          // Sending data to server
          length = sendto(socketfd,rollbuffer,1,0,(struct sockaddr *)&server_address,addrlen);
          mc_what_message = 0; 
          round++;
        
        }
////////////// NEXT_ROUND
        if (mc_what_message == 31){
        
          // then lets ask for some dices
          
          rollbuffer[0] = rollint;  
      
          // Sending data to server
          length = sendto(socketfd,rollbuffer,1,0,(struct sockaddr *)&server_address,addrlen);        
          mc_what_message = 0;
          round++;
        
        }
////////////// STOP_GAME
        if (mc_what_message == 32){
        
          printf("Client got QUIT message from server, shutting down.\n");
          close(mcsocketfd);
          close(socketfd);
          close(tcp_socketfd);
          exit(1);        
        }        
        
////////////// PLAYER_LEFT
        if (mc_what_message == 33){
        
		      for (e=0;e<=6;e++){
		        printf("%c",recmessage[e+3]);
		      }
		      printf(" has left the game.\n");
 
        
        } 
////////////// PLAYER_JOIN
        if (mc_what_message == 34){
        
          for (e=0;e<=6;e++){
		        printf("%c",recmessage[e+3]);
		      }
		      printf(" has joined the game.\n");
        
        }        
////////////// SCORESHEETS
        if (mc_what_message == 35){
        
		      // we be getting big message, need to cut it to little pieces
		      // Lets only display our own scores, Dont care about others.
		      // Need to find the spot where our stuff begins
		      // its either 2-3, 23-24, 44-45 or 65-66 

		      int awesomehelp = 0;

          uint16_t helpsson1 = ((uint16_t) recmessage[2] << 8) | recmessage[3];
          uint16_t helpsson2 = ((uint16_t) recmessage[23] << 8) | recmessage[24];
					uint16_t helpsson3 = ((uint16_t) recmessage[44] << 8) | recmessage[45];
					uint16_t helpsson4 = ((uint16_t) recmessage[65] << 8) | recmessage[66];

		      if (player_id == helpsson1){
		        awesomehelp = 4;
		      }
		      if (player_id == helpsson2){
		        awesomehelp = 25;
		      }
		      if (player_id == helpsson3){
		        awesomehelp = 46;
		      }
		      if (player_id == helpsson4){
		        awesomehelp = 67;
		      }
		      
					// Need to score own score in container as we print this now
		      // We have a problem here as we got those chars mixed up with 8bit integers

		      awesomehelp = awesomehelp -1;

		      if (recmessage[awesomehelp+1] == 'a') { // Ones
		        printf("Ones: a ");
		        myscore[0]=recmessage[awesomehelp+1];
		      }
		      else {
		        ones = recmessage[awesomehelp+1];
		        printf("Ones: %u ",ones);
		        myscore[0]=recmessage[awesomehelp+1];
		      }
		      if (recmessage[awesomehelp+2] == 'a') { // Twos
		        printf("Twos: a ");
		        myscore[1]=recmessage[awesomehelp+2];
		      }
		      else {
		        twos = recmessage[awesomehelp+2];
		        printf("Twos: %u ",twos);
		        myscore[1]=recmessage[awesomehelp+2];
		      }
		      if (recmessage[awesomehelp+3] == 'a') { // Threes
		        printf("Threes: a ");
		        myscore[2]=recmessage[awesomehelp+3];
		      }
		      else {
		        threes = recmessage[awesomehelp+3];
		        printf("Threes: %u ",threes);
		        myscore[2]=recmessage[awesomehelp+3];
		      }
		      if (recmessage[awesomehelp+4] == 'a') { // Fours
		        printf("Fours: a ");
		        myscore[3]=recmessage[awesomehelp+4];
		      }
		      else {
		        fours = recmessage[awesomehelp+4];
		        printf("Fours: %u ",fours);
		        myscore[3]=recmessage[awesomehelp+4];
		      }
		      if (recmessage[awesomehelp+5] == 'a') { // Fives
		        printf("Fives: a ");
		        myscore[4]=recmessage[awesomehelp+5];
		      }
		      else {
		        fives = recmessage[awesomehelp+5];
		        printf("Fives: %u ",fives);
		        myscore[4]=recmessage[awesomehelp+5];
		      }
		      if (recmessage[awesomehelp+6] == 'a') { // Sixes
		        printf("Sixes: a ");
		        myscore[5]=recmessage[awesomehelp+6];
		      }
		      else {
		        sixes = recmessage[awesomehelp+6];
		        printf("Sixes: %u ",sixes);
		        myscore[5]=recmessage[awesomehelp+6];
		      }
		      if (recmessage[awesomehelp+7] == 'a') { // Threeofakind
		        printf("Three-of-a-kind: a ");
		        myscore[6]=recmessage[awesomehelp+7];
		      }
		      else {
		        threeofakind = recmessage[awesomehelp+7];
		        printf("Three-of-a-kind: %u ",threeofakind);
		        myscore[6]=recmessage[awesomehelp+7];
		      }
		      if (recmessage[awesomehelp+8] == 'a') { // Fourofakind
		        printf("Four-of-a-kind: a ");
		        myscore[7]=recmessage[awesomehelp+8];
		      }
		      else {
		        fourofakind = recmessage[awesomehelp+8];
		        printf("Four-of-a-kind: %u ",fourofakind);
		        myscore[7]=recmessage[awesomehelp+8];
		      }
		      if (recmessage[awesomehelp+9] == 'a') { // Fullhouse
		        printf("Full House: a ");
		        myscore[8]=recmessage[awesomehelp+9];
		      }
		      else {
		        fullhouse = recmessage[awesomehelp+9];
		        printf("Full House: %u ",fullhouse);
		        myscore[8]=recmessage[awesomehelp+9];
		      }
		      if (recmessage[awesomehelp+10] == 'a') { // Smallstraight
		        printf("Small straight: a ");
		        myscore[9]=recmessage[awesomehelp+10];
		      }
		      else {
		        lowstraight = recmessage[awesomehelp+10];
		        printf("Small straight: %u ",lowstraight);
		        myscore[9]=recmessage[awesomehelp+10];
		      }
		      if (recmessage[awesomehelp+11] == 'a') { // Largestraight
		        printf("Large straight: a ");
		        myscore[10]=recmessage[awesomehelp+11];
		      }
		      else {
		        highstraight = recmessage[awesomehelp+11];
		        printf("Large straight: %u ",highstraight);
		        myscore[10]=recmessage[awesomehelp+11];
		      }
		      if (recmessage[awesomehelp+12] == 'a') { // Yahtzee
		        printf("Yahtzee: a ");
		        myscore[11]=recmessage[awesomehelp+12];
		      }
		      else {
		        yahtzee = recmessage[awesomehelp+12];
		        printf("Yahtzee: %u ",yahtzee);
		        myscore[11]=recmessage[awesomehelp+12];
		      }
		      if (recmessage[awesomehelp+13] == 'a') { // Chance
		        printf("Chance: a ");
		        myscore[12]=recmessage[awesomehelp+13];
		      }
		      else {
		        chance = recmessage[awesomehelp+13];
		        printf("Chance: %u ",chance);
		        myscore[12]=recmessage[awesomehelp+13];
		      }
		      
		      uppertotal = ((uint16_t) recmessage[awesomehelp+14] << 8) | recmessage[awesomehelp+15]; // bitshifting and casting to 16bit
		      myscore[13]=uppertotal;
		      lowertotal = ((uint16_t) recmessage[awesomehelp+16] << 8) | recmessage[awesomehelp+17]; // bitshifting and casting to 16bit
		      myscore[15]=lowertotal;
		      totaltotal = ((uint16_t) recmessage[awesomehelp+18] << 8) | recmessage[awesomehelp+19]; // bitshifting and casting to 16bit
		      myscore[17]=totaltotal;

		      printf("Upper total: %u Lower total: %u Total: %u \n\n",uppertotal,lowertotal,totaltotal);
	 
		      
        }  
////////////// HELLO
        if (mc_what_message == 36){
        
        
		      printf("%c%c%c%c%c\n",recmessage[1],recmessage[2],recmessage[3],recmessage[4],recmessage[5]);
		          
		      cheersbuffer[0] = cheersint;
		         
		      cheersbuffer[1] = 'C';
		      cheersbuffer[2] = 'H';
		      cheersbuffer[3] = 'E';
		      cheersbuffer[4] = 'E';
		      cheersbuffer[5] = 'R';
		      cheersbuffer[6] = 'S';
		      cheersbuffer[7] = '\0';
	 
		      length = sendto(mcsocketfd,cheersbuffer,8,0,(struct sockaddr *)&multicast_group_address,multicast_group_addrlen);
        
        } 
////////////// CHEERS
        if (mc_what_message == 37){
        
		      int theend = 0;
		      
		      for (e=0;e<20;e++){
		        if (recmessage[e] == '\0'){
		          theend = e;
		          e=21;
		        }
		      }
		      for (e=1;e<=theend-1;e++){
						printf("%c",recmessage[e]);
		      }        
          printf("\n");
        
        }                  
      } 

///////////////////////////////////////
/////////////////UDP triggered
/////////////////////////////////////

      if(FD_ISSET(socketfd, &RecFd)) { // this checks if anything happens in socket
   
        // so we wont have anything lingering there
        for (e=0;e<BUFFLEN;e++){
          recmessage[e] = '\0';
        }

        // getting a message
        length = recvfrom(socketfd,recmessage,BUFFLEN,0,(struct sockaddr *)&client_address,&addrlen);
        
        if (length <0) { printf("Recvfrom error.\n");}

        // This is used to see the 1st byte of the message which always contains the message type id according to protocol
        what_message = recmessage[0];

        //printf("\nSaatii viesti ID: %d\n",what_message);
        //fflush(stdout);

 

//////////////////////////////////////////////////
////////// We got DICES message from server.
//////////////////////////////////////////////////

        if (what_message == 4){
          
          printf("We got rolled dices from server!\n");
          dice1 = recmessage[1];
          dice2 = recmessage[2];
          dice3 = recmessage[3];
          dice4 = recmessage[4];
          dice5 = recmessage[5];

          printf("You got:\nDice 1: %d\nDice 2: %d\nDice 3: %d\nDice 4: %d\nDice 5: %d\n",dice1,dice2,dice3,dice4,dice5);
          fflush(stdout);
          printf("Choose what you want to so (/lock, /score /quit).\nNow write your command and press ENTER.\n");
          fflush(stdout);
          // now this goes to wait for either of the filedescriptors to trigger (FD_ISSETs)
          
          // NEXT we must answer the server before the server gives us NEXT_ROUND
          
          what_message = 0;
        }

//////////////////////////////////////////////////
////////// We got POSSIBLE message from server.
//////////////////////////////////////////////////

        if (what_message == 10){
        
        
	        // Need to find the end of the message
	        for (e=0;e<BUFFLEN;e++){
	          if (message[e] == '\0'){
	            whereistheend = e;
	          }
	        }        
          
          printf("Your scoreable dices are: %d %d %d %d %d\n",dice1,dice2,dice3,dice4,dice5);
          printf("Available positions to score:\n");
          
          // prints all the available positions
          for (e=2;e<whereistheend;e++){
            if (recmessage[e] == 1){
              printf("1:Ones ");
            }
            if (recmessage[e] == 2){
              printf("2:Twos ");
            }
            if (recmessage[e] == 3){
              printf("3:Threes ");
            }
            if (recmessage[e] == 4){
              printf("4:Fours ");
            }
            if (recmessage[e] == 5){
              printf("5:Fives ");
            }
            if (recmessage[e] == 6){
              printf("6:Sixes ");
            }
            if (recmessage[e] == 7){
              printf("7:Three-of-a-kind ");
            }
            if (recmessage[e] == 8){
              printf("8:Four-of-a-kind ");
            }
            if (recmessage[e] == 9){
              printf("9:Full House ");
            }
            if (recmessage[e] == 10){
              printf("10:Low Straigth ");
            }
            if (recmessage[e] == 11){
              printf("11:High straigth ");
            }
            if (recmessage[e] == 12){
              printf("12:Yahtzee ");
            }
            if (recmessage[e] == 13){
              printf("13:Chance ");
            }
          }
          
          printf("\nGive /position X where X is the number of the field you wish to score.\n");

          // takes position from STDIN and sends it to server
          fgets (message , 16 , stdin);
          if (strncmp(message, "/position ", 10) == 0) {

            setscorebuffer[0] = setscoreint;
            setscorebuffer[1] = round;
            setscorebuffer[2] = atoi(&message[10]);

            // Sending data to server
            length = sendto(socketfd,setscorebuffer,3,0,(struct sockaddr *)&server_address,addrlen);  
                    
					}
        }

//////////////////////////////////////////////////
////////// We got POSITION_SET message from server.
//////////////////////////////////////////////////

        if (what_message == 12){
        
          //printf("We got POSITION SET: %d %d %d %d\n",recmessage[0],recmessage[1],recmessage[2],recmessage[3]);

          // this just prints out what we scored and howmany points we got
          if (recmessage[2] == 1){
            printf("We scored aces: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 2){
            printf("We scored twos: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 3){
            printf("We scored threes: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 4){
            printf("We scored fours: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 5){
            printf("We scored fives: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 6){
            printf("We scored sixes: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 7){
            printf("We scored 3-of-a-kind: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 8){
            printf("We scored 4-of-a-kind: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 9){
            printf("We scored full house: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 10){
            printf("We scored small straight: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 11){
            printf("We scored large straight: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 12){
            printf("We scored yahtzee: %d points.\n",recmessage[3]);
          }
          if (recmessage[2] == 13){
            printf("We scored chance: %d points.\n",recmessage[3]);
          }
        }

//////////////////////////////////////////////////
////////// We got QUIT message from server.
//////////////////////////////////////////////////

        if (what_message == 20){
          printf("Client got QUIT message from server, shutting down.\n");
          close(socketfd);
          close(mcsocketfd);
          exit(1);        
        }

//////////////////////////////////////////////////
////////// We got an ERROR or GAME_FULL message from server.
//////////////////////////////////////////////////

        if (what_message == 21){
          
          if (recmessage[1] == 1){
		        printf("Game is full. Please try again later.\n");
						printf("Client shutting down.\n");
		        close(socketfd);
		        close(mcsocketfd);
            close(tcp_socketfd);
		        exit(1);
          }
          if (recmessage[1] == 2){
		        printf("Game is currently running. Please try again later.\n");
						printf("Client shutting down.\n");
		        close(socketfd);
		        close(mcsocketfd);
            close(tcp_socketfd);
		        exit(1);
          }
          if (recmessage[1] == 3){
		        printf("Position is not valid.\nPlease select another position.\n");
            int y = 0;
            while (y==0){
		          fgets (message , 16 , stdin);
			        if (strncmp(message, "/position ", 10) == 0) {
                y=1;
		          }
							else {
                printf("You must give a new position.\nType /position X where X is the number of the position you wish to choose.\n");
              }
            } 
            setscorebuffer[0] = setscoreint;
            setscorebuffer[1] = round;
            setscorebuffer[2] = atoi(&message[10]);

            // Sending data to server
            length = sendto(socketfd,setscorebuffer,3,0,(struct sockaddr *)&server_address,addrlen); 
                     
					}            
        }
      }
      FD_ZERO(&RecFd); // resetting, getting new ones at top from master
    } // while loop ends.
    //close(socketfd); // just incase we left socket open
    //close(tcp_socketfd);
    //close(mcsocketfd);
  }
  else {
    printf("Invalid port. Choose something between 1025 - 65000\n");
    //close(socketfd);
    return -1;
  }
  //close(socketfd);
  return 0;
}

// EOF
