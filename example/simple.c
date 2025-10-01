#include "canvas.h"

void update(int window)
{
        canvas_color(window, (float[]){window, 1.0f, 0.0f, 1.0f});
}

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");
        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        return canvas_run(update);
}