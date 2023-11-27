# BMP
High-Performance File Transferring System

gcc server_mssc.c -o server && ./server
<br>
gcc client_mssc.c -o client && ./client
<br>
gcc server_msmc.c -pthread -lm -o server && ./server
<br>
gcc client_msmc.c -pthread -lm -o client && ./client
<br>
gcc server_worker.c -pthread -lm -o worker && ./worker