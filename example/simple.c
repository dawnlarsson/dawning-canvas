#define CANVAS_IMPL
#include "canvas.h"

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");

        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        canvas_color(window_1, (const float[]){0.0f, 1.0f, 0.4f, 1.0f});
        canvas_color(window_2, (const float[]){1.0f, 0.3f, 0.4f, 1.0f});

        while (canvas_update())
        {
        }

        return 0;
}