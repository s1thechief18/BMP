# User Manual
High-Performance File Transferring System

## Single Server Multiple Socket Commands
&emsp; gcc server_mssc.c -o server && ./server
<br>
&emsp; gcc client_mssc.c -o client && ./client
<br>

## Multi Server Single Socket Commands
&emsp; gcc server_msmc.c -pthread -lm -o server && ./server
<br>
&emsp; gcc client_msmc.c -pthread -lm -o client && ./client
<br>
&emsp; gcc server_worker.c -pthread -lm -o worker && ./worker

## Note:
This project will work only for linux based machines

<hr>

### Step 1:
Download project to all machines.
You can either download zip file of the project or use "git clone https://github.com/xxS1xx/BMP.git" in the terminal after changing directory to target folder

<img width="576" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/049a0953-bc84-49ab-a219-b196261de57f">


### Step 2:
Set all parameters according to your need in "config2.h" file

<img width="569" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/46d34d6e-23c7-44f1-975f-ddcfaa08e97a">

### Step 3:

Start server <br>
Command: "gcc server_msmc.c -pthread -lm -o server && ./server"

<img width="1054" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/6137704e-60dd-4f0f-a48c-c7a2b3f83e72">

### Step 4:

Start all workers <br>
Command: "gcc server_worker.c -pthread -lm -o worker && ./worker"

<figure>
  <img width="1030" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/8f0f15e5-0d62-4ebe-8874-f57670c42c73">
  <figcaption>Fig.1 - Worker</figcaption>
  <img width="1047" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/657e039c-6b51-4675-9fed-7d7cb9ab2a15">
  <figcaption>Fig.2 - Server</figcaption>
</figure>

### Step 5:

Connect client to server <br>
Command: "gcc client_msmc.c -pthread -lm -o client && ./client"

<figure>
  <img width="1037" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/191057b0-db00-44b4-91c6-ccc7eec212bf">
  <figcaption>Fig.1 - Client</figcaption>
  <img width="1115" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/dc5e2f6c-7d29-42a2-ae40-90b1f198ae88">
  <figcaption>Fig.2 - Server</figcaption>
</figure>

### Step 6

Enter filename to be sent

<figure>
  <img width="1086" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/cf9e4d92-9f3c-4892-8015-d3949270c14e">
  <figcaption>Fig.1 - Client</figcaption>
</figure>

### Step 7

Wait for file to get transfered successfully

<figure>
  <img width="1041" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/845352d9-de62-45fe-9df2-06299ee3b073">
  <figcaption>Fig.1 - Server</figcaption>
  <img width="1035" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/b162e016-564a-4109-a3d5-b554f97114d4">
  <figcaption>Fig.2 - Worker</figcaption>
  <img width="1106" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/8e222cbd-a1cb-4cae-bb1b-07b4553a139f">
  <figcaption>Fig.3 - Client</figcaption>
  <img width="1138" alt="image" src="https://github.com/xxS1xx/BMP/assets/123160190/ca3c6cb2-fc68-41f2-a59d-22caac1400ad">
  <figcaption>Fig.4 - Output Files</figcaption>
</figure>

<hr>

If you have any queries then reach me out at "s1thechief18@gmail.com"













