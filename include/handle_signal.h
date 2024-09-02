#ifndef CHESS_SIGNAL_H
#define CHESS_SIGNAL_H

#include <signal.h>

#ifdef CHESS_WINDOWS_VERSION
	#define INIT_SIGNAL_HANDLER(handle_func) init_signal_handler_windows(handle_func)
#else
	#define INIT_SIGNAL_HANDLER(handle_func) init_signal_handler_posix(handle_func)
#endif

typedef void (*signal_handle_func)(int);

int		init_signal_handler_posix(signal_handle_func func);
int		init_signal_handler_windows(signal_handle_func func);

#endif /* CHESS_SIGNAL_H */