#include "../include/chess.h"
#include "../include/chess_log.h"
#include "../include/network.h"

/*
	@Brief Parser flag to handle listen and join mode
	- Network mode --network, network mode
		- Listen mode: Create lobby, wait for connection
			* -l --listen, wait for connection, default local port 8080
			* -i <ip>, server ip
			* example ./C_chess -n -l -i 127.0.0.1
		- Join mode: Connect to a server
			* -j --join, try to join a loby/game
			* -i <ip>, server ip
			* example ./C_chess -n -j -i 127.0.0.1
		- Reconect mode: Reconnect to a server
			* -r --reconnect, try to reconnect to a peer
			* -i <ip>, server ip
			* example ./C_chess -n -r -i
	- Local mode, local mode: no flag needed
		* example ./C_chess
*/


s8 is_power_of_two(int x) {
    return (x & (x - 1)) == 0 && x != 0;
}

s8 check_single_flag(u32 flag_value) {
    u32 combined_flags = 0;

    if (has_flag(flag_value, FLAG_LISTEN)) {
        combined_flags += FLAG_LISTEN;
    }
    if (has_flag(flag_value, FLAG_JOIN)) {
        combined_flags += FLAG_JOIN;
    }
    if (has_flag(flag_value, FLAG_RECONNECT)) {
        combined_flags += FLAG_RECONNECT;
    }

    return is_power_of_two(combined_flags);
}

u32 handle_chess_flag(int argc, char **argv, s8 *error, PlayerInfo *player_info) {
	ChessFlagContext	flag_ctx;
	u32					flag_value = 0;

	fast_bzero(&flag_ctx, sizeof(ChessFlagContext));

	/* Add flag option */
	add_flag_option(&flag_ctx, LISTEN_OPT_CHAR, FLAG_LISTEN, OPT_NO_VALUE, OPT_NO_VALUE, LISTEN_STR);
	add_flag_option(&flag_ctx, JOIN_OPT_CHAR, FLAG_JOIN, OPT_NO_VALUE, OPT_NO_VALUE, JOIN_STR);
	// add_flag_option(&flag_ctx, PORT_OPT_CHAR, FLAG_PORT, 65535, DECIMAL_VALUE, PORT_STR);
	add_flag_option(&flag_ctx, RECONNECT_OPT_CHAR, FLAG_RECONNECT, OPT_NO_VALUE, OPT_NO_VALUE, RECONNECT_STR);
	
	add_flag_option(&flag_ctx, SERVER_IP_OPT_CHAR, FLAG_SERVER_IP, 15, CHAR_VALUE, SERVER_IP_STR);
	add_flag_option(&flag_ctx, NETWORK_OPT_CHAR, FLAG_NETWORK, OPT_NO_VALUE, OPT_NO_VALUE, NETWORK_STR);

	add_flag_option(&flag_ctx, HELP_OPT_CHAR, FLAG_HELP, OPT_NO_VALUE, OPT_NO_VALUE, HELP_STR);


	player_info->dest_ip = ft_strdup("127.0.0.1");
	player_info->dest_port = SERVER_PORT;


	/* Parse flag in argv */
	flag_value = parse_flag(argc, argv, &flag_ctx, error);
	
	/* Check if error occured */
	if (*error == -1) {
		CHESS_LOG(LOG_ERROR, "Flag parser %d\n", *error);
		display_option_list(flag_ctx);
		goto flag_error;
	}

	if (has_flag(flag_value, FLAG_HELP)) {
		printf(HELP_MESSAGE);
		goto flag_error;
	}

	if (has_flag(flag_value, FLAG_NETWORK)) {
		CHESS_LOG(LOG_INFO, "Network mode\n");


		/* Get ip if needed */
		if (has_flag(flag_value, FLAG_SERVER_IP)) {
			player_info->dest_ip = get_opt_value(flag_ctx.opt_lst, flag_value, FLAG_SERVER_IP);
		} else {
			CHESS_LOG(LOG_INFO, "No server ip, default to localhost\n");
			player_info->dest_ip = ft_strdup("127.0.0.1");
		}
		
		if (has_flag(flag_value, FLAG_LISTEN) && has_flag(flag_value, FLAG_JOIN) && has_flag(flag_value, FLAG_RECONNECT)) {
			CHESS_LOG(LOG_ERROR, "Can't have listen join and reconnect flag at the same time\n");
			goto flag_error;
		} else if (check_single_flag(flag_value) == 0) {
			CHESS_LOG(LOG_ERROR, "Need to have listen or join flag\n");
			goto flag_error;
		}
	} else {
		CHESS_LOG(LOG_INFO, "Local mode\n");
		if (has_flag(flag_value, FLAG_RECONNECT) || has_flag(flag_value, FLAG_SERVER_IP) || has_flag(flag_value, FLAG_LISTEN) || has_flag(flag_value, FLAG_JOIN)) {
			CHESS_LOG(LOG_ERROR, "Can't have server ip, listen, join or reconnect flag in local mode\n");
			goto flag_error;
		}
	}

	// display_option_list(flag_ctx);
	free_flag_context(&flag_ctx);
	return (flag_value);

	flag_error:
		display_option_list(flag_ctx);
		free_flag_context(&flag_ctx);
		*error = -1;
		return (0);
}
