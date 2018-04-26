#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"

image add_image(image a, image b)
{
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    int i;
    image sum = make_image(a.w, a.h, a.c);
    for(i = 0; i < a.w*a.h*a.c; ++i){
        sum.data[i] = a.data[i] + b.data[i];
    }
    return sum;
}

image sub_image(image a, image b)
{
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    int i;
    image sum = make_image(a.w, a.h, a.c);
    for(i = 0; i < a.w*a.h*a.c; ++i){
        sum.data[i] = a.data[i] - b.data[i];
    }
    return sum;
}

void minimum_image(image im, float thresh)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] = im.data[i] > thresh ? im.data[i]  : 0;
    }
}

void feature_normalize(image im)
{
    int i;
    float min = im.data[0];
    float max = im.data[0];
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if(im.data[i] > max) max = im.data[i];
        if(im.data[i] < min) min = im.data[i];
    }
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] = (im.data[i] - min)/(max-min);
    }
}

void l1_normalize(image im)
{
    int i; 
    float sum = 0;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        sum += im.data[i];
    }
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] /= sum;
    }
}

void l2_normalize(image im)
{
    int i; 
    float sum = 0;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        sum += im.data[i]*im.data[i];
    }
    sum = sqrt(sum);
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] /= sum;
    }
}

image make_emboss_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -2;
    f.data[1] = -1;
    f.data[2] = 0;
    f.data[3] = -1;
    f.data[4] = 1;
    f.data[5] = 1;
    f.data[6] = 0;
    f.data[7] = 1;
    f.data[8] = 2;
    return f;
}

image make_horz_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1;
    f.data[1] = -1;
    f.data[2] = -1;
    f.data[3] = 2;
    f.data[4] = 2;
    f.data[5] = 2;
    f.data[6] = -1;
    f.data[7] = -1;
    f.data[8] = -1;
    return f;
}

image make_highpass_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = 0;
    f.data[1] = -1;
    f.data[2] = 0;
    f.data[3] = -1;
    f.data[4] = 4;
    f.data[5] = -1;
    f.data[6] = 0;
    f.data[7] = -1;
    f.data[8] = 0;
    return f;
}

image make_box_filter(int w)
{
    image f = make_image(w,w,1);
    int i;
    for(i = 0; i < w*w; ++i){
        f.data[i] = 1;
    }
    l1_normalize(f);
    return f;
}

void print_image(image im)
{
    int i,j,k;
    for(k = 0; k < im.c; ++k){
        printf("\nChannel %d\n", k);
        for(j = 0; j < im.h; ++j){
            for(i = 0; i < im.w; ++i){
                printf("%5.3f ", im.data[i+im.w*(j + im.h*k)]);
            }
            printf("\n");
        }
    }
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO


image make_gx_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1;
    f.data[1] = 0;
    f.data[2] = 1;
    f.data[3] = -2;
    f.data[4] = 0;
    f.data[5] = 2;
    f.data[6] = -1;
    f.data[7] = 0;
    f.data[8] = 1;
    return f;
}

image make_gy_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = -1;
    f.data[1] = -2;
    f.data[2] = -1;
    f.data[3] = 0;
    f.data[4] = 0;
    f.data[5] = 0;
    f.data[6] = 1;
    f.data[7] = 2;
    f.data[8] = 1;
    return f;
}

image make_sharpen_filter()
{
    image f = make_image(3,3,1);
    f.data[0] = 0;
    f.data[1] = -1;
    f.data[2] = 0;
    f.data[3] = -1;
    f.data[4] = 1+4;
    f.data[5] = -1;
    f.data[6] = 0;
    f.data[7] = -1;
    f.data[8] = 0;
    return f;
}

image make_gaussian_filter(float sigma)
{
    int w = ((int)(6*sigma))|1;
    int i,j;
    image f = make_image(w, w, 1);
    for(i = 0; i < w; ++i){
        for(j = 0; j < w; ++j){
            float x = w / 2. - i - .5;
            float y = w / 2. - j - .5;
            float val = 1./(TWOPI*sigma*sigma) * exp(-(x*x + y*y)/(2*sigma*sigma));
            set_pixel(f, i, j, 0, val);
        }
    }
    l1_normalize(f);
    return f;
}

image convolve_image(image im, image filter, int preserve)
{
    image out = make_image(im.w, im.h, preserve ? im.c : 1);
    assert(im.c == filter.c || filter.c == 1);
    int i, j, k, dx, dy;
    for(k = 0; k < im.c; ++k){
        for(j = 0; j < im.h; ++j){
            for(i = 0; i < im.w; ++i){
                for(dy = 0; dy < filter.h; ++dy){
                    for(dx = 0; dx < filter.w; ++dx){
                        float weight = get_pixel(filter, dx, dy, (filter.c == 1) ? 0 : k);
                        float val = get_pixel(im, i+dx-filter.w/2, j+dy-filter.h/2, k);
                        if(preserve) out.data[i + im.w*(j+im.h*k)] += val*weight;
                        else         out.data[i + im.w*j] += val*weight;
                    }
                }
            }
        }
    }
    return out;
}

image colorize_sobel(image im)
{
    image *s = sobel_image(im);
    feature_normalize(s[0]);
    feature_normalize(s[1]);
    image r = make_image(im.w, im.h, 3);
    memcpy(r.data, s[1].data, im.w*im.h*sizeof(float));
    memcpy(r.data+im.w*im.h, s[0].data, im.w*im.h*sizeof(float));
    memcpy(r.data+2*im.w*im.h, s[0].data, im.w*im.h*sizeof(float));
    free_image(s[0]);
    free_image(s[1]);
    hsv_to_rgb(r);
    return r;
}

image *sobel_image(image im)
{
    image *ims = calloc(2, sizeof(image));
    image g = make_image(im.w, im.h, 1);
    image d = make_image(im.w, im.h, 1);
    ims[0] = g;
    ims[1] = d;
    image xf = make_gx_filter();
    image yf = make_gy_filter();
    image gx = convolve_image(im, xf, 0);
    image gy = convolve_image(im, yf, 0);
    int i;
    for(i = 0; i < im.w*im.h; ++i){
        g.data[i] = sqrt(gx.data[i]*gx.data[i] + gy.data[i]*gy.data[i]);
        d.data[i] = atan2(gy.data[i], gx.data[i]);
    }
    free_image(xf);
    free_image(yf);
    free_image(gx);
    free_image(gy);
    return ims;
}

