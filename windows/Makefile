WCC         = x86_64-w64-mingw32-gcc

W_SDL2      = -L../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/lib -I../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/include

W_FLAGS     = -Wall -lmingw32 -lSDL2main -lSDL2 -lws2_32

# Définir les répertoires de sources
SRC_DIRS    = ../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string ../src

# Trouver tous les fichiers .c dans les répertoires de sources et leurs sous-dossiers
SRCS        = $(shell find $(SRC_DIRS) -name '*.c')

NAME        = WinCchess

$(NAME):    
			@echo "\033[5;36m ----- Compiling $(NAME) window version...  ----- \033[0m\n"
			@${UPDATE_BIN_DIR}
			@${WCC} $(SRCS) $(W_SDL2) $(W_FLAGS) -DCHESS_WINDOWS_VERSION -o $(NAME)
			@echo "\033[7;32m -----  Compiling $(NAME) done  ----- \033[0m\n"

clean:
			@echo "\033[7;31m\n -----  Cleaning all objects...  ----- \033[0m\n"
			@${RM} ${NAME}.exe
			@echo "\033[7;33m -----  Cleaning done  ----- \033[0m\n"

fclean:     clean

re:         fclean all