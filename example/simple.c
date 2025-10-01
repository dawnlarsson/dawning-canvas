#include <math.h>
#include "canvas.h"

#include <stdio.h>

void update(int window)
{
        printf("FPS: %.1f, Delta: %.6f\n", canvas_time.fps, canvas_time.delta);

        canvas_color(window, (float[]){(sin(canvas_time.delta) + 1.0f) * 0.5f, 0.0f, 0.0f, 1.0f});
}

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");
        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        return canvas_run(update);
}