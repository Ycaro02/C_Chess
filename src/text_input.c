#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

#include "../include/android_macro.h"

s32 parse_dot(char *ip) {
	s32 count = 0;
	s8	is_dot = FALSE;

	for (s32 i = 0; ip[i]; i++) {
		if (ip[i] == '.') {
			if (is_dot) {
				return (-1);
			}
			count++;
			is_dot = TRUE;
		} else {
			is_dot = FALSE;
		}
	}
	return (count);
}

/* Check if the ip is in ipv4 format */
s8 is_ipv4_format(char *ip) {
	char **split = ft_split(ip, '.');
	int  nb = 0;

	if (split == NULL) {
		return (FALSE);
	}

	if (double_char_size(split) != 4 || parse_dot(ip) != 3) {
		goto ipv4_error;
	}

	for (s32 i = 0; i < 4; i++) {
		// printf("split[%d] = %s\n", i, split[i]);
		if (fast_strlen(split[i]) > 3) {
			goto ipv4_error;
		}
		for (s32 j = 0; j < fast_strlen(split[i]); j++) {
			if (!ft_isdigit(split[i][j])) {
				goto ipv4_error;
			}
		}
		nb = ft_atoi(split[i]);
		if (nb < 0 || nb > 255) {
			goto ipv4_error;
		}
	}

	free_double_char(split);
	return (TRUE);

	ipv4_error:
		CHESS_LOG(LOG_ERROR, "Not ipv4 format %s\n", ip);
		free_double_char(split);
		return (FALSE);
}


void destroy_text_field(TextField *text_field) {
	if (text_field->text) {
		free(text_field->text);
	}
	free(text_field);
}

TextField *init_text_field(SDL_Rect rect, TTF_Font *font, char *initial_text, s32 buff_size_max, AcceptedCharFunc accept_char_func, UpdateFunc update_func) {
	TextField	*text_field = ft_calloc(1, sizeof(TextField));
	s32			len = fast_strlen(initial_text);

	if (!text_field) {
		CHESS_LOG(LOG_ERROR, "Failed to allocate memory for text field\n");
		return (NULL);
	}

	ft_memcpy(&text_field->rect, &rect, sizeof(SDL_Rect));
	text_field->buffer_size = buff_size_max;
	text_field->font = font;
	if (!(text_field->text = ft_calloc(buff_size_max + 1, sizeof(char)))) {
		CHESS_LOG(LOG_ERROR, "Failed to allocate memory for text field\n");
		return (NULL);
	}
	if (len > buff_size_max) {
		CHESS_LOG(LOG_ERROR, "Initial string to long |%s| -> %d, buffer size %d\n", initial_text, len, buff_size_max);
		len = buff_size_max;
		text_field->cursor = 0;
	} else {
		ftlib_strcpy(text_field->text, initial_text, len);
		text_field->cursor = len;
	}
	text_field->is_accepted_char = accept_char_func;
	text_field->update_data = update_func;
	return (text_field);
}

s8 ip_server_accepted_char(SDL_Keycode type) {
	/* Number classic and keypad */
	if ((type >= SDLK_0 && type <= SDLK_9) || (type >= SDLK_KP_1 && type <= SDLK_KP_0))
		return (TRUE);
	else if (type == SDLK_PERIOD || type == SDLK_KP_PERIOD) /* Dot classic and keypad */
		return (TRUE);
	return (FALSE);
}

void ip_server_update_data(SDLHandle *h, TextField *text_field) {
	if (is_ipv4_format(text_field->text)) {
		free(h->player_info.dest_ip);
		h->player_info.dest_ip = ft_strdup(text_field->text);
		text_field->cursor = fast_strlen(text_field->text);
	} else {
		fast_bzero(text_field->text, text_field->buffer_size);
		ft_memcpy(text_field->text, h->player_info.dest_ip, fast_strlen(h->player_info.dest_ip));
		text_field->cursor = fast_strlen(text_field->text);
	}
}


// Need to refactor this to take the TextFIeld as a parameter
// TextField need to have a function pointer on accepted_char function
// TextField need to have a function pointer on update function
void handle_text_input(SDLHandle *h, SDL_Event *event, TextField *tf) {
	s8 ret = FALSE, uppercase = 0;
	u8 mod = 0;

	get_keyboard_mod(&mod);

    if (event->type == SDL_KEYDOWN) {
        // if (ip_server_accepted_char(event->key.keysym.sym)) {
        if ((ret = tf->is_accepted_char(event->key.keysym.sym))) {
            if (tf->cursor < tf->buffer_size - 1) {
				if (ret == UPPERCASE_CHAR) {
					uppercase = 32;
				}
                tf->text[tf->cursor] = ((char)event->key.keysym.sym - uppercase);
				tf->cursor += 1;
                tf->text[tf->cursor] = '\0';
				/* Update the cursor time to avoid hiden him when the user write */
				tf->last_cursor_time = SDL_GetTicks64();
            } 
        } else if (event->key.keysym.sym == SDLK_BACKSPACE && u8ValueGet(mod, IS_CTRL)) {
			tf->cursor = 0;
			fast_bzero(tf->text, tf->buffer_size);
		} else if (event->key.keysym.sym == SDLK_BACKSPACE && tf->cursor > 0) {
            tf->cursor -= 1;
			tf->text[tf->cursor] = '\0';
			tf->last_cursor_time = SDL_GetTicks64();
        } else if (event->key.keysym.sym == SDLK_RETURN) {
			tf->update_data(h, tf);
			DISABLE_TEXTFIELD(&tf->is_active);
		} else if (ESCAPE_KEY(event->key.keysym.sym)) {
			tf->update_data(h, tf);
			DISABLE_TEXTFIELD(&tf->is_active);
		}

    }
}

void render_text_field(SDL_Renderer *renderer, TextField *text_field, SDL_Color text_color, SDL_Color bg_color) {
	u64			current_time = SDL_GetTicks64();

    SDL_Surface *surface = TTF_RenderText_Blended(text_field->font, text_field->text, text_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int text_width;
    TTF_SizeText(text_field->font, text_field->text, &text_width, NULL);
    SDL_Rect renderQuad = {text_field->rect.x + (text_field->rect.x >> 6), text_field->rect.y, text_width, text_field->rect.h};
	
	// Draw the text field
	SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
	// SDL_SetRenderDrawColor(h->renderer, 255,255,255,255);
	SDL_RenderFillRect(renderer, &text_field->rect);

    SDL_FreeSurface(surface);

	// Draw the text
    SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
    SDL_DestroyTexture(texture);

	if (current_time - text_field->last_cursor_time > 530) {
		text_field->last_cursor_time = current_time;
		text_field->cursor_visible = !text_field->cursor_visible;
	}

	// Draw the cursor if the text field is active and the cursor is visible
    if (text_field->cursor_visible && text_field->is_active) {
        int cursor_x = text_field->rect.x + (text_field->rect.x >> 6) + text_width;
        SDL_Rect cursor_rect = {cursor_x, text_field->rect.y+2, 2, text_field->rect.h-4}; // 2 pixels wide cursor
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
        SDL_RenderFillRect(renderer, &cursor_rect);
    }
}