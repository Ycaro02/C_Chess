#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


void disable_active_field(SDLHandle *h, Profile *profile) {
	for (s32 i = 0; i < profile->nb_field; i++) {
		if (profile->tf[i]->is_active) {
			profile->tf[i]->update_data(h, profile->tf[i]);
		}
		profile->tf[i]->is_active = FALSE;
	}
}

void edit_name_func(SDLHandle *h) {
	if (h->menu.profile->tf[PFT_NAME]->is_active) {
		h->menu.profile->tf[PFT_NAME]->update_data(h, h->menu.profile->tf[PFT_NAME]);
	} else {
		disable_active_field(h, h->menu.profile);
	}
	h->menu.profile->tf[PFT_NAME]->is_active = !h->menu.profile->tf[PFT_NAME]->is_active;
}

void edit_timer_func(SDLHandle *h) {
	if (h->menu.profile->tf[PFT_TIMER]->is_active) {
		h->menu.profile->tf[PFT_TIMER]->update_data(h, h->menu.profile->tf[PFT_TIMER]);
	} else {
		disable_active_field(h, h->menu.profile);
	}
	h->menu.profile->tf[PFT_TIMER]->is_active = !h->menu.profile->tf[PFT_TIMER]->is_active;
}

static void set_profile_btn_text_func(Profile *profile, s32 idx, ProfileFieldType type) {
	if (type == PFT_NAME) {
		profile->btn[idx].text = ft_strdup("Edit");
		profile->btn[idx].func = edit_name_func;
	} else if (type == PFT_TIMER) {
		profile->btn[idx].text = ft_strdup("Edit");
		profile->btn[idx].func = edit_timer_func;
	}
}

typedef struct s_profile_field_info {
	char *opt_name;
	char *text;
	AcceptedCharFunc	accept_char_func;
	UpdateFunc 			update_func;
	s32		height;
	s32		width;
	s32		height_pad;
} ProfileFieldInfo;

TextField * init_profile_text_field(TTF_Font *font, SDL_Rect profile_rect, ProfileFieldInfo info, s32 i) {
	TextField *tf = NULL;
	SDL_Rect rect;

	rect.x = profile_rect.x + (profile_rect.w >> 3);
	rect.y = profile_rect.y + (profile_rect.y >> 1) + ((i * info.height_pad) * 2)+ (i * info.height) + info.height_pad;
	rect.w = info.width >> 1;
	rect.h = info.height;

	if (!(tf = init_text_field(rect, font, info.text, 8, info.accept_char_func, info.update_func))) {
		CHESS_LOG(LOG_ERROR, "init_text_field failed\n");
		return (NULL);
	}
	return (tf);
}

Button init_pofile_button(TextField *tf, TTF_Font *font, s32 btn_text_width, s32 btn_text_height) {
	Button btn = {0};

	btn.start.x = tf->rect.x + tf->rect.w + (tf->rect.w >> 1);
	btn.start.y = tf->rect.y;
	btn.width = (btn_text_width >> 1) - (btn_text_width >> 3);
	btn.height = btn_text_height;
	btn.end.x = btn.start.x + btn.width;
	btn.end.y = btn.start.y + btn.height;
	btn.text = ft_strdup("Edit");
	btn.func = NULL;
	center_btn_text(&btn, font);
	return (btn);
}

s8 is_nickname_accepted_char(SDL_Keycode code) {
	u8 mod = 0;

	get_keyboard_mod(&mod);

	if ((code >= SDLK_0 && code <= SDLK_9) || (code >= SDLK_KP_1 && code <= SDLK_KP_0)
		|| (code >= SDLK_a && code <= SDLK_z)) {
			if ((code >= SDLK_a && code <= SDLK_z) && (u8ValueGet(mod, IS_SHIFT) || u8ValueGet(mod, IS_CAPS))) {
				return (UPPERCASE_CHAR);
			}
		return (TRUE);
	}
	return (FALSE);
}

s8 is_timer_accepted_char(SDL_Keycode code) {
	(void)code;
	if ((code >= SDLK_0 && code <= SDLK_9) || (code >= SDLK_KP_1 && code <= SDLK_KP_0)) {
		return (TRUE);
	}
	return (FALSE);
}

void update_nickname(SDLHandle *h, TextField *tf) {
	(void)h;
	(void)tf;
	if (fast_strlen(tf->text) == 0) { return ; }
	free(h->player_info.name);
	h->player_info.name = ft_strdup(tf->text);
}

void update_timer(SDLHandle *h, TextField *tf) {
	(void)h;
	(void)tf;
	if (fast_strlen(tf->text) == 0) { return ; }
}

Profile *init_profile_page(SDLHandle *h, s32 nb_field) {
	Profile *profile = ft_calloc(sizeof(Profile), 1);

	if (!profile) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		return (NULL);
	}

	if (!(profile->tf = ft_calloc(sizeof(TextField *), nb_field))) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		free(profile);
		return (NULL);
	}

	/* Set default profile data */
	profile->btn_hover = PFT_INVALID;
	profile->current_btn_clicked = PFT_INVALID;
	profile->field_active = PFT_INVALID;
	profile->nb_field = nb_field;
	
	/* Set profile rect data */
	profile->rect.x = h->band_size.left + (h->tile_size.x << 1) + (h->tile_size.x >> 1);
	profile->rect.y = h->band_size.top + (h->tile_size.x << 1) - (h->tile_size.x >> 1);
	profile->rect.w = (h->tile_size.x << 2) - (h->tile_size.x);
	profile->rect.h = (h->tile_size.x << 2) + h->tile_size.x;
	
	/* Compute text field and button size */
	s32 field_height_pad = (profile->rect.h >> 4);
	s32 btn_text_height = (((profile->rect.h >> 1) - (field_height_pad * (nb_field + 1))) / nb_field) >> 1; 
	s32 btn_text_width = (profile->rect.w >> 1);

	/* Load font */
	if (!(profile->font = load_font(FONT_PATH, (btn_text_height >> 1) - (btn_text_height >> 4)))) {
		CHESS_LOG(LOG_ERROR, "load_font failed\n");
		free(profile->tf);
		free(profile);
		return (NULL);
	}
	
	/* Load font */
	if (!(profile->describe_font = load_font(FONT_PATH, (btn_text_height >> 1) + (btn_text_height >> 3)))) {
		CHESS_LOG(LOG_ERROR, "load_font failed\n");
		free(profile->tf);
		free(profile);
		return (NULL);
	}

	/* Alloc button */
	if (!(profile->btn = ft_calloc(sizeof(Button), nb_field))) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		unload_font(profile->font);
		free(profile->tf);
		free(profile);
		return (NULL);
	}

	if (!(profile->describe_field = ft_calloc(sizeof(char *), nb_field))) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		free(profile->btn);
		unload_font(profile->font);
		free(profile->tf);
		free(profile);
		return (NULL);
	}

	ProfileFieldInfo profile_field_info[] = {
		{"Nickname", "Default", is_nickname_accepted_char, update_nickname, btn_text_height, btn_text_width, field_height_pad},
		{"Timer", "30", is_timer_accepted_char, update_timer, btn_text_height, btn_text_width, field_height_pad},
	};

	/* Set profile field */
	for (s32 i = 0; i < nb_field; i++) {
		profile->describe_field[i] = ft_strdup(profile_field_info[i].opt_name);
		profile->tf[i] = init_profile_text_field(profile->font, profile->rect, profile_field_info[i], i);
		profile->btn[i] = init_pofile_button(profile->tf[i], profile->font,  btn_text_width, btn_text_height);
		set_profile_btn_text_func(profile, i, i);
	}


	return (profile);
}

void destroy_profile_page(Profile *profile) {
	for (s32 i = 0; i < profile->nb_field; i++) {
		if (profile->describe_field[i]) {
			free(profile->describe_field[i]);
		}
		if (profile->tf[i]) {
			destroy_text_field(profile->tf[i]);
		}
	}
	free(profile->tf);
	if (profile->font) {
		unload_font(profile->font);
	}
	free(profile);
}

void draw_profile_page(SDLHandle *h, Profile *profile) {
	SDL_Rect	rect = {0,0,0,0};

	/* Draw profile rect */
	rect.x = profile->rect.x;
	rect.y = profile->rect.y;
	rect.w = profile->rect.w;
	rect.h = profile->rect.h;
	SDL_SetRenderDrawColor(h->renderer, MENU_BG_COLOR);
	SDL_RenderFillRect(h->renderer, &rect);


	// Display profile in the top center of the rect
	write_text_in_rect(h, h->name_font, profile->rect, "Profile", TOP_CENTER, U32_WHITE_COLOR);

	/* Draw profile field */
	SDL_Color bg_color = {70,70,70,0};
	/* Draw the server ip text input */
	for (s32 i = 0; i < profile->nb_field; i++) {
		write_text_in_rect(h, profile->describe_font, profile->tf[i]->rect, profile->describe_field[i], TOP, U32_WHITE_COLOR);
		bg_color = (SDL_Color){70,70,70,0};
		if (profile->tf[i]->is_active) {
			bg_color = (SDL_Color){WHITE_COLOR};
		}
		render_text_field(h->renderer, profile->tf[i], (SDL_Color){0, 0, 0, 255}, bg_color);
	}

	draw_multiple_button(h, profile->font, profile->btn, profile->btn_hover, 0, profile->nb_field);

}