include libft/rsc/mk/color.mk
include rsc/mk/source.mk

NAME            =   C_Chess
CC              =   clang

SDL_LIB			=	-L./rsc/lib/install/lib -rpath ./rsc/lib/install/lib -lSDL2 -lSDL2_ttf

CURL_DEPS		= 	-lz -lbrotlidec -lssl -lcrypto -lnghttp2 -lpsl -lssh -lzstd -lldap -lrtmp -lgssapi_krb5 -lidn2 -lber

CURL_LIB		= 	-L./rsc/lib/curl_lib/lib -rpath ./rsc/lib/curl_lib/lib -lcurl $(CURL_DEPS)

CURL_INC		=	-I./rsc/lib/curl_lib/include/

LIB_DEPS		=   rsc/lib

GET_LOCAL_IP 	=	./rsc/sh/get_ipv4.sh 

IP_SERVER		=	$(shell $(GET_LOCAL_IP))

# Server sources and executable
SERVER_SRC		=	server/server.c src/network_os.c src/handle_signal.c src/move_save.c src/chess_log.c src/handle_reconnect.c -DCHESS_SERVER
SERVER_EXE		=	chess_server

all:        $(NAME)

$(NAME): $(LIB_DEPS) $(LIBFT) $(LIST) $(OBJ_DIR) $(OBJS) $(SERVER_EXE)
	@$(MAKE_LIBFT)
	@$(MAKE_LIST)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBFT) $(LIST) $(SDL_LIB) $(CURL_LIB) 
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"

$(SERVER_EXE): $(LIBFT) $(LIST)
	@printf "$(CYAN)Compiling ${SERVER_EXE} ...$(RESET)\n"
	@$(CC) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC) $(LIBFT) $(LIST)
	@printf "$(GREEN)Compiling $(SERVER_EXE) done$(RESET)\n"

$(LIST):
ifeq ($(shell [ -f ${LIST} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling list...$(RESET)\n"
	@$(MAKE_LIST)
	@printf "$(GREEN)Compiling list done$(RESET)\n"
endif

$(LIBFT):
ifeq ($(shell [ -f ${LIBFT} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling libft...$(RESET)\n"
	@$(MAKE_LIBFT)
	@printf "$(GREEN)Compiling libft done$(RESET)\n"
endif

$(LIB_DEPS) :
ifeq ($(shell [ -d $(LIB_DEPS) ] && echo 0 || echo 1), 1)
	@./rsc/install/load_lib.sh
endif

$(OBJ_DIR):
	@printf "$(YELLOW)Create $(ALL_OBJ_DIR)$(RESET)\n"
	@mkdir -p $(ALL_OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "$(YELLOW)Compile $<$(RESET) $(BRIGHT_BLACK)-->$(RESET) $(BRIGHT_MAGENTA)$@$(RESET)\n"
	@$(CC) $(CFLAGS) $(CURL_INC) -o $@ -c $<

bonus: clear_mandatory $(NAME)

clear_mandatory:
ifeq ($(shell [ -f ${OBJ_DIR}/main.o ] && echo 0 || echo 1), 0)
	@printf "$(RED)Clean mandatory obj $(RESET)\n"
	@rm -rf ${OBJ_DIR}
endif

clean:
ifeq ($(shell [ -d ${OBJ_DIR} ] && echo 0 || echo 1), 0)
	@$(RM) $(OBJ_DIR) $(SERVER_EXE)
	@printf "$(RED)Clean $(OBJ_DIR) $(SERVER_EXE) done$(RESET)\n"
	@$(RM)
endif

clean_deps:
ifeq ($(shell [ -d "rsc/lib" ] && echo 0 || echo 1), 0)
	@$(RM) rsc/lib rsc/log
	@printf "$(RED)Remove rsc/lib rsc/log$(RESET)\n"
endif

fclean:	clean_android clean_lib clean
	@make -s -C windows fclean
	@$(RM) $(NAME) $(SERVER_EXE) $(SERVER_EXE) 
	@printf "$(RED)Clean $(NAME) $(SERVER_EXE)$(RESET)\n"

clean_android:
ifeq ($(shell [ -d "android/chess_app/app/build" ] && echo 0 || echo 1), 0)
	@cd android/chess_app && ./build_android.sh clean && cd ../..
	@printf "$(RED)Remove android/chess_app/app/build$(RESET)\n"
endif

clean_lib:
	@$(MAKE_LIST) fclean
	@$(MAKE_LIBFT) fclean
	@printf "$(RED)Clean libft, list$(RESET)\n"

test: $(NAME)
	@./$(NAME)

test_server: $(SERVER_EXE)
	@ echo "Server running on : $(IP_SERVER):24242"
	@ ./$(SERVER_EXE)

test_listen: $(NAME)
	@./$(NAME) -n -l -i $(IP_SERVER)

test_local: $(NAME)
	@./rsc/sh/chess_local_test.sh


# @ulimit -c unlimited
leak thread debug: clean $(NAME)
	@printf	"$(CYAN)CFLAGS: $(CFLAGS)$(RESET)\n"
# @./$(NAME)

re: clean $(NAME)

.PHONY:		all clean fclean re bonus" > Makefile
