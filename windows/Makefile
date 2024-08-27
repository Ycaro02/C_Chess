include ../libft/rsc/mk/color.mk

WCC         = x86_64-w64-mingw32-gcc

W_SDL2      = -L../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/lib -I../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/include

W_FLAGS     = -Wall -lmingw32 -lSDL2main -lSDL2 -lws2_32

# Définir les répertoires de sources
SRC_DIRS    = ../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string ../src

# Trouver tous les fichiers .c dans les répertoires de sources et leurs sous-dossiers
SRCS        = $(shell find $(SRC_DIRS) -name '*.c')

NAME        = WinCchess.exe

all:	$(NAME)

$(NAME):    
			@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
			@${UPDATE_BIN_DIR}
			@${WCC} $(SRCS) $(W_SDL2) $(W_FLAGS) -DCHESS_WINDOWS_VERSION -o $(NAME)
			@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"


update: $(NAME)
	@printf "$(CYAN)Updating windows version in /media/ycaro/Data/C_chess_win ${NAME} ...$(RESET)\n"
	@cp -r * /media/ycaro/Data/C_chess_win
	@printf "$(GREEN)Update done $(NAME) done$(RESET)\n"

connect: $(NAME)
	wine $(NAME) -j -n -i 127.0.0.1


clean:
			@${RM} ${NAME}
			@printf "$(RED)Clean $(NAME)$(RESET)\n"

fclean:     clean

re:         fclean all