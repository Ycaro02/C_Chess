
#include "../include/network.h"

#ifdef CHESS_WINDOWS_VERSION
	int init_network_windows() {
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			return 1;
		}
		return (0);
	}

	void cleanup_network_windows() {
		WSACleanup();
	}

	s8 socket_no_block_windows(Socket sockfd, struct timeval timeout) {
		u_long mode = 1;
		(void)timeout;
		if (ioctlsocket(sockfd, FIONBIO, &mode) != 0) {
			perror("ioctlsocket failed");
			CLOSE_SOCKET(sockfd);
			return (FALSE);
		}
		return (TRUE);
	}

#else

	int init_network_posix() {
		return (0); // No initialization needed for POSIX
	}

	void cleanup_network_posix() {
		// No cleanup needed for POSIX
	}

	s8 socket_no_block_posix(Socket sockfd, struct timeval timeout) {
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
			perror("Error setting socket timeout");
			CLOSE_SOCKET(sockfd);
			return (FALSE);
		}
		return (TRUE);
	}

#endif /* CHESS_WINDOWS_VERSION */