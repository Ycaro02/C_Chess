#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


/* Check if the ip is in ipv4 format */
s8 is_ipv4_format(char *ip) {
	char **split = ft_split(ip, '.');
	int  nb = 0;

	if (split == NULL) {
		return (FALSE);
	}

	for (s32 i = 0; i < 4; i++) {
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
	return (TRUE);

	ipv4_error:
		CHESS_LOG(LOG_ERROR, "Error: is_ipv4_format %s\n", ip);
		free_double_char(split);
		return (FALSE);
}


TextField init_text_field(SDL_Rect rect, int buff_size_max, TTF_Font *font, char *initial_text) {
	TextField text_field = {};

	int len = fast_strlen(initial_text);

	ft_memcpy(&text_field.rect, &rect, sizeof(SDL_Rect));
	text_field.buffer_size = buff_size_max;
	text_field.font = font;
	fast_bzero(text_field.text, buff_size_max);
	ftlib_strcpy(text_field.text, initial_text, len);
	text_field.cursor = len;
	return (text_field);
}

s8 is_accepted_char(char *ip, SDL_Keycode type) {
	(void)ip;
	if (type >= SDLK_0 && type <= SDLK_9) // number
		return (TRUE);
	else if (type == SDLK_PERIOD) // dot
		return (TRUE);
	return (FALSE);
}

void update_server_ip(SDLHandle *h, TextField *text_field) {
	if (is_ipv4_format(text_field->text)) {
		free(h->player_info.dest_ip);
		h->player_info.dest_ip = ft_strdup(text_field->text);
	} else {
		fast_bzero(text_field->text, text_field->buffer_size);
		ft_memcpy(text_field->text, h->player_info.dest_ip, fast_strlen(h->player_info.dest_ip));
	}
}

void handle_text_input(SDLHandle *h, SDL_Event *event) {
	TextField *text_field = &h->menu.ip_field;

    if (event->type == SDL_KEYDOWN) {
        if (is_accepted_char(text_field->text, event->key.keysym.sym)) {
            if (text_field->cursor < text_field->buffer_size - 1) {
                text_field->text[text_field->cursor] = (char)event->key.keysym.sym;
				text_field->cursor += 1;
                text_field->text[text_field->cursor] = '\0';
            } else {
                printf("Text buffer full\n");
            }
        } else if (event->key.keysym.sym == SDLK_BACKSPACE && text_field->cursor > 0) {
            text_field->cursor -= 1;
			text_field->text[text_field->cursor] = '\0';
        } else if (event->key.keysym.sym == SDLK_RETURN) {
			update_server_ip(h, text_field);
			text_field->is_active = FALSE;
		} else if (event->key.keysym.sym == SDLK_ESCAPE) {
			text_field->is_active = FALSE;
		}
    }
}

void render_text_field(SDL_Renderer *renderer, TextField *text_field, SDL_Color text_color, SDL_Color bg_color) {
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
}