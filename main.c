#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <float.h>

#include <string.h>

#include <X11/Xlib.h>

#define HEIGHT 1000
#define WIDTH 1000

int8_t in_bounds(int32_t, int32_t, int64_t, int64_t);
void gc_put_pixel(void *, int32_t, int32_t, uint32_t);
void update(Display *, GC *, Window *, XImage *);

uint32_t decodeRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) + (g << 8) + b;
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) / (in_max - in_min) * (out_max - out_min) + out_min;
}

float x = 0;
float y = 0;

int8_t exitloop = 0;
int8_t auto_update = 0;

int main(void)
{
    Display *display = XOpenDisplay(NULL);

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        0, 0,
        WIDTH, HEIGHT,
        0, 0,
        0
    );

    char *memory = (char *) malloc(sizeof(uint32_t)*HEIGHT*WIDTH);

    XWindowAttributes winattr = {0};
    XGetWindowAttributes(display, window, &winattr);

    XImage *image = XCreateImage(
        display, winattr.visual, winattr.depth,
        ZPixmap, 0, memory,
        WIDTH, HEIGHT,
        32, WIDTH*4
    );

    GC graphics = XCreateGC(display, window, 0, NULL);

    Atom delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &delete_window, 1);

    Mask key = KeyPressMask | KeyReleaseMask;
    XSelectInput(display, window, ExposureMask | key);

    XMapWindow(display, window);
    XSync(display, False);

    XEvent event;

    srand(time(NULL));

    update(display, &graphics, &window, image);

    while(!exitloop) {
        while(XPending(display) > 0) {
            XNextEvent(display, &event);
            switch(event.type) {
                case Expose: {
                    update(display, &graphics, &window, image);
                    break;
                }
                case ClientMessage: {
                    if((Atom) event.xclient.data.l[0] == delete_window) {
                        exitloop = 1;   
                    }
                    break;
                }
                case KeyPress: {
                    if(event.xkey.keycode == 0x24) {
                        // Draw Once
                        update(display, &graphics, &window, image);
                    }
                    if(event.xkey.keycode == 0x41) {
                        auto_update = !auto_update;
                    }
                    break;
                }
            }
        }

        if(auto_update) {
            float r = ((float)rand()/(float)(RAND_MAX));
            if(r < 0.01) {
                x = 0.0;
                y = 0.16 * y;
            } else if(r < 0.86) {
                x = 0.85 * x + 0.04 * y;
                y = -0.04 * x + 0.85 * y + 1.6;
            } else if(r < 0.93) {
                x = 0.2 * x - 0.26 * y;
                y = 0.23 * x + 0.22 * y + 1.6;
            } else {
                x = -0.15 * x + 0.28 * y;
                y = 0.26 * x + 0.24 * y + 0.44;
            }
            int px = (int) map(x, -2.1820, 2.6558, 0, WIDTH);
            int py = (int) map(y, 0, 9.9983, HEIGHT, 0);
            gc_put_pixel(memory, px, py, 0x0000FF00);
            update(display, &graphics, &window, image);
        }
    }


    XCloseDisplay(display);

    free(memory);

    return 0;
}

void update(Display *display, GC *graphics, Window *window, XImage *image)
{
    XPutImage(
        display,
        *window,
        *graphics,
        image,
        0, 0,
        0, 0,
        WIDTH, HEIGHT
    );

    XSync(display, False);
}

int8_t in_bounds(int32_t x, int32_t y, int64_t w, int64_t h)
{
    return (x >= 0 && x < w && y >= 0 && y < h);
}

void gc_put_pixel(void *memory, int32_t x, int32_t y, uint32_t color)
{
    if(in_bounds(x, y, WIDTH, HEIGHT))
        *((uint32_t *) memory + y * WIDTH + x) = color;
}


