#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/handle_signal.h"
#include "../include/chess_log.h"

static void signal_handler(int signum)
{
	SDLHandle *stat = get_SDL_handle();

	CHESS_LOG(LOG_DEBUG, RED"\nSignal Catch: %d\n"RESET, signum);
	chess_destroy(stat);
	exit(signum);
}

#ifdef CHESS_WINDOWS_VERSION
	int init_signal_handler_windows(void)
	{
		if (signal(SIGINT, signal_handler) == SIG_ERR){
			perror("Can't catch SIGINT");
			return (FALSE);
		}
		if (signal(SIGTERM, signal_handler) == SIG_ERR){
			perror("Can't catch SIGTERM");
			return (FALSE);
		}
		return (TRUE);
	}
#else
	int init_signal_handler_posix(void)
	{
		struct sigaction sa;

		sa.sa_handler = signal_handler;
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
		return (TRUE);
	}
#endif