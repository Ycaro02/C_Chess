#!/bin/bash

gcc test_server.c -o server_chess && gcc client.c -o client_chess

# clean rule

if [ "$1" == "clean" ]; then
	rm server_chess client_chess
elif [ "$1" == "run" ]; then
	./server_chess
fi