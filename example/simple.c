#include <math.h>
#include "canvas.h"

int main()
{
        int window_1 = canvas(400, 400, 600, 600, "My Window");

        int window_2 = canvas(500, 500, 600, 600, "My Window 2");

        int time = 0;
        while (canvas_update())
        {
                canvas_color(window_1, (float[]){1.0, sin(time * 0.1), 0.0, 1.0});
                canvas_color(window_2, (float[]){1.0, sin(time * 0.01), sin(time * 0.2), 1.0});
                time++;
        }

        return 0;
}