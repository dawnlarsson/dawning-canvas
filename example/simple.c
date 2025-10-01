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
        canvas(100, 100, 500, 500, "1");
        canvas(150, 150, 500, 500, "2");
        canvas(200, 200, 500, 500, "3");
        canvas(250, 250, 500, 500, "4");
        canvas(300, 300, 500, 500, "5");
        canvas(350, 350, 500, 500, "6");
        canvas(400, 400, 500, 500, "7");
        canvas(450, 450, 500, 500, "8");

        return canvas_run(update);
}