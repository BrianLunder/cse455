#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    int lx = (int) round(x);
    int ly = (int) round(y);
    float v00 = get_pixel(im, lx, ly, c);
    return v00;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    int lx = (int) floor(x);
    int ly = (int) floor(y);
    float dx = x - lx;
    float dy = y - ly;
    float v00 = get_pixel(im, lx, ly, c);
    float v10 = get_pixel(im, lx+1, ly, c);
    float v01 = get_pixel(im, lx, ly+1, c);
    float v11 = get_pixel(im, lx+1, ly+1, c);
    float v =   v00*(1-dx)*(1-dy) + v10*dx*(1-dy) + 
                v01*(1-dx)*dy + v11*dx*dy;
    return v;
}

image bilinear_resize(image im, int w, int h)
{
    image r = make_image(w, h, im.c);   
    float xscale = (float)im.w/w;
    float yscale = (float)im.h/h;
    int i, j, k;
    for(k = 0; k < im.c; ++k){
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                float y = (j+.5)*yscale - .5;
                float x = (i+.5)*xscale - .5;
                float val = bilinear_interpolate(im, x, y, k);
                set_pixel(r, i, j, k, val);
            }
        }
    }
    return r;
}

image nn_resize(image im, int w, int h)
{
    image r = make_image(w, h, im.c);   
    float xscale = (float)im.w/w;
    float yscale = (float)im.h/h;
    int i, j, k;
    for(k = 0; k < im.c; ++k){
        for(j = 0; j < h; ++j){
            for(i = 0; i < w; ++i){
                float y = (j+.5)*yscale - .5;
                float x = (i+.5)*xscale - .5;
                float val = nn_interpolate(im, x, y, k);
                set_pixel(r, i, j, k, val);
            }
        }
    }
    return r;
}

