/**
 * @file sdl_handle_file.c
 * @brief SDL file handling functions for the chess application.
 * 
 * This file contains functions to handle file operations using SDL_RWops.
 * It includes functions for opening, reading, writing, and closing files,
 * as well as some utility functions for file operations specific to the 
 * Android platform.
 */

#include "../include/handle_sdl.h"

#ifdef __ANDROID__
	/**
	 * @brief Retrieves the internal storage path for a given file.
	 *
	 * @param file The file name.
	 * @return The full path to the file in the internal storage.
	 */
	static char *get_internal_storage_path(const char* file) {
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
#endif

SDL_RWops* sdl_open(const char* file, const char* mode) {
#ifdef __ANDROID__
	char **split = ft_split(file, '/');
	char* fullPath = get_internal_storage_path(split[1]);
	SDL_RWops* rw = SDL_RWFromFile(fullPath, mode);
	if (!rw) {
		CHESS_LOG(LOG_ERROR, RED"Failed to open file %s: %s\n"RESET, fullPath, SDL_GetError());
	}
	free_double_char(split);
	free(fullPath);
	return (rw);
#else
    SDL_RWops* rw = SDL_RWFromFile(file, mode);
    if (!rw) {
        CHESS_LOG(LOG_ERROR, "Failed to open file %s: %s\n", file, SDL_GetError());
    }
    return (rw);
#endif
}

size_t sdl_read(SDL_RWops* rw, void* buffer, size_t size, size_t maxnum) {
    size_t num_read = SDL_RWread(rw, buffer, size, maxnum);
    if (num_read == 0 && SDL_RWseek(rw, 0, RW_SEEK_CUR) != SDL_RWseek(rw, 0, RW_SEEK_END)) {
        fprintf(stderr, "Failed to read from file: %s\n", SDL_GetError());
		return (-1);
	}
    return (num_read);
}

size_t sdl_write(SDL_RWops* rw, const void* buffer, size_t size, size_t num) {
    size_t num_written = SDL_RWwrite(rw, buffer, size, num);
    if (num_written != num) {
        fprintf(stderr, "Failed to write to file: %s\n", SDL_GetError());
		return (-1);
	}
    return (num_written);
}

int sdl_close(SDL_RWops* rw) {
    if (SDL_RWclose(rw) != 0) {
        fprintf(stderr, "Failed to close file: %s\n", SDL_GetError());
        return (-1);
    }
    return (0);
}

void sdl_erase_file_data(const char* filename) {
	SDL_RWops* rw = sdl_open(filename, "w");
	if (!rw) {
		fprintf(stderr, "Failed to open file %s: %s\n", filename, SDL_GetError());
		return;
	}
	SDL_RWclose(rw);
}

size_t sdl_read_complete_file(SDL_RWops *rw, void *buffer, size_t total_size) {
	size_t num_read = SDL_RWread(rw, buffer, 1, total_size);
	if (num_read != total_size) {
		fprintf(stderr, "Failed to read complete file: %s\n", SDL_GetError());
		return (-1);
	}
	return (num_read);
}

s64 get_file_size_sdl(const char *filename) {
	SDL_RWops* rw = sdl_open(filename, "rb");
    if (!rw) {
        fprintf(stderr, "Cannot open file %s: %s\n", filename, SDL_GetError());
        return (-1);
    }
    s64 size = SDL_RWseek(rw, 0, RW_SEEK_END);
    SDL_RWclose(rw);
    return (size);
}