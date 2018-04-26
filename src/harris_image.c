#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>
#include "stb_image.h"



// Utility methods
void printaf(char * toPrint) {
  printf(toPrint);
  printf("\n");
  fflush(stdout);
}

matrix matrix_from_image(image im, int transpose) {
  matrix res;
  //image gray = rgb_to_grayscale(im);
  if (transpose) {
    res = make_matrix(im.h, im.w);
    for (int i = 0; i < im.w; i++){
      for (int j = 0; j < im.h; j++) {
        res.data[j][i] = (double)get_pixel(im, i, j, 0);
      }
    }
  } else {
    res = make_matrix(im.w, im.h);
    for (int i = 0; i < im.w; i++) {
      for (int j = 0; j < im.h; j++) {
        res.data[i][j] = (double)get_pixel(im, i, j, 0);
      }
    }
  }
  return res;
}

double determinant(matrix m) {
  double res = 0.0f;
  if (m.rows == m.cols && m.rows >= 2) {
    if (m.rows == 2) {
      res = m.data[0][0] * m.data[1][1] - m.data[1][0] * m.data[0][1];

    } else  {
      printaf("non 2x2 determinant called");
      int sign = 1;
      for (int i = 0; i < m.rows; i++) {
        matrix minor = make_matrix(m.rows - 1, m.cols - 1);
        for (int mi = 0; mi < minor.rows; mi++) {
          for (int mj = 0; mj < minor.cols; mj++) {
            minor.data[mj][mi] = m.data[1 + mj][mi < i ? mi : mi + 1];
          }
        }
        res += sign * m.data[0][i] * determinant(minor);
        free_matrix(minor);
        sign = sign * -1;
      }
    }
  } else {
    fprintf(stderr, "Matrix not square\n");
  }

  return res;
}

double trace(matrix m) {
  double res = 0.0f;
  if (m.rows != m.cols) {
    fprintf(stderr, "Matrix not square\n");
    return 0;
  }
  for (int i = 0; i < m.rows; i++) {
    res += m.data[i][i];
  }
  return res;
}



// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i%im.w;
    d.p.y = i/im.w;
    d.data = calloc(w*w*im.c, sizeof(float));
    d.n = w*w*im.c;
    int c, dx, dy;
    int count = 0;
    // If you want you can experiment with other descriptors
    // This subtracts the central value from neighbors
    // to compensate some for exposure/lighting changes.
    for(c = 0; c < im.c; ++c){
        float cval = im.data[c*im.w*im.h + i];
        for(dx = -w/2; dx < (w+1)/2; ++dx){
            for(dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(im, i%im.w+dx, i/im.w+dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for(i = -9; i < 10; ++i){
        set_pixel(im, x+i, y, 0, 1);
        set_pixel(im, x, y+i, 0, 1);
        set_pixel(im, x+i, y, 1, 0);
        set_pixel(im, x, y+i, 1, 0);
        set_pixel(im, x+i, y, 2, 1);
        set_pixel(im, x, y+i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma)
{
    // TODO: optional, make separable 1d Gaussian.
    return make_image(1,1,1);
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma)
{
    if(1){
        image g = make_gaussian_filter(sigma);
        image s = convolve_image(im, g, 1);
        free_image(g);
        return s;
    } else {
        // TODO: optional, use two convolutions with 1d gaussian filter.
        // If you implement, disable the above if check.
        return copy_image(im);
    }
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{
    image S = make_image(im.w, im.h, 3);
    image gxFilter = make_gx_filter();
    image gyFilter = make_gy_filter();
    image gx = convolve_image(im, gxFilter, 0);
    image gy = convolve_image(im, gyFilter, 0);
    float max = -999999999;
    for (int i = 0; i < im.w; i++) {
      for (int j = 0; j < im.h; j++) {
        set_pixel(S, i, j, 0, powf(get_pixel(gx, i, j, 0), 2));
        set_pixel(S, i, j, 1, powf(get_pixel(gy, i, j, 0), 2));
        float xyres = get_pixel(gx, i, j, 0) * get_pixel(gy, i, j, 0);
        set_pixel(S, i, j, 2, xyres);
        if (xyres > max) {
          printf("%f\n", xyres);
          max = xyres;
        }
      }
    }
    smooth_image(im, sigma);

    free_image(gx);
    free_image(gy);
    free_image(gxFilter);
    free_image(gyFilter);
    
    return S;
}
// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    double alpha = 0.06f;
    
    image R = make_image(S.w, S.h, 1);
    // TODO: fill in R, "cornerness" for each pixel using the structure matrix.
    // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.
    float max = -99999999;
    for (int i = 0; i < R.w; i++) {
      for (int j = 0; j < R.h; j++) {
        matrix Sm = make_matrix(2, 2);
        Sm.data[0][0] = get_pixel(S, i, j, 0);
        Sm.data[1][0] = get_pixel(S, i, j, 2);
        Sm.data[0][1] = get_pixel(S, i, j, 2);
        Sm.data[1][1] = get_pixel(S, i, j, 1);
        float sum = (float)(determinant(Sm) - (alpha * powf(trace(Sm), 2)));
        set_pixel(R, i, j, 0, sum);
        if (sum > max) {
          //print_matrix(Sm);
          //printf("%f\n", sum);
          max = sum;
        }
        free_matrix(Sm);
      }
    }
    return R;
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    image r = copy_image(im);
    // TODO: perform NMS on the response map.
    // for every pixel in the image:
    //     for neighbors within w:
    //         if neighbor response greater than pixel response:
    //             set response to be very low (I use -999999 [why not 0??])
    float lowVal = -999999;
    for (int i = 0; i < r.w; i++) {
      for (int j = 0; j < r.h; j++) {
        float pixel = get_pixel(r, i, j, 0);
        int lowerFound = 0;
        for (int iw = -w; iw <= w; iw++) {
          for (int jw = -w; jw <= w; jw++) {
            if (iw != 0 && jw != 0 && get_pixel(r, i + iw, j + jw, 0) > pixel) {
              set_pixel(r, i, j, 0, lowVal);
              lowerFound = 1;
            }
          } 
          if (lowerFound) {
            break;
          }
        }
      }
    }

    return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{
    printaf("starting harris corner detector");
    // Calculate structure matrix
    image S = structure_matrix(im, sigma);
    printaf("created structure matrix");
    // Estimate cornerness
    image R = cornerness_response(S);
    printaf("completed Cornerness Response");
    // Run NMS on the responses
    image Rnms = nms_image(R, nms);
    printaf("did Non-max suprression");

    //TODO: count number of responses over threshold
    printf("%f\n", thresh);
    float max = -999999999999;
    int count = 0;
    for (int i = 0; i < Rnms.w; i++) {
      for (int j = 0; j < Rnms.h; j++) {
        if (get_pixel(R, i, j, 0) >= thresh) {
          count++;
          printaf("found corner");
        }
        if (get_pixel(R, i, j, 0) > max) {
          max = get_pixel(R, i, j, 0);
        }
      }
    }
    printf("%f\n", max);
    printaf("counted responses");
    
    *n = count; // <- set *n equal to number of corners in image.
    descriptor *d = calloc(count, sizeof(descriptor));
    //TODO: fill in array *d with descriptors of corners, use describe_index.
    int descriptorIndex = 0;
    for (int i = 0; i < Rnms.w; i++) {
      for (int j = 0; j < Rnms.h; j++) {
        if (get_pixel(R, i , j, 0) >= thresh && descriptorIndex < count) {
          d[descriptorIndex] = describe_index(im, i * Rnms.w + j);
          descriptorIndex++;
        }
      }
    }
    printaf("saved descriptors");
    save_image(S, "S");
    save_image(R, "R");
    save_image(Rnms, "Rnms");

    free_image(S);
    //printaf("freed S");
    free_image(R);
    //printaf("freed R");
    free_image(Rnms);
    //printaf("freed Rnms");
    
    //printaf("freed images");
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    printaf("starting");
    //fprintf(stdout, "starting fprintf");
    //fflush(stdout);
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    printf("%d\n", n);
    save_image(im, "corners1");
    mark_corners(im, d, n);

    
}
