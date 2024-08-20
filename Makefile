include libft/rsc/mk/color.mk
include rsc/mk/source.mk

NAME            =   C_Chess
CC              =   clang

# SDL_LIB			=	-L./rsc/lib/install/lib -rpath ./rsc/lib/install/lib -lSDL2 -lSDL2_ttf
SDL_LIB			=	-L./rsc/lib/install/lib -rpath ./rsc/lib/install/lib -lSDL2

all:        $(NAME)

$(NAME): $(LIBFT) $(LIST) $(OBJ_DIR) $(OBJS)
	@$(MAKE_LIBFT)
	@$(MAKE_LIST)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LIBFT) $(LIST) $(SDL_LIB)
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"

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

$(OBJ_DIR):
	@mkdir -p $(ALL_SRC_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "$(YELLOW)Compile $<$(RESET) $(BRIGHT_BLACK)-->$(RESET) $(BRIGHT_MAGENTA)$@$(RESET)\n"
	@$(CC) $(CFLAGS) -o $@ -c $<
bonus: clear_mandatory $(NAME)

clear_mandatory:
ifeq ($(shell [ -f ${OBJ_DIR}/main.o ] && echo 0 || echo 1), 0)
	@printf "$(RED)Clean mandatory obj $(RESET)\n"
	@rm -rf ${OBJ_DIR}
endif

clean:
ifeq ($(shell [ -d ${OBJ_DIR} ] && echo 0 || echo 1), 0)
	@$(RM) $(OBJ_DIR)
	@printf "$(RED)Clean $(OBJ_DIR) done$(RESET)\n"
	@$(RM)
endif

clean_deps:
ifeq ($(shell [ -d "rsc/lib" ] && echo 0 || echo 1), 0)
	@$(RM) rsc/lib rsc/log
	@printf "$(RED)Remove rsc/lib rsc/log$(RESET)\n"
endif

fclean:		clean_lib clean
	@$(RM) $(NAME)
	@printf "$(RED)Clean $(NAME)$(RESET)\n"

clean_lib:
	@$(MAKE_LIST) fclean
	@$(MAKE_LIBFT) fclean
	@printf "$(RED)Clean libft, list$(RESET)\n"

test: $(NAME)
	@./$(NAME) -l

# @ulimit -c unlimited
leak thread debug: clean $(NAME)
	@printf	"$(CYAN)CFLAGS: $(CFLAGS)$(RESET)\n"
# @./$(NAME)

re: clean $(NAME)

.PHONY:		all clean fclean re bonus" > Makefile
