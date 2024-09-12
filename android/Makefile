NAME            =   C_Chess
CC              =   $(NDK_TOOLCHAIN)/bin/clang
CXX             =   $(NDK_TOOLCHAIN)/bin/clang++
SYSROOT         =   $(NDK_SYSROOT)

CFLAGS          =   --sysroot=$(SYSROOT) -target $(C_TARGET) -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/${SYSROOT_SUB_DIR} -I../rsc/lib/install/include -I../rsc/lib/SDL2_ttf-2.22.0 -DANDROID ${ARM_DISABLE}
LDFLAGS         =   --sysroot=$(SYSROOT) -target $(C_TARGET) -L$(SYSROOT)/usr/lib -L$(SDL2_LIBS) -L../rsc/lib/SDL2_ttf-2.22.0/libs/${C_ARCHITECTURE} -lSDL2 -lSDL2_ttf -landroid -llog -ldl

SDL2_DIR        = ../rsc/lib/SDL2
SDL2_TTF_DIR    = ../rsc/lib/SDL2_ttf-2.22.0

SRC_DIRS        = ../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string ../src
SRCS            = $(shell find $(SRC_DIRS) -name '*.c')

all: $(NAME)

$(NAME): $(SRCS)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@printf "$(CYAN)ARCHI: $(C_ARCHITECTURE), CLANG: $(CLANG_TOOLCHAIN), C_TARGET: $(C_TARGET), SYSROOT_DIR: $(SYSROOT_SUB_DIR)\n"
	$(CC) $(CFLAGS) -o $(NAME) $(SRCS) real_main.c $(LDFLAGS)
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"

clean:
	@printf "$(RED)Clean $(RESET)\n"

fclean: clean
	@printf "$(RED)Fclean $(RESET)\n"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re