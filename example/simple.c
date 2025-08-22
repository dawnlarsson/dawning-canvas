#define CANVAS_IMPL
#include "canvas.h"

int main()
{
        __canvas_window(400, 400, 600, 600, "My Window");

        while (canvas_update())
        {
        }

        return 0;
}