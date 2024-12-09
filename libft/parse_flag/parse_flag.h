#ifndef PARSE_FLAG_H
#define PARSE_FLAG_H 1

#include "../libft.h"

/* Message invalid option */
#define PARSE_FLAG_ERR_MSG              RED"%s: invalid option -- %s\nTry %s -h for more information\n"RESET

/* Message argument required for flag */
#define PARSE_FLAG_ERR_MSG_ARGS_REQ     RED"%s: option requires an argument -- %c\nTry %s -h for more information\n"RESET

/* Message wrong arguments for flag */
#define PARSE_FLAG_ERR_MSG_WRONG_ARGS   RED"%s: invalid argument -- %c don't accept [%s] as argument\nTry %s -h for more information\n"RESET


#define CHAR_FORMAT		0U          /* short format for check_for_flag */
#define LONG_FORMAT		1U          /* long format for check_for_flag */

#define OPT_NO_VALUE		0U       /* no value linked */
#define DECIMAL_VALUE		1U       /* decimal base for value */
#define HEXA_VALUE			2U       /* hexadecimal base for value */
#define OCTAL_VALUE			3U       /* octal base for value */
#define BINARY_VALUE		4U       /* binary base for value */
#define CHAR_VALUE			5U       /* char value just store string */

typedef struct opt_node {
    uint8_t             flag_char;  /* char represent flag */
    uint32_t            flag_val;   /* flag value, used with bitwise to create application flag */
    uint32_t			max_val;	/* max value for linked val, or strlen max for string store */
    uint8_t             has_value;  /* if value is linked */
	int8_t				value_type; /* value type */
    char                *full_name; /* full name opt */
	/* union value if value is linked */
	union u_val {
		uint32_t digit;
		char	 *str;	
	} val;

}   t_opt_node;

typedef struct flag_context {
    char		*prg_name;		/* program name */
	char        *opt_str;       /* full char opt */
    t_list      *opt_lst;       /* list of opt node */
}   t_flag_context;

/* parse cmd_line */
t_list  *extract_args(int argc, char **argv);


/****************************/
/*		Parse flag			*/
/****************************/

/**
 * @brief Parse flag
 * @param argc number of argument
 * @param argv pointer on argument
 * @param flag_c pointer on flag context
 * @param error pointer on error
 * @return flags if valid, 0 otherwise and set error to -1
*/
u32     parse_flag(int argc, char **argv,t_flag_context *flag_c, int8_t *error);

/**
 * @brief Display flags
 * @param all_flag all flag char available
 * @param flags flags to display
*/
void    display_flags(char *all_flag, int flags);

/**
 * @brief get option value
 * @param opt_lst list of opt node
 * @param flag flag to check
 * @param to_find flag to find
 * @return value if found, NULL otherwise
*/
void *get_opt_value(t_list *opt_lst, uint32_t flag, uint32_t to_find);

/**
 *	@brief Set flag, enable target flag_val in flags
 *	@param flags pointer on flags
 *	@param flag_val flag value to set
*/
void set_flag(u32 *flags, u32 flag_val);

/**
 *	@brief Unset flag, disable target flag_val in flags
 *	@param flags pointer on flags
 *	@param flag_val flag value to unset
*/
void unset_flag(u32 *flags, u32 flag_val);

/**
 *	@brief Check if flag_val is enable in flags
 *	@param flags flags to check
 *	@param flag_val flag value to check
 *	@return 1 if flag_val is enable in flags, 0 otherwise
*/
int8_t  has_flag(u32 flags, u32 flag_val);

/* has flag wrapper */
int8_t  flag_already_present(u32 flags, u32 flag_val);


void	reverse_flag(u32 *flags, u32 flag_val);


/****************************/
/*		Handle Option		*/
/****************************/

/**
 *	@brief Add flag option
 *	@param flag_c flag context
 *	@param c flag char
 *	@param flag_val flag value
 *	@param value value
 *	@param value_type type of value
 *	@param full_name full name of the flag
 *	@return 1 if success, 0 otherwise
*/
int8_t add_flag_option(t_flag_context *flag_c, uint8_t c, uint32_t flag_val, uint32_t value, int8_t value_type, char* full_name);

/**
 * @brief Display option list for debug
 * @param flag_c flag context
 * @param opt_lst list of opt node
*/
void display_option_list(t_flag_context flag_c);

/**
 *	@brief Free flag context
 *	@param flag_c flag context
*/
void    free_flag_context(t_flag_context *flag_c);

/* explicit compare function for search exist opt */
int8_t	is_same_full_name(void *node, void *data);
int8_t  is_same_char_opt(void *content, void *c);
int8_t  is_same_flag_val_opt(void *content, void *value);

/**
 * @brief Search for exist opt
 * @param opt_lst list of opt node
 * @param cmp compare function
 * @param data data to compare
 * @return opt_node if found, NULL otherwise
*/
void	*search_exist_opt(t_list *opt_lst, int8_t (cmp()), void *data);

/**
 * @brief Search for value linked to flag
 * @param argv pointer on argv from main
 * @param i pointer on first argv index counter
 * @param opt pointer on opt node to update
 * @param long_format_bool long format or short (char) format
 * @return 1 if found, 0 otherwise
*/
// int		search_opt_value(char **argv, int *i, t_opt_node *opt, uint8_t long_format_bool);

#endif /* PARSE_FLAG_H */