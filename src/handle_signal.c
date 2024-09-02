#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/handle_signal.h"
#include "../include/chess_log.h"

#ifdef CHESS_WINDOWS_VERSION
	int init_signal_handler_windows(signal_handle_func func)
	{
		if (signal(SIGINT, func) == SIG_ERR){
			perror("Can't catch SIGINT");
			return (FALSE);
		}
		if (signal(SIGTERM, func) == SIG_ERR){
			perror("Can't catch SIGTERM");
			return (FALSE);
		}
		return (TRUE);
	}
#else
	int init_signal_handler_posix(signal_handle_func func)
	{
		struct sigaction sa;

		sa.sa_handler = func;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);

		/* Handle SIGINT */
		if (sigaction(SIGINT, &sa, NULL) == -1) {
			perror("Can't catch SIGINT");
			return (FALSE);
		}
		/* Handle SIGTERM */
		if (sigaction(SIGTERM, &sa, NULL) == -1) {
			perror("Can't catch SIGTERM");
			return (FALSE);
		}

		/* Handle SIGQUIT */
		if (sigaction(SIGQUIT, &sa, NULL) == -1) {
			perror("Can't catch SIGQUIT");
			return (FALSE);
		}

		/* Handle SIGHUP */
		if (sigaction(SIGHUP, &sa, NULL) == -1) {
			perror("Can't catch SIGHUP");
			return (FALSE);
		}

		/* Handle SIGSEV */
		if (sigaction(SIGSEGV, &sa, NULL) == -1) {
			perror("Can't catch SIGSEGV");
			return (FALSE);
		}
		
		/* Handle SIGTERM */
		if (sigaction(SIGTERM, &sa, NULL) == -1) {
			perror("Can't catch SIGTERM");
			return (FALSE);
		}

		return (TRUE);
	}
#endif