include ../libft/rsc/mk/color.mk


SRC_DIRS    = ../libft/libft ../libft/list ../libft/printf_fd ../libft/parse_flag ../libft/stack_string ../src

# Find all source files
SRCS        = $(shell find $(SRC_DIRS) -name '*.c')

FIRST_NAME  = output

NAME        = output.js


all:	$(NAME)

$(NAME):
ifeq ($(shell [ -f ${NAME} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@cp -r ../windows/rsc .
	@emcc -s USE_SDL=2 -s USE_SDL_TTF=2 -s MODULARIZE=1 -s EXPORT_NAME="createModule" --preload-file rsc -s -D_EMSCRIPTEN_VERSION_ -o $(NAME) $(SRCS)
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"
endif

test:
	@./load_emcc.sh
	@printf "$(CYAN)Test $(NAME), http_server runing on port 8000 ...$(RESET)\n"
	@python3 -m http.server 8000

clean:
	@printf "$(RED)Clean $(RESET)\n"

fclean:	clean
	@${RM} ${NAME} $(FIRST_NAME).wasm $(FIRST_NAME).data
	@printf "$(RED)Full clean ${NAME} $(FIRST_NAME).wasm $(FIRST_NAME).data output.wasm.map $(RESET)\n"
re:         fclean all