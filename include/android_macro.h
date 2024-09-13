#include "../include/handle_sdl.h"

#ifndef _ANDROID_MACRO_HANDLE_SDL_H_
#define _ANDROID_MACRO_HANDLE_SDL_H_

#ifdef __ANDROID__

    JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_showKeyboard(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_hideKeyboard(JNIEnv* env, jobject obj);

	FT_INLINE void android_show_keyboard(s8 *is_active) {
		JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
		jobject activity = (jobject) SDL_AndroidGetActivity();
		jclass clazz = (*env)->GetObjectClass(env, activity);
		jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "showKeyboard", "()V");

		(*env)->CallStaticVoidMethod(env, clazz, method_id);
		(*env)->DeleteLocalRef(env, activity);
		(*env)->DeleteLocalRef(env, clazz);
		*is_active = TRUE;
	}

	FT_INLINE void android_hide_keyboard(s8 *is_active) {
		JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
		jobject activity = (jobject) SDL_AndroidGetActivity();
		jclass clazz = (*env)->GetObjectClass(env, activity);
		jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "hideKeyboard", "()V");

		(*env)->CallStaticVoidMethod(env, clazz, method_id);
		(*env)->DeleteLocalRef(env, activity);
		(*env)->DeleteLocalRef(env, clazz);
		*is_active = FALSE;
	}

	#define ENABLE_TEXFIELD(_is_active_) android_show_keyboard(_is_active_)
	#define DISABLE_TEXTFIELD(_is_active_) android_hide_keyboard(_is_active_)

#else 

	FT_INLINE void pc_show_keyboard(s8 *is_active) {
		*is_active = TRUE;
	}
	FT_INLINE void pc_hide_keyboard(s8 *is_active) {
		*is_active = FALSE;
	}
	#define ENABLE_TEXFIELD(_is_active_) 	pc_show_keyboard(_is_active_)
	#define DISABLE_TEXTFIELD(_is_active_)	pc_hide_keyboard(_is_active_)

#endif // __ANDROID__

#endif // _ANDROID_MACRO_HANDLE_SDL_H_