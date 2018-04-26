#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    if(x >= im.w) x = im.w - 1;
    if(y >= im.h) y = im.h - 1;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    assert(c >= 0);
    assert(c < im.c);
    return im.data[x + im.w*(y + im.h*c)];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    assert(c >= 0);
    assert(c < im.c);
    if(x >= 0 && x < im.w && y >= 0 && y < im.h){
        im.data[x + im.w*(y + im.h*c)] = v;
    }
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, im.w*im.h*im.c*sizeof(float));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    int i, j, k;
    image gray = make_image(im.w, im.h, 1);
    float scale[] = {0.299, 0.587, 0.114};
    for(k = 0; k < im.c; ++k){
        for(j = 0; j < im.h; ++j){
            for(i = 0; i < im.w; ++i){
                gray.data[i+im.w*j] += scale[k]*im.data[i + im.w*(j + im.h*k)];
            }
        }
    }
    return gray;
}

void scale_image(image im, int c, float v)
{
    int i;
    for(i = 0; i < im.w*im.h; ++i){
        im.data[i + c*im.w*im.h] *= v;
    }
}

void shift_image(image im, int c, float v)
{
    int i;
    for(i = 0; i < im.w*im.h; ++i){
        im.data[i + c*im.w*im.h] += v;
    }
}

void clamp_image(image im)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] = (im.data[i] < 0) ? 0 : ((im.data[i] > 1) ? 1 : im.data[i]);
    }
}

image grayscale_to_rgb(image im, float r, float g, float b)
{
    float color[3] = {r, g, b};
    assert(im.c == 1);
    int i, j, k;
    image rgb = make_image(im.w, im.h, 3);
    for(k = 0; k < 3; ++k){
        for(j = 0; j < im.h; ++j){
            for(i = 0; i < im.w; ++i){
                rgb.data[i + im.w*(j + im.h*k)] += color[k]*im.data[i+im.w*j];
            }
        }
    }
    return rgb;
}

image get_channel(image im, int c)
{
    int i,j;
    assert(c >= 0 && c < im.c);
    image chan = make_image(im.w, im.h, 1);
    for(j = 0; j < im.h; ++j){
        for(i = 0; i < im.w; ++i){
            chan.data[i+im.w*j] += im.data[i + im.w*(j + im.h*c)];
        }
    }
    return chan;
}

float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    assert(im.c == 3);
    int i, j;
    float r, g, b;
    float h, s, v;
    for(j = 0; j < im.h; ++j){
        for(i = 0; i < im.w; ++i){
            r = get_pixel(im, i , j, 0);
            g = get_pixel(im, i , j, 1);
            b = get_pixel(im, i , j, 2);
            float max = three_way_max(r,g,b);
            float min = three_way_min(r,g,b);
            float delta = max - min;
            v = max;
            if(max == 0){
                s = 0;
                h = 0;
            }else{
                s = delta/max;
                if(delta == 0){
                    h = 0;
                } else if(r == max){
                    h = (g - b) / delta;
                } else if (g == max) {
                    h = 2 + (b - r) / delta;
                } else {
                    h = 4 + (r - g) / delta;
                }
                if (h < 0) h += 6;
                h = h/6.;
            }
            set_pixel(im, i, j, 0, h);
            set_pixel(im, i, j, 1, s);
            set_pixel(im, i, j, 2, v);
        }
    }
}

void hsv_to_rgb(image im)
{
    assert(im.c == 3);
    int i, j;
    float r, g, b;
    float h, s, v;
    float f, p, q, t;
    for(j = 0; j < im.h; ++j){
        for(i = 0; i < im.w; ++i){
            h = 6 * get_pixel(im, i , j, 0);
            s = get_pixel(im, i , j, 1);
            v = get_pixel(im, i , j, 2);
            if (s == 0) {
                r = g = b = v;
            } else {
                int index = floor(h);
                f = h - index;
                p = v*(1-s);
                q = v*(1-s*f);
                t = v*(1-s*(1-f));
                if(index == 0){
                    r = v; g = t; b = p;
                } else if(index == 1){
                    r = q; g = v; b = p;
                } else if(index == 2){
                    r = p; g = v; b = t;
                } else if(index == 3){
                    r = p; g = q; b = v;
                } else if(index == 4){
                    r = t; g = p; b = v;
                } else {
                    r = v; g = p; b = q;
                }
            }
            set_pixel(im, i, j, 0, r);
            set_pixel(im, i, j, 1, g);
            set_pixel(im, i, j, 2, b);
        }
    }
}
