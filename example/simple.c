#include "canvas.h"
#include <math.h>

void update(int window)
{
        float t = canvas_main_time.current + window;

        float r = (sinf(t * 0.5f) + 1.0f) * 0.5f;
        float g = (sinf(t * 0.7f + 2.0f) + 1.0f) * 0.5f;
        float b = (sinf(t * 0.9f + 4.0f) + 1.0f) * 0.5f;

        canvas_color(window, (float[]){r, g, b, 1.0f});
}

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");
        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        return canvas_run(update);
}