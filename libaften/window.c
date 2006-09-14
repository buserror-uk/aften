/**
 * Aften: A/52 audio encoder
 * Copyright (c) 2006  Justin Ruggles <jruggle@earthlink.net>
 *
 * Based on "The simplest AC3 encoder" from FFmpeg
 * Copyright (c) 2000 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file window.c
 * A/52 Kaiser-Bessel Derived Window
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "a52.h"
#include "window.h"

#ifdef CONFIG_DOUBLE
#define ONE 1.0
#else
#define ONE 1.0f
#endif

static FLOAT a52_window[512];

/**
 * Generate a Kaiser-Bessel Derived Window.
 * @param alpha         Determines window shape
 * @param out_window    Array to fill with window values
 * @param n             Full window size
 * @param iter          Number of iterations to use in BesselI0
 */
static void
kbd_window_init(FLOAT alpha, FLOAT *window, int n, int iter)
{
    int i, j, n2;
    FLOAT a, x, bessel, sum;

    n2 = n >> 1;
    a = alpha * AFT_PI / n2;
    a = a*a;
    sum = 0.0;
    for(i=0; i<n2; i++) {
        x = i * (n2 - i) * a;
        bessel = ONE;
        for(j=iter; j>0; j--) {
            bessel = (bessel * x / (j*j)) + ONE;
        }
        sum += bessel;
        window[i] = sum;
    }
    sum += ONE;
    for(i=0; i<n2; i++) {
        window[i] = AFT_SQRT(window[i] / sum);
        window[n-1-i] = window[i];
    }
}

void
apply_a52_window(FLOAT *samples)
{
    int i;
    for(i=0; i<512; i++) {
        samples[i] *= a52_window[i];
    }
}

void
a52_window_init()
{
    kbd_window_init(5.0, a52_window, 512, 50);
}