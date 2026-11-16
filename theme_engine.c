#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <unistd.h>

// دالة مساعدة لنسخ الملفات
int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }
    fclose(in);
    fclose(out);
    return 0;
}

// تحويل وتعيين أي صورة خارجية (JPG أو PNG) كخلفية للنظام بدقة 1920x1080
int apply_external_image(const char *image_path) {
    SDL_Surface *loaded = IMG_Load(image_path);
    if (!loaded) return -1;

    // تجهيز سطح جديد بدقة شاشة PS4 القياسية
    SDL_Surface *formatted = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
    if (!formatted) {
        SDL_FreeSurface(loaded);
        return -2;
    }

    // مطابقة وتوسيط الصورة على الشاشة
    SDL_BlitScaled(loaded, NULL, formatted, NULL);

    // أخذ نسخة احتياطية من خلفية النظام الافتراضية أول مرة
    if (access("/user/system/theme_backup.png", F_OK) != 0) {
        copy_file("/user/home/10000000/theme/wallpaper.png", "/user/system/theme_backup.png");
    }

    // حفظ الصورة بصيغة PNG في مسار خلفية النظام
    int result = IMG_SavePNG(formatted, "/user/home/10000000/theme/wallpaper.png");

    SDL_FreeSurface(loaded);
    SDL_FreeSurface(formatted);
    return result;
}

// استعادة الخلفية الافتراضية
void restore_default_theme(void) {
    if (access("/user/system/theme_backup.png", F_OK) == 0) {
        copy_file("/user/system/theme_backup.png", "/user/home/10000000/theme/wallpaper.png");
    }
}
