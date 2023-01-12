#!/bin/bash
# This script is used to compile the client and server programs
gcc -w client/Client.c client/ClientLib.c -o Client
gcc -w server/Server.c server/ServerLib.c -o Server