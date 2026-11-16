#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <orbis/libkernel.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

extern int apply_external_image(const char *image_path);
extern void restore_default_theme(void);

// البحث التلقائي عن الصور في الفلاشة أو الذاكرة الداخلية
int find_external_image(char *out_path, size_t max_len) {
    const char *search_dirs[] = { "/mnt/usb0/custom_bg", "/data/custom_bg" };
    
    for (int i = 0; i < 2; i++) {
        DIR *dir = opendir(search_dirs[i]);
        if (!dir) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, ".png") || strstr(entry->d_name, ".jpg") || strstr(entry->d_name, ".jpeg")) {
                snprintf(out_path, max_len, "%s/%s", search_dirs[i], entry->d_name);
                closedir(dir);
                return 1;
            }
        }
        closedir(dir);
    }
    return 0;
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    SDL_Window *window = SDL_CreateWindow("PS4 Theme Hub", 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_GameController *pad = SDL_GameControllerOpen(0);

    char external_img_path[512] = {0};
    SDL_Texture *preview_tex = NULL;

    if (find_external_image(external_img_path, sizeof(external_img_path))) {
        SDL_Surface *surf = IMG_Load(external_img_path);
        if (surf) {
            preview_tex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_X: // زر X: تطبيق الصورة المخصصة
                        if (strlen(external_img_path) > 0) {
                            if (apply_external_image(external_img_path) == 0) {
                                sceKernelIccSetBuzzer(1); // صوت تنبيه عند النجاح
                            }
                        }
                        break;
                    case SDL_CONTROLLER_BUTTON_Y: // زر المثلث: استعادة الافتراضي
                        restore_default_theme();
                        sceKernelIccSetBuzzer(2);
                        break;
                    case SDL_CONTROLLER_BUTTON_B: // زر الدائرة: خروج
                        running = 0;
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 18, 22, 30, 255);
        SDL_RenderClear(renderer);

        if (preview_tex) {
            SDL_Rect preview_rect = { 320, 180, 1280, 720 };
            SDL_RenderCopy(renderer, preview_tex, NULL, &preview_rect);
        }

        SDL_RenderPresent(renderer);
    }

    if (preview_tex) SDL_DestroyTexture(preview_tex);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
