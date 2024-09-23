
#ifndef _ANDROID_MACRO_HANDLE_SDL_H_
#define _ANDROID_MACRO_HANDLE_SDL_H_

#include "../include/handle_sdl.h"

/**
 * @file android_macro.h

 * Macros:
 * - ENABLE_TEXFIELD: Macro to enable the text field (show the keyboard).
 * - DISABLE_TEXTFIELD: Macro to disable the text field (hide the keyboard).
 * - DRAW_PIECE_KILL: Macro to draw a piece kill on the screen.
 * - COMPUTE_WIN_BAND_SIZE: Macro to compute the size of the window band.
 * - BUILD_TIMER_RECT: Macro to build the timer rectangle.
 * - BUILD_NAME_RECT: Macro to build the name rectangle.
 * - BMP_TO_TEXTURE: Macro to load a texture from a BMP file.
 */

#ifdef __ANDROID__

	/**
	 * @brief JNI function to call when the activity is destroyed
	 */
	JNIEXPORT FT_INLINE void JNICALL Java_org_libsdl_app_SDLActivity_chessOnDestroy(JNIEnv* env, jobject obj) {
		SDLHandle *h = get_SDL_handle();
		chess_destroy(h);
		// register_data(h, DATA_SAVE_FILE);
		// if (h->player_info.nt_info) {
		// 	send_disconnect_to_server(h->player_info.nt_info->sockfd, h->player_info.nt_info->servaddr, h->my_remaining_time);
		// }
	}

	/**
	 * @brief JNI function to call when the activity is paused
	 */
	JNIEXPORT FT_INLINE void JNICALL Java_org_libsdl_app_SDLActivity_chessOnPause(JNIEnv* env, jobject obj) {
		SDLHandle *h = get_SDL_handle();
		CHESS_LOG(LOG_INFO, "Call chessOnPause()\n");
		chess_destroy(h);
	}

	/**
	 * @brief JNI function to call when the activity is resumed
	 */
	JNIEXPORT FT_INLINE void JNICALL Java_org_libsdl_app_SDLActivity_chessOnResume(JNIEnv* env, jobject obj) {
		CHESS_LOG(LOG_INFO, "Call chessOnResume()\n");
		chess_start_program();
	}

	/* src/handle_textfield_keyboard.c */
	void android_show_keyboard(s8 *is_active);
	void android_hide_keyboard(s8 *is_active);

	/* src/display_piece_kill.c */
	void android_draw_piece_kill(SDLHandle *h, s8 is_bot, s8 is_black);

	/* src/compute_win_elem_size.c */
	void android_compute_win_size(SDLHandle *h);
	SDL_Rect android_build_timer_rect(SDLHandle *h, s8 is_bot_band);
	SDL_Rect android_build_name_rect(SDLHandle *h, s8 is_bot_band);
	
	/* src/android_asset_manager.c */
	SDL_Texture *android_load_texture(SDL_Renderer *renderer, const char* path);

	/* Macros for enabling and disabling the text field */
	#define ENABLE_TEXFIELD(_is_active_) android_show_keyboard(_is_active_)
	#define DISABLE_TEXTFIELD(_is_active_) android_hide_keyboard(_is_active_)
	#define DRAW_PIECE_KILL(_h_, _is_bot_, _is_black_) android_draw_piece_kill(_h_, _is_bot_, _is_black_)
	#define COMPUTE_WIN_BAND_SIZE(_h_) android_compute_win_size(_h_)
	#define BUILD_TIMER_RECT(_h_, _is_bot_) android_build_timer_rect(_h_, _is_bot_)
	#define BUILD_NAME_RECT(_h_, _is_bot_) android_build_name_rect(_h_, _is_bot_)
	#define BMP_TO_TEXTURE(_r_, _bmp_path_) android_load_texture(_r_, _bmp_path_)

#else 

	/* src/handle_textfield_keyboard.c */
	void pc_show_keyboard(s8 *is_active);
	void pc_hide_keyboard(s8 *is_active);
	
	/* src/display_piece_kill.c */
	void pc_draw_piece_kill(SDLHandle *h, s8 is_bot, s8 is_black);
	
	/* src/compute_win_elem_size.c */
	void pc_compute_win_size(SDLHandle *h);
	SDL_Rect pc_build_timer_rect(SDLHandle *h, s8 is_bot_band);
	SDL_Rect pc_build_name_rect(SDLHandle *h, s8 is_bot_band);
	
	/* src/sdl_handle.c */
	SDL_Texture *pc_load_texture(SDL_Renderer *renderer, const char* path);
	
	/* Macros for enabling and disabling the text field */
	#define ENABLE_TEXFIELD(_is_active_) 	pc_show_keyboard(_is_active_)
	#define DISABLE_TEXTFIELD(_is_active_)	pc_hide_keyboard(_is_active_)
	#define DRAW_PIECE_KILL(_h_, _is_bot_, _is_black_) pc_draw_piece_kill(_h_, _is_bot_, _is_black_)
	#define COMPUTE_WIN_BAND_SIZE(_h_) pc_compute_win_size(_h_)
	#define BUILD_TIMER_RECT(_h_, _is_bot_) pc_build_timer_rect(_h_, _is_bot_)
	#define BUILD_NAME_RECT(_h_, _is_bot_) pc_build_name_rect(_h_, _is_bot_)
	#define BMP_TO_TEXTURE(_r_, _bmp_path_) pc_load_texture(_r_, _bmp_path_)

#endif /* __ANDROID__ */

#endif /* _ANDROID_MACRO_HANDLE_SDL_H_ */