/* Basic SDL include */
#include "../rsc/lib/install/include/SDL2/SDL.h"
#include "../rsc/lib/install/include/SDL2/SDL_video.h"
#include "../rsc/lib/install/include/SDL2/SDL_render.h"

/* TTF include */
#include "../rsc/lib/install/include/SDL2/SDL_ttf.h"
#include <string.h>

#define BUFF_TEXT_SIZE 16

typedef struct {
    SDL_Rect rect;
    char text[BUFF_TEXT_SIZE];
	int buffer_size;
    int cursor;
    TTF_Font *font;
    SDL_Color color;
} TextField;

TextField *init_text_field(SDL_Rect rect, int buff_size_max, int font_size, SDL_Color color) {
	TextField *text_field = malloc(sizeof(TextField));

	text_field->rect = rect;
	text_field->buffer_size = buff_size_max;
	text_field->cursor = 0;
	text_field->font = TTF_OpenFont("rsc/font/arial.ttf", font_size);
	text_field->color = color;
	bzero(text_field->text, BUFF_TEXT_SIZE);
	return (text_field);
}

int is_accepted_char(SDL_Keycode type) {
	if (type >= SDLK_0 && type <= SDLK_9) // number
		return (1);
	else if (type == SDLK_PERIOD) // dot
		return (1);
	return (0);
}

void handle_text_input(SDL_Event *event, TextField *text_field) {
    if (event->type == SDL_KEYDOWN) {
        if (is_accepted_char(event->key.keysym.sym)) {
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
            printf("Text: %s\n", text_field->text);
        }
    }
}

void render_text_field(SDL_Renderer *renderer, TextField *text_field) {
    SDL_Surface *surface = TTF_RenderText_Blended(text_field->font, text_field->text, text_field->color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int text_width;
    TTF_SizeText(text_field->font, text_field->text, &text_width, NULL);

    SDL_Rect renderQuad = {text_field->rect.x + (text_field->rect.x >> 3), text_field->rect.y, text_width, text_field->rect.h};

    SDL_FreeSurface(surface);

    SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
    SDL_DestroyTexture(texture);
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("Text Input", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Rect rect = {100, 100, 200, 50};
    TextField *text_field = init_text_field(rect, BUFF_TEXT_SIZE, 24, (SDL_Color){255, 255, 255, 255});


    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
            handle_text_input(&event, text_field);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw rect
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &text_field->rect);

        render_text_field(renderer, text_field);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(text_field->font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}