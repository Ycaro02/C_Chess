#include "../include/handle_sdl.h"

#ifndef _ANDROID_MACRO_HANDLE_SDL_H_
#define _ANDROID_MACRO_HANDLE_SDL_H_

#ifdef __ANDROID__

	// Get internal storage path
	/**
	 * @file android_macro.h
	 * @brief This file contains inline functions and macros for handling Android-specific operations such as retrieving internal storage paths and showing/hiding the keyboard.
	 *
	 * The functions in this file utilize JNI (Java Native Interface) to interact with Android's Java environment.
	 *
	 * Functions:
	 * - get_internal_storage_path: Retrieves the internal storage path for a given file.
	 * - android_show_keyboard: Shows the Android keyboard.
	 * - android_hide_keyboard: Hides the Android keyboard.
	 * - pc_show_keyboard: Dummy function to simulate showing the keyboard on non-Android platforms.
	 * - pc_hide_keyboard: Dummy function to simulate hiding the keyboard on non-Android platforms.
	 *
	 * Macros:
	 * - ENABLE_TEXFIELD: Macro to enable the text field (show the keyboard).
	 * - DISABLE_TEXTFIELD: Macro to disable the text field (hide the keyboard).
	 *
	 * JNIEXPORT Declarations:
	 * - Java_org_libsdl_app_SDLActivity_showKeyboard: JNI function to show the keyboard.
	 * - Java_org_libsdl_app_SDLActivity_hideKeyboard: JNI function to hide the keyboard.
	 */
	FT_INLINE char *get_internal_storage_path(const char* file) {
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity = (jobject)SDL_AndroidGetActivity();
		jclass activityClass = (*env)->GetObjectClass(env, activity);

		jmethodID methodID = (*env)->GetMethodID(env, activityClass, "getFilesDir", "()Ljava/io/File;");
		jobject fileObject = (*env)->CallObjectMethod(env, activity, methodID);
		jclass fileClass = (*env)->GetObjectClass(env, fileObject);

		jmethodID getPathMethodID = (*env)->GetMethodID(env, fileClass, "getPath", "()Ljava/lang/String;");
		jstring pathString = (jstring)(*env)->CallObjectMethod(env, fileObject, getPathMethodID);

		const char* basePath = (*env)->GetStringUTFChars(env, pathString, NULL);
		size_t fullPathLength = strlen(basePath) + strlen(file) + 2;
		char* fullPath = (char*)malloc(fullPathLength);
		snprintf(fullPath, fullPathLength, "%s/%s", basePath, file);

		(*env)->ReleaseStringUTFChars(env, pathString, basePath);
		(*env)->DeleteLocalRef(env, pathString);
		(*env)->DeleteLocalRef(env, fileObject);
		(*env)->DeleteLocalRef(env, fileClass);
		(*env)->DeleteLocalRef(env, activityClass);
		(*env)->DeleteLocalRef(env, activity);

		CHESS_LOG(LOG_INFO, ORANGE"Internal storage path: %s\n"RESET, fullPath);
		return (fullPath);
	}

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