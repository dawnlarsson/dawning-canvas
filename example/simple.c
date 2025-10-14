#include "canvas.h"
#include <math.h>

void update(int window)
{
        float t = canvas_info.time.current + window;

        float r = (sinf(t * 0.5f) + 1.0f) * 0.5f;
        float g = (sinf(t * 0.7f + 2.0f) + 1.0f) * 0.5f;
        float b = (sinf(t * 0.9f + 4.0f) + 1.0f) * 0.5f;

        canvas_color(window, (float[]){r, g, b, 1.0f});

        // don't move if the user has moved / resized the window
        if (canvas_info.canvas[window].os_moved || canvas_info.canvas[window].os_resized)
                return;

        canvas_set(window, 0,
                   200 + (int)(200.0f * sinf(t * 0.6f)),
                   200 + (int)(200.0f * cosf(t * 0.6f)),
                   128 + (int)(32.0f * sinf(t * 1.0f)),
                   128 + (int)(32.0f * sinf(t * 1.0f)),
                   "Moving Window");
}

int main()
{
        canvas(-1, -1, 128, 128, "1");
        canvas(150, 150, 128, 128, "2");
        canvas(200, 200, 128, 128, "3");
        canvas(250, 250, 128, 128, "4");
        canvas(300, 300, 128, 128, "5");
        canvas(350, 350, 128, 128, "6");
        canvas(400, 400, 128, 128, "7");
        canvas(450, 450, 128, 128, "8");

        return canvas_run(update);
}