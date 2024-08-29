#!/bin/bash

LOG_DIR="rsc/log"

SERVER_LOG="${LOG_DIR}/server.log"
CLIENT_1_LOG="${LOG_DIR}/client1.log"
CLIENT_2_LOG="${LOG_DIR}/client2.log"

CHESS="C_Chess"
SERVER="chess_server"

if [[ ! -f ${CHESS} || ! -f ${SERVER} ]] ; then
	printf "Building...\n"
	make -s -j 1>/dev/null
fi

mkdir -p ${LOG_DIR}

./chess_server > ${SERVER_LOG} &
SERVER_PID=$!

./C_Chess -n -l -i 127.0.0.1 > ${CLIENT_1_LOG} &
CLIENT_1_PID=$!

./C_Chess -n -j -i 127.0.0.1 > ${CLIENT_2_LOG} &
CLIENT_2_PID=$!

wait ${CLIENT_1_PID}
wait ${CLIENT_2_PID}
kill ${SERVER_PID}

cat ${SERVER_LOG}