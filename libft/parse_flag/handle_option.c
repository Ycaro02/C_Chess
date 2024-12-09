#include "parse_flag.h"

/* Start option handle */
/**
 * @brief Display option list for debug
*/
void display_option_list(t_flag_context flag_c)
{
    t_opt_node *node = NULL; 
	ft_printf_fd(2, CYAN"Option list: Full flag str: %s\n"RESET, flag_c.opt_str);
    for (t_list *tmp = flag_c.opt_lst; tmp; tmp = tmp->next) {
        node = tmp->content;
        ft_printf_fd(1, CYAN"Flag: %c"RESET", "PURPLE"Flag_val %d "RESET, node->flag_char, node->flag_val);
    
		if (node->value_type == DECIMAL_VALUE) {
			ft_printf_fd(1, ORANGE"Digit value: %u\n"RESET, node->val.digit);
		}
		else if (node->value_type == HEXA_VALUE || node->value_type == CHAR_VALUE) {
			ft_printf_fd(1, PURPLE"Str value: %s\nLen str %d\n"RESET, node->val.str, ft_strlen(node->val.str));
		} else {
			ft_printf_fd(1, "No value\n");
		}
	}
}

/**
 * @brief Check if node full name is the same as data
 * @param node node to check
 * @param data data to compare
*/
int8_t is_same_full_name(void *node, void *data) {
    if (!node || !data)
        return (0);
    return (ft_strncmp(((t_opt_node *)node)->full_name, (char *)data, ft_strlen(((t_opt_node *)node)->full_name)) == 0);
}

/**
 * @brief Check if content flag char is the same as c
 * @param content content to check
 * @param c char to compare
*/
int8_t is_same_char_opt(void *content, void *c) {
    if (!content || !c)
        return (0);
    return (((t_opt_node *)content)->flag_char == *(uint8_t *)c);
}

/**
 * @brief Check if content flag val is the same as value
 * @param content content to check
 * @param value value to compare
*/
int8_t is_same_flag_val_opt(void *content, void *value) {
    if (!content || !value)
        return (0);
    return (((t_opt_node *)content)->flag_val == *(uint32_t *)value);
}

/**
 * @brief Search for exist opt
 * @param opt_lst list of opt node
 * @param cmp compare function
 * @param data data to compare
 * @return opt_node if found, NULL otherwise
*/
void *search_exist_opt(t_list *opt_lst, int8_t (cmp(void *, void *)), void *data)
{
    for (t_list *tmp = opt_lst; tmp; tmp = tmp->next) {
        if (cmp(tmp->content, data)) {
            return (tmp->content);
        }
    }
    return (NULL);
}

/**
 *	@brief Create option node
 *	@param c flag char
 *	@param flag_val flag value
 *	@param value value
 *	@param full_name full name of the flag
 *	@return opt_node if success, NULL otherwise
*/
static t_opt_node *create_opt_node(uint8_t c, uint32_t flag_val, uint32_t value, char *full_name, int8_t value_type)
{
    t_opt_node *opt = ft_calloc(sizeof(t_opt_node), 1);

    if (!opt) {
        ft_printf_fd(2, "Failed to allocate memory for opt\n");
        return (NULL);
    }
    opt->flag_char = c;
    opt->flag_val = flag_val;
	opt->max_val = value;
    // opt->value = (value != OPT_NO_VALUE);
    opt->has_value = (value != OPT_NO_VALUE);
    opt->full_name = ft_strdup(full_name);
	opt->value_type = value_type;
	if (value_type == DECIMAL_VALUE) {
		opt->val.digit = 0;
	}
	else if (value_type == HEXA_VALUE || value_type == CHAR_VALUE) {
		opt->val.str = NULL;
	}
	//
	// ft_printf_fd(2, RED"full_name: %s, max_val: %u, has_vas %u\n"RESET, full_name, value, opt->has_value);

    return (opt);
}

/**
 *	@brief Update opt_str
 *	@param flag_c flag context
 *	@param c char to add
 *	@return 1 if success, 0 otherwise
*/
static int8_t update_opt_str(t_flag_context *flag_c, uint8_t c)
{
    char new_char[2] = {c, 0};
    if (flag_c->opt_str == NULL) {
        flag_c->opt_str = ft_strdup(new_char);
    } else {
        flag_c->opt_str = ft_strjoin_free(flag_c->opt_str, new_char, 'f');
    }
    if (!flag_c->opt_str) {
        ft_printf_fd(2, "Failed to allocate memory for opt_str\n");
        return (0);
    }
    return (1);
}

/**
 *	@brief Add flag option
 *	@param flag_c flag context
 *	@param c flag char
 *	@param flag_val flag value
 *	@param value max value accepted
 *	@param value_type type of value
 *	@param full_name full name of the flag
 *	@return 1 if success, 0 otherwise
*/
int8_t add_flag_option(t_flag_context *flag_c, uint8_t c, uint32_t flag_val, uint32_t value, int8_t value_type, char* full_name)
{
    t_opt_node *opt = NULL;
    int8_t ret = 0;

    if (!flag_c) {
        ft_printf_fd(2, "Invalid list option addr\n");
        return (0);
    } 
    else if (search_exist_opt(flag_c->opt_lst, is_same_char_opt, &c)\
        || search_exist_opt(flag_c->opt_lst, is_same_flag_val_opt, &flag_val)) {
        ft_printf_fd(2, RED"Opt char |%c| or flag val |%d| already present\n"RESET, c, flag_val);
        return (0);
    }
    opt = create_opt_node(c, flag_val, value, full_name, value_type);
    ft_lstadd_back(&flag_c->opt_lst, ft_lstnew(opt));
    ret = update_opt_str(flag_c, c); 
    return (ret);
}

/**
 *	@brief Free opt node
 *	@param content content to free
*/
void free_opt_node(void *content)
{
    t_opt_node *opt = (t_opt_node *)content;
    if (opt) {
        if (opt->full_name) {
            free(opt->full_name);
        }
		if ((opt->value_type == HEXA_VALUE || opt->value_type == CHAR_VALUE) && opt->val.str) {
			free(opt->val.str);
		}
        free(opt);
    }
}

/**
 *	@brief Free flag context
 *	@param flag_c flag context
*/
void free_flag_context(t_flag_context *flag_c)
{
    if (flag_c) {
		if (flag_c->opt_str) {
            free(flag_c->opt_str);
        }
        if (flag_c->opt_lst) {
            ft_lstclear(&flag_c->opt_lst, free_opt_node);
        }
    }
}