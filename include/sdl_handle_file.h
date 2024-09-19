#ifndef _SDL_HANDLE_FILE_H_
#define _SDL_HANDLE_FILE_H_

/**
 * @file sdl_handle_file.h
 * @note This file is to provide a way to handle file operations using SDL_RWops.
 * It includes functions for opening, reading, writing, and closing files,
 * This file is include to handle_sdl.h.
 */

/**
 * @brief Opens a file using SDL_RWops.
 * 
 * On Android, it splits the file path to get the internal storage path.
 * 
 * @param file The path to the file to open.
 * @param mode The mode in which to open the file.
 * @return SDL_RWops* Pointer to the SDL_RWops structure, or NULL on failure.
 */
SDL_RWops* sdl_open(const char* file, const char* mode);

/**
 * @brief Reads data from an SDL_RWops file.
 * 
 * @param rw Pointer to the SDL_RWops structure.
 * @param buffer Buffer to store the read data.
 * @param size Size of each element to read.
 * @param maxnum Maximum number of elements to read.
 * @return size_t Number of elements read, or -1 on failure.
 */
size_t sdl_read(SDL_RWops* rw, void* buffer, size_t size, size_t maxnum);

/**
 * @brief Writes data to an SDL_RWops file.
 * 
 * @param rw Pointer to the SDL_RWops structure.
 * @param buffer Buffer containing the data to write.
 * @param size Size of each element to write.
 * @param num Number of elements to write.
 * @return size_t Number of elements written, or -1 on failure.
 */
size_t sdl_write(SDL_RWops* rw, const void* buffer, size_t size, size_t num);

/**
 * @brief Closes an SDL_RWops file.
 * 
 * @param rw Pointer to the SDL_RWops structure.
 * @return int 0 on success, -1 on failure.
 */
int sdl_close(SDL_RWops* rw);

/**
 * @brief Erases the data in a file by opening it in write mode.
 * 
 * @param filename The path to the file to erase.
 */
void sdl_erase_file_data(const char* filename);

/**
 * @brief Reads the entire content of a file into a buffer.
 * 
 * @param rw Pointer to the SDL_RWops structure.
 * @param buffer Buffer to store the read data.
 * @param total_size Total size of the data to read.
 * @return size_t Number of elements read, or -1 on failure.
 */
size_t sdl_read_complete_file(SDL_RWops *rw, void *buffer, size_t total_size);

/**
 * @brief Gets the size of a file.
 * 
 * @param filename The path to the file.
 * @return s64 Size of the file, or -1 on failure.
 */
s64 get_file_size_sdl(const char *filename);

#endif /* _SDL_HANDLE_FILE_H_ */