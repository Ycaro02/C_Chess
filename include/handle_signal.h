#ifndef CHESS_SIGNAL_H
#define CHESS_SIGNAL_H

#include <signal.h>

#ifdef CHESS_WINDOWS_VERSION
	#define INIT_SIGNAL_HANDLER() init_signal_handler_windows()
#else
	#define INIT_SIGNAL_HANDLER() init_signal_handler_posix()
#endif


int		init_signal_handler_posix(void);
int		init_signal_handler_windows(void);

#endif /* CHESS_SIGNAL_H */