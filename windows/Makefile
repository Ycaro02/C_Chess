include ../libft/rsc/mk/color.mk

WCC         = x86_64-w64-mingw32-gcc

W_SDL2      = -L../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/lib -I../rsc/lib/win_lib/SDL2-2.30.5/x86_64-w64-mingw32/include

W_SDL2_TTF = -L../rsc/lib/win_lib/SDL2_ttf-2.22.0/x86_64-w64-mingw32/lib -I../rsc/lib/win_lib/SDL2_ttf-2.22.0/x86_64-w64-mingw32/include

W_CURL		= -L../rsc/lib/win_lib/curl_lib/lib -I../rsc/lib/win_lib/curl_lib/include 

W_FLAGS     = -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lws2_32 -lcurl

# NO_TERMINAL = -mwindows
NO_TERMINAL =

SRC_DIRS    = ../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string ../src

# Find all source files
SRCS        = $(shell find $(SRC_DIRS) -name '*.c')

SERVER_SRC_DIRS =	../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string

SERVER_SRC_DEPS	=	$(shell find $(SERVER_SRC_DIRS) -name '*.c')

SERVER_SRC		=	../server/server.c ../src/network_os.c ../src/handle_signal.c ../src/move_save.c ../src/chess_log.c ../src/handle_reconnect.c -DCHESS_SERVER

SERVER_FLAGS	=	-Wall -lmingw32 -lws2_32 -DCHESS_WINDOWS_VERSION

SERVER_EXE		=	WinChessServer.exe

NAME        = WinCchess.exe

# Resource iconefile
RESOURCE    = rsc/resource.rc
RESOURCE_OBJ = resource.o

all:	$(NAME)

CACERT_URL = https://curl.se/ca/cacert.pem
CACERT_FILE = ./rsc/cacert.pem

$(CACERT_FILE):
	@echo "Downloading cacert.pem..."
	@wget $(CACERT_URL) -O $(CACERT_FILE)

$(NAME): $(SERVER_EXE) $(CACERT_FILE) $(RESOURCE_OBJ)
ifeq ($(shell [ -f ${NAME} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@${UPDATE_BIN_DIR}
	@${WCC} $(SRCS) $(W_SDL2) $(W_SDL2_TTF) $(W_FLAGS) $(W_CURL) -DCHESS_WINDOWS_VERSION -o $(NAME) $(NO_TERMINAL) $(RESOURCE_OBJ)
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"
endif

$(SERVER_EXE):
ifeq ($(shell [ -f ${SERVER_EXE} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling $(SERVER_EXE) ...$(RESET)\n"
	@${WCC} $(SERVER_SRC_DEPS) $(SERVER_SRC) $(SERVER_FLAGS) -o $(SERVER_EXE)
	@printf "$(GREEN)Compiling $(SERVER_EXE) done$(RESET)\n"
endif

$(RESOURCE_OBJ):
	@x86_64-w64-mingw32-windres $(RESOURCE) -o $(RESOURCE_OBJ)

update: $(NAME)
	@printf "$(CYAN)Updating windows version in /media/ycaro/Data/C_chess_win ${NAME} ...$(RESET)\n"
	@cp -r * /media/ycaro/Data/C_chess_win
	@printf "$(GREEN)Update done $(NAME) done$(RESET)\n"

clean:
	@${RM} $(RESOURCE_OBJ)
	@printf "$(RED)Clean $(RESOURCE_OBJ) $(RESET)\n"

fclean:	clean
	@${RM} ${NAME} $(SERVER_EXE)
	@printf "$(RED)Full clean $(NAME) $(SERVER_EXE) $(RESET)\n"
re:         fclean all