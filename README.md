# Bank System
By: Gabriel Gutierrez (gg548)
      Ayush Joshi (aj470)
#Description:

-	Our bank system uses multi-process design. 
-	It uses fork() to create child processes for each new client connection. 
-	Parent process waits on all child processes before shutting down the server.
-	Implements semaphores as a lock for commands.
-	Server prints out account log every 20 seconds.
-	Handles up to 20 clients.
-	2 second throttle between commands
-	Uses mmap to create map for shared data (accounts) between processes.

