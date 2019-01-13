//////////////////////////////////
//                              //
//  Network Programming         //
//  Assignment 4                //
//  Readme file                 //
//                              //
//  Kimmo Pietiläinen           //
//                              //
//////////////////////////////////

THIS IS THE BUGFIX VERSION.

------------------------------------------------------

What was fixed:

 -bug with POSSIBLE msg parsing in client
 -POSITION_SET message was entirely missing
 -score-function had multiple bugs and it works now
 -INVALID_POSITION message was entirely missing
 -Game now actually plays if no player leaves 
 -Server now tells clients if they have already scored and asks for another position
 -TCP peeks work now!
 -TCP messages are going well back and forth, tested with example, BUT I ran out of time to do the logic
    -> /name wont work 
    -> /msg <message> will send that message to all clients and the sender will always be Jeba  
 -Client side all TCP messages that client can recieve are being handled, its just my server that doesnt send 
  them right. So this client should in theory work with someone elses server.
  
Newly found horrors:
  
 -multicast schoresheets has something wonky going on
 
------------------------------------------------------

This is a readme file for yahtzeeclient and yahtzeeserver programs for assignment 4.

The whole assignment consist of 6 files: 

	yahtzeeclient.c 
	yahtzeeclient.h 
	yahtzeeserver.c
	yahtzeeserver.h
	makefile  
	readme.txt 

You can run the programs by first by unpacking the 0303036_kimmo_pietilainen.tar.gz file.
Then go to the folder it just made and type 'make' (or 'make build') to complile the code according to the 
makefile.

Then there should be 2 programs: yahtzeeclient and yahtzeeserver.

It is based on my work for assignment 1 and 2. Uses UDP, Multicast and TCP.

Read the Assignment 4 pages for more info on how this was supposed to be implemented.
http://www2.it.lut.fi/wiki/doku.php/courses/ct30a5000/assignment4

------ CLIENT -------

You can run the client by typing the following (in any argument order):

./yahtzeeclient -h <server address> -up <UDP port number> -tp <TCP port number> -m <multicast addr> -mp <multicast port>

Server address is the IPv4 number for the server and portnumber is the port for the server and the TCP port
number is the port for servers TCP listening port. Multicast addr is the multicast groups IPv4 address and 
multicast port is the port for the multicast group.

You should use ports between 1025-65000 and IPv4 for IP.

After client is started it automatically connects to the server. 
One could also /quit at this time and shut down the client.

Once user has connected it will get accepter msg from server and the game begins. At this point the player can
always choose to either /quit, /lock, /score, /scoresheet, /name and /msg.

/quit: sends QUIT message to server so that server can reset everything and be able to take in another player.

/lock <dices separated with commas>: For example /lock 1,2,3 would lock dices 1,2 and 3. Just sending /lock 
                                     will ask the server to roll all the dices without locking.

/score: Will ask server to score the current dices.

/scoresheet: Will show the players most current scoresheet.

/name <playername>: Will ask the server to allow you to use a playername given after /name command (Uses TCP).
                    This is also used after /msg if a player name hasn't been inputted before. Server will ask 
                    for it.
                    
/msg <chat message>: Chat message, that will be relayed to all players (uses TCP). If /name hasn't been used
                     before using this the server will ask the client to enter a playername.

The game goes on untill either player /quits or server /quits or players have reached the end of the game, which 
is 13 rounds.

For more info read assignment 4 course pages.

------ SERVER -------

You can run the server according to the following (in any argument order):

./yahtzeeserver -up <UDP port> -tp <TCP listening port> -m <multicast addr> -mp <multicast port>

Where UDP port is the port number the server listens to (1025-65000) and TCP listening port is the port that
server uses to listen to incoming TCP connections. Multicast addr is the multicast groups IPv4 address and 
multicast port is the port for the multicast group.

It will run and respond to the messages sent from the client according to the protocol given by the assignment.

After a client quits or has run out of rounds, server will reset everything and be ready to serve another client.
Server can serve only 1 client at a time.

(added OPTIONAL) Server will also take /quit command and it will send a STOP_GAME multicast message to 
the clients who were using it. After that it shuts down.

----- EXTRAS -------

-TOP10-message also serves as a QUIT, no need for spam.
-Server has a /quit keyboard command, when done (or when CTRL+C is pressed) the TCP connections will break.
 and the clients will detect it and also shut down.
-Compiles without warnings or errors.
-Valgrind has been run and no memory leaks.

----- STUFF that doesn't work or is INCOMPLETE: (Will try to fix in bugfix version) ------

- Does not have any timeouts, game will only start after 4 players connect. (NOT FIXED)
- Game logic is incomplete, crashes and bugs after 1st round. (PARTIALLY)
- TCP in server works as telnet echo (telnet <hostip> <port>), but doesnt check anything about what messages 
  have been sent or by who. It relays the text types in telnet to all other telnet clients that are connected. 
  (SOME IMPROVEMENT)(See top of readme)
- Chat needs to be sorted out and getting players names aswell. (NOT FIXED)
- Comments are still a mess and debug prints need to be ironed out. (FIXED)
- No IPv6 support. (NOT FIXED)
- Missing TOP10 implementation. As my TCP doesnt yet work so names don't work so this wont work. (NOT FIXED)

------ QUESTIONS / ANSWERS -------

1. Is there any limit for connection count when TCP is used?

	Yes, the limit is the number of available sockets. As each socket is tied to a port, so the safe maximum 
	amount of connections is 65535-1024 = 64511. This way no reserved ports are taken.

2. How to guarantee that all data is sent with TCP?

	TCP takes care of this. One just has to code it the right way so it is sent with TCP. Also need to make
	sure that everything from TCP stream is also read in the other end.

3. What are the limitations for messages to be sent to server over TCP? Is there a maximum length for packets 
   to be sent according to protocol? What about maximum length of a TCP packet?

	The maximum segment size (MMS) is the limit for message length for TCP in bytes. It's derived from 
	the maximum transmission unit (MTU) which is the limit for data link layer (routers etc.). TCP senders
	can use the path MTU discovery to discover the minimum MTU along the network path between the sender and
	reciever, and use this to dynamically adjust the MSS to avoid IP fragmentation.
	
	So the max length of a TCP packet would be MTU-EthernetHeader-IPHeader-TCPHeader (one could argue that 
	TCPHeader is part of the packet, but for data it is not)

------- REFERENCES --------

I discussed this assignment with Lauri Naukkarinen. He was able to help me with some C things.

THESE ARE FROM ASSIGNMENT 1 BUT I USED THEM FOR ASSIGNMENT 2 and 4 ALSO

Web:

http://www2.it.lut.fi/wiki/doku.php/courses/ct30a5000/start
All the course related stuff. Also makefile template and udpexample, which i used to get started.
Also help on how to tar.gz something.

http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
For many different problems with sockets/addresses and select().

http://www.wikihow.com/Compare-Two-Strings-in-C-Programming
Helped with comparing strings.

http://www.lix.polytechnique.fr/~liberti/public/computing/prog/c/C/CONCEPT/data_types.html
Helped with different data types and their sizes.

http://www.delorie.com/gnu/docs/glibc/libc_445.html
Help with sleep()

http://www.avrfreaks.net/index.php?name=PNphpBB2&file=printview&t=41718&start=0
http://cboard.cprogramming.com/c-programming/102936-uint16_t-two-uint_8-values.html
http://stackoverflow.com/questions/4007170/uint8-t-to-uint16-t
Helped with converting uint16_t to uint8_t. I used this to pack 16bit integers into 2x 8bit integers as the 8bit ones go straight into char buffer[spot].

http://www.cs.cf.ac.uk/Dave/C/node19.html
String handling

http://www.ohjelmointiputka.net/oppaat/opas.php?tunnus=cohj_3
Basic C stuff

http://en.wikipedia.org/wiki/Global_variable
Help with global variables

http://www.cprogramming.com/tutorial/bitwise_operators.html
Bitwise operations 

http://www.cplusplus.com/reference/clibrary/cstdlib/rand/
Helped with randomizing numbers.

http://stackoverflow.com/questions/5360660/using-select-system-call-for-listening-on-stdin-and-the-server
Select() help

http://cboard.cprogramming.com/networking-device-communication/114459-multiplexing-read-socket-stdin.html
Helped with stdin and socket multiplexing

http://www.cplusplus.com/reference/clibrary/cstdio/fgets/
Helped with fgets()

http://pubs.opengroup.org/onlinepubs/007904875/functions/fflush.html
fflush help

http://www.gamedev.net/topic/310343-udp-client-server-echo-example/
Basic client-server example.

ASSIGNMENT 2 SPECIFIC:
Web:

http://www.tenouk.com/Module41c.html
simple multicast client/server example

http://www.ohjelmointiputka.net/oppaat/opas.php?tunnus=cohj_2
Some more basic C

http://www.tldp.org/HOWTO/Multicast-HOWTO-6.html
Loads of information about multicast in general.

http://en.wikipedia.org/wiki/Struct_%28C_programming_language%29
Some help with structs.

http://www2.it.lut.fi/wiki/doku.php/courses/ct30a5000/assignment1notes
Notes from 1st assignment.

http://www.java2s.com/Code/C/Structure/Asimplemailinglistexampleusinganarrayofstructures.htm
Some help with array structs.

http://www.network-theory.co.uk/docs/gccintro/gccintro_94.html
Compiler help.

http://publib.boulder.ibm.com/infocenter/iseries/v5r3/topic/apis/ssocko.htm
setsockopt() help

http://pubs.opengroup.org/onlinepubs/009695399/functions/inet_addr.html
Help with inet_addr

http://tack.ch/multicast/
simple multicast client/server example

and ofcourse many linux man pages.

ASSIGNMENT 4 SPECIFIC:
Web:

http://www2.it.lut.fi/wiki/doku.php/courses/ct30a5000/assignment4
Course pages.

http://beej.us/guide/bgnet/output/html/multipage/advanced.html#select
Beejs pages. This helped alot with the TCP select and new connections and sockets.

http://www.tenouk.com/Module41a.html
http://www.tenouk.com/Module41.html
http://www.tenouk.com/Module40c.html
There 3 are small client/server socket example codes that helped me understand alot.

http://en.wikipedia.org/wiki/Transmission_Control_Protocol
Basic TCP stuff.

http://unixhelp.ed.ac.uk/tables/telnet_commands.html
I used telnet to test TCP so needed some help with it.

EOF
