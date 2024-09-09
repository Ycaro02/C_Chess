CFLAGS			=	-Wall -Wextra -Werror -O3

OBJ_DIR			=	obj

ALL_OBJ_DIR 	=	obj

SRC_DIR 		=	src

MAIN_MANDATORY 	=	main.c

SRCS			=	chess_board.c \
					chess_flag.c \
					chess_piece_move.c \
					generic_piece_move.c \
					handle_board.c \
					draw_board.c \
					network_os.c \
					chess_network.c \
					network_routine.c \
					handle_message.c \
					handle_signal.c \
					handle_sdl.c \
					pawn_promotion.c \
					chess_log.c \
					text_display.c \
					move_save.c \
					handle_reconnect.c \
					parse_message_receive.c \
					timer.c \
					chess_menu.c \
					text_input.c \
					button.c \
					profile.c \
					nickname.c \

MAKE_LIBFT		=	make -s -C libft -j

MAKE_LIST		=	make -s -C libft/list -j

LIBFT			= 	libft/libft.a

LIST			= 	libft/list/linked_list.a

OBJS 			= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

RM			=	rm -rf

ifeq ($(findstring bonus, $(MAKECMDGOALS)), bonus)
ASCII_NAME	= "bonus"
SRCS += $(SRCS_BONUS)
else
SRCS += $(MAIN_MANDATORY)
endif

ifeq ($(findstring leak, $(MAKECMDGOALS)), leak)
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
else ifeq ($(findstring thread, $(MAKECMDGOALS)), thread)
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=thread
else ifeq ($(findstring debug, $(MAKECMDGOALS)), debug)
CFLAGS = -Wall -Wextra -Werror -g3
endif
