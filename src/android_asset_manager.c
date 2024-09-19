#include "../include/handle_sdl.h"
#include "../include/chess_log.h"

#ifdef __ANDROID__

/**
 * @file android_asset_manager.c
 * @brief This file contains functions for managing Android assets and integrating them with SDL.
 */

/* Global asset manager pointer */
AAssetManager* g_asset_manager = NULL;

/**
 * @brief Initializes the global asset manager.
 * 
 * This function is called from the SDLActivity to initialize the global asset manager pointer.
 * 
 * @param env The JNI environment pointer.
 * @param obj The Java object calling this function.
 * @param assetManagerObj The Java asset manager object.
 */
JNIEXPORT void JNICALL Java_org_libsdl_app_SDLActivity_nativeInit(JNIEnv* env, jobject obj, jobject assetManagerObj) {
    g_asset_manager = AAssetManager_fromJava(env, assetManagerObj);
}

/**
 * @brief Gets the size of an Android asset.
 * 
 * @param context The SDL_RWops context.
 * @return The size of the asset.
 */
Sint64 android_rwops_size(SDL_RWops* context) {
    return AAsset_getLength((AAsset*)context->hidden.unknown.data1);
}

/**
 * @brief Seeks to a position in an Android asset.
 * 
 * @param context The SDL_RWops context.
 * @param offset The offset to seek to.
 * @param whence The seek mode.
 * @return The new position in the asset.
 */
Sint64 android_rwops_seek(SDL_RWops* context, Sint64 offset, int whence) {
    return AAsset_seek((AAsset*)context->hidden.unknown.data1, offset, whence);
}

/**
 * @brief Reads data from an Android asset.
 * 
 * @param context The SDL_RWops context.
 * @param ptr The buffer to read data into.
 * @param size The size of each element to read.
 * @param maxnum The maximum number of elements to read.
 * @return The number of elements read.
 */
size_t android_rwops_read(SDL_RWops* context, void* ptr, size_t size, size_t maxnum) {
    return AAsset_read((AAsset*)context->hidden.unknown.data1, ptr, size * maxnum) / size;
}

/**
 * @brief Closes an Android asset.
 * 
 * @param context The SDL_RWops context.
 * @return 0 on success, or a negative error code on failure.
 */
int android_rwops_close(SDL_RWops* context) {
    AAsset_close((AAsset*)context->hidden.unknown.data1);
    SDL_FreeRW(context);
    return 0;
}

/**
 * @brief Creates an SDL_RWops structure from an Android asset.
 * 
 * @param asset The Android asset.
 * @return A pointer to the SDL_RWops structure, or NULL on failure.
 */
SDL_RWops* SDL_RWFromAsset(AAsset* asset) {
    if (!asset) {
        return NULL;
    }

    SDL_RWops* rw = SDL_AllocRW();
    if (!rw) {
        return NULL;
    }

    rw->size = android_rwops_size;
    rw->seek = android_rwops_seek;
    rw->read = android_rwops_read;
    rw->close = android_rwops_close;
    rw->hidden.unknown.data1 = asset;

    return rw;
}

/**
 * @brief Loads an asset file in read-only mode.
 * 
 * @param file The path to the asset file.
 * @return A pointer to the SDL_RWops structure, or NULL on failure.
 */
SDL_RWops* load_asset(const char* file) {
    AAsset* asset = AAssetManager_open(g_asset_manager, file, AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        CHESS_LOG(LOG_ERROR, "Failed to open asset: %s\n", file);
        return NULL;
    }

    SDL_RWops* rw = SDL_RWFromAsset(asset);
    if (rw == NULL) {
        CHESS_LOG(LOG_ERROR, "Failed to create RWops from asset: %s\n", file);
        AAsset_close(asset);
    }
    CHESS_LOG(LOG_INFO, "Load asset: |%s|\n", file);
    return rw;
}

/**
 * @brief Loads a texture from an Android asset.
 * 
 * @param renderer The SDL renderer.
 * @param path The path to the asset file.
 * @return A pointer to the SDL_Texture structure, or NULL on failure.
 */
SDL_Texture* android_load_texture(SDL_Renderer* renderer, const char* path) {
    SDL_Texture* texture = NULL;
    SDL_Surface* surface = NULL;
    SDL_RWops* rw = NULL;

    if (!renderer) {
        return NULL;
    }

    rw = load_asset(path);
    if (!rw) {
        CHESS_LOG(LOG_ERROR, "%s: load_asset failed\n", __func__);
        return NULL;
    }
    surface = SDL_LoadBMP_RW(rw, 1);
    if (!surface) {
        SDL_ERR_FUNC();
        return NULL;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_ERR_FUNC();
        return NULL;
    }
    SDL_FreeSurface(surface);
    return texture;
}

#endif