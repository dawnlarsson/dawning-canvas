#include "canvas.h"
#include <math.h>

#define NUM_WINDOWS 8
#define ORBIT_RADIUS 160.0f
#define WINDOW_SIZE 96

void update(int window)
{
        canvas_pointer *mouse = canvas_get_pointer(0);

        float t = canvas_info.time.current;
        float offset = window * (3.14159f * 2.0f / NUM_WINDOWS);

        float r = (sinf(t * 0.8f + offset) + 1.0f) * 0.5f;
        float g = (sinf(t * 1.2f + offset + 2.0f) + 1.0f) * 0.5f;
        float b = (sinf(t * 1.5f + offset + 4.0f) + 1.0f) * 0.5f;

        canvas_color(window, (float[]){r, g, b, 1.0f});

        if (mouse && mouse->active)
        {
                // Each window gets a position in a circle around the cursor
                float angle = (t * 0.5f) + offset;

                // Calculate orbit position relative to cursor
                int center_x = mouse->screen_x;
                int center_y = mouse->screen_y;

                int target_x = center_x + (int)(ORBIT_RADIUS * cosf(angle)) - WINDOW_SIZE / 2;
                int target_y = center_y + (int)(ORBIT_RADIUS * sinf(angle)) - WINDOW_SIZE / 2;

                // Smooth movement - lerp toward target position
                int current_x = canvas_info.canvas[window].x;
                int current_y = canvas_info.canvas[window].y;

                float lerp_speed = 5.0f * canvas_info.time.delta;
                if (lerp_speed > 1.0f)
                        lerp_speed = 1.0f;

                int new_x = (int)(current_x + (target_x - current_x) * lerp_speed);
                int new_y = (int)(current_y + (target_y - current_y) * lerp_speed);

                canvas_set(window, mouse->display, new_x, new_y, WINDOW_SIZE, WINDOW_SIZE, NULL);
        }
        else
        {
                // Fallback: circular motion around screen center if no mouse data
                float angle = (t * 0.5f) + offset;

                int screen_w = canvas_info.display[0].width;
                int screen_h = canvas_info.display[0].height;

                int center_x = screen_w / 2;
                int center_y = screen_h / 2;

                int x = center_x + (int)(ORBIT_RADIUS * cosf(angle)) - WINDOW_SIZE / 2;
                int y = center_y + (int)(ORBIT_RADIUS * sinf(angle)) - WINDOW_SIZE / 2;

                canvas_set(window, 0, x, y, WINDOW_SIZE, WINDOW_SIZE, NULL);
        }
}

int main()
{
        // Create windows in a circle initially
        for (int i = 0; i < NUM_WINDOWS; i++)
        {
                float angle = i * (3.14159f * 2.0f / NUM_WINDOWS);

                int start_x = 400 + (int)(100.0f * cosf(angle));
                int start_y = 300 + (int)(100.0f * sinf(angle));

                char title[32];
                snprintf(title, sizeof(title), "Orbit %d", i + 1);

                canvas(start_x, start_y, WINDOW_SIZE, WINDOW_SIZE, title);
        }

        return canvas_run(update);
}