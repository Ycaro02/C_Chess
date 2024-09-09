#include "../include/chess.h"
#include "../include/handle_sdl.h"
#include "../include/chess_log.h"


void edit_name_func(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Edit name\n");
}

void edit_timer_func(SDLHandle *h) {
	(void)h;
	CHESS_LOG(LOG_INFO, "Edit Timer\n");
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

TextField * init_profile_text_field(TTF_Font *font , SDL_Rect profile_rect, char *text, s32 height, s32 width, s32 height_pad, s32 i) {
	TextField *tf = NULL;
	SDL_Rect rect;

	rect.x = profile_rect.x + (profile_rect.w >> 3);
	rect.y = profile_rect.y + (profile_rect.y >> 1) + (i * height_pad) + (i * height) + height_pad;
	rect.w = width;
	rect.h = height;

	if (!(tf = init_text_field(rect, 8, font, text))) {
		CHESS_LOG(LOG_ERROR, "init_text_field failed\n");
		return (NULL);
	}
	return (tf);
}

Button init_pofile_button(TextField *tf, TTF_Font *font, s32 btn_text_width, s32 btn_text_height) {
	Button btn = {0};

	btn.start.x = tf->rect.x + tf->rect.w + (tf->rect.w >> 2);
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
	profile->rect.x = h->band_size.left + (h->tile_size.x << 1);
	profile->rect.y = h->band_size.top + (h->tile_size.x << 1) - (h->tile_size.x >> 1);
	profile->rect.w = h->tile_size.x << 2;
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

	/* Alloc button */
	if (!(profile->btn = ft_calloc(sizeof(Button), nb_field))) {
		CHESS_LOG(LOG_ERROR, "malloc failed\n");
		unload_font(profile->font);
		free(profile->tf);
		free(profile);
		return (NULL);
	}

	char *basic_string[] = {
		"Nickname",
		"30",
	};
	/* Set profile field */
	for (s32 i = 0; i < nb_field; i++) {
		profile->tf[i] = init_profile_text_field(profile->font, profile->rect, basic_string[i], btn_text_height, btn_text_width, field_height_pad, i);
		profile->btn[i] = init_pofile_button(profile->tf[i], profile->font, btn_text_width, btn_text_height);
		set_profile_btn_text_func(profile, i, i);
	}


	return (profile);
}

void destroy_profile_page(Profile *profile) {
	for (s32 i = 0; i < profile->nb_field; i++) {
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
	write_text_in_rect(h, h->menu.btn_text_font, profile->rect, "Profile", TOP_CENTER);

	/* Draw profile field */
	for (s32 i = 0; i < profile->nb_field; i++) {
		render_text_field(h->renderer, profile->tf[i], (SDL_Color){255, 255, 255, 255}, (SDL_Color){0, 0, 0, 255});
	}

	draw_multiple_button(h, profile->font, profile->btn, profile->btn_hover, 0, profile->nb_field);

}