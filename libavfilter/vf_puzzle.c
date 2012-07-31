/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 *  puzzle video filter
 */

#include "avfilter.h"
#include "internal.h"
#include "video.h"
#include "libavutil/lfg.h"

typedef struct {
    int* xoffsets;
    int* yoffsets;
    int xs;
    int ys;
} PuzzleContext;


static void shuffle(int* arr, size_t len)
{
   int replace, index;

   if (len <= 1) return;

   av_lfg_get(&index);
   index = 1 + index % len;
   replace = arr[0];
   arr[0] = arr[index];
   arr[index] = replace;

   shuffle(arr+1, len-1);
}

static void memflip(char *a, char *b, size_t n)
{
    static char tmp;
    for (; n; a++, b++, n--) {
        tmp = *a;
        *a = *b;
        *b = *a;
    }
}

static av_cold int init(AVFilterContext *ctx, const char *args)
{
    PuzzleContext *puzzle = ctx->priv;


    if (args)
        sscanf(args, "%d:%d", &puzzle->xs, &puzzle->ys);
    else
        puzzle->xs = puzzle->ys = 5;

    if (!((puzzle->xoffsets = av_calloc(sizeof(int), puzzle->xs)) &&
          (puzzle->yoffsets = av_calloc(sizeof(int), puzzle->ys))))
        return AVERROR(ENOMEM);

    return 0;
}

static void uninit(AVFilterContext *ctx)
{
    PuzzleContext *puzzle = ctx->priv;

    av_free(puzzle->xoffsets);
    av_free(puzzle->yoffsets);
}

static int config_props(AVFilterLink* link)
{
    PuzzleContext *puzzle = link->dst->priv;
    size_t i;

    for (i = 0; i != puzzle->xs; i++)
        puzzle->xoffsets[i] = i;
    for (i = 0; i != puzzle->ys; i++)
        puzzle->yoffsets[i] = i;

    /* shuffle */
    shuffle(puzzle->yoffsets, puzzle->ys);
    shuffle(puzzle->xoffsets, puzzle->xs);

    return 0;

}


static int draw_slice(AVFilterLink *link, int y, int h, int slice_dir)
{
   PuzzleContext *puzzle = link->dst->priv;
   AVFilterBufferRef *pic = link->cur_buf;
   const int block = link->w / puzzle->xs;
   unsigned char *p;
   size_t item, i;

   p = pic->data[0] + y * pic->linesize[0];
   for (i = 0; i != h; i++) {
       for (item = 0; item != puzzle->xs; item++) {
           memflip(p + block * item, p + puzzle->xoffsets[block] * item,
                   block-1);
           // put white pixel in the middle
           p[block*item] = 0xff;
       }
       p += pic->linesize[0];
   }
   return 0;
}

static int end_frame(AVFilterLink *link)
{
    return  ff_null_end_frame(link);
}



AVFilter avfilter_vf_puzzle = {
    .name        = "puzzle",
    .description = NULL_IF_CONFIG_SMALL("Create a fuckin' puzzle from the given video"),
    .init        = init,
    .uninit      = uninit,
    .priv_size   = sizeof(PuzzleContext),

    .inputs      = (const AVFilterPad[]) {{ .name             = "default",
                                            .type             = AVMEDIA_TYPE_VIDEO,
                                            .get_video_buffer = ff_null_get_video_buffer,
                                            .start_frame      = ff_null_start_frame_keep_ref,
                                            .config_props     = config_props,
                                            .draw_slice       = draw_slice,
                                            .end_frame        = end_frame, },
                                          { .name = NULL}},

    .outputs     = (const AVFilterPad[]) {{ .name             = "default",
                                            .type             = AVMEDIA_TYPE_VIDEO, },
                                          { .name = NULL}},
};
