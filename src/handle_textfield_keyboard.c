#include "../include/handle_sdl.h"

#ifdef __ANDROID__

	/* Declaration for showing and hiding the keyboard java function */
	JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_showKeyboard(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_hideKeyboard(JNIEnv* env, jobject obj);

	/**
	 * @brief Shows the Android keyboard.
	 *
	 * @param is_active Pointer to a boolean value that indicates whether the keyboard is active.
	 */
	void android_show_keyboard(s8 *is_active) {
		JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
		jobject activity = (jobject) SDL_AndroidGetActivity();
		jclass clazz = (*env)->GetObjectClass(env, activity);
		jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "showKeyboard", "()V");

		(*env)->CallStaticVoidMethod(env, clazz, method_id);
		(*env)->DeleteLocalRef(env, activity);
		(*env)->DeleteLocalRef(env, clazz);
		*is_active = TRUE;
	}

	/**
	 * @brief Hides the Android keyboard.
	 *
	 * @param is_active Pointer to a boolean value that indicates whether the keyboard is active.
	 */
	void android_hide_keyboard(s8 *is_active) {
		JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
		jobject activity = (jobject) SDL_AndroidGetActivity();
		jclass clazz = (*env)->GetObjectClass(env, activity);
		jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "hideKeyboard", "()V");

		(*env)->CallStaticVoidMethod(env, clazz, method_id);
		(*env)->DeleteLocalRef(env, activity);
		(*env)->DeleteLocalRef(env, clazz);
		*is_active = FALSE;
	}

#else

	/**
	 * @brief Dummy function to simulate showing the keyboard on non-Android platforms.
	 * @param is_active Pointer to a boolean value that indicates whether the keyboard is active.
	 */
	void pc_show_keyboard(s8 *is_active) {
		*is_active = TRUE;
	}

	/**
	 * @brief Dummy function to simulate hiding the keyboard on non-Android platforms.
	 * @param is_active Pointer to a boolean value that indicates whether the keyboard is active.
	 */
	void pc_hide_keyboard(s8 *is_active) {
		*is_active = FALSE;
	}


#endif // __ANDROID__