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
 * negative video filter
 */

#include "avfilter.h"
#include "internal.h"
#include "video.h"

static int draw_slice(AVFilterLink *inlink, int y, int h, int slice_dir)
{
    AVFilterBufferRef *pic = inlink->cur_buf;
    int x, i, plane;
    uint8_t *p;

    for (plane = 0; plane != 4; plane++) {
        p = pic->data[0] + y * pic->linesize[plane];
        for (i = 0; i != h; i++) {
            for (x = 0; x != inlink->w; x++)
                p[x] = ~p[x];
            p += pic->linesize[plane];
        }
    }
    return 0;
}



AVFilter avfilter_vf_negative = {
    .name      = "negative",
    .description = NULL_IF_CONFIG_SMALL("Invert colors of the source given"),

    .priv_size = 0,

    .inputs    = (const AVFilterPad[]) {{ .name             = "default",
                                          .type             = AVMEDIA_TYPE_VIDEO,
                                          .draw_slice       = draw_slice,
                                          .get_video_buffer = ff_null_get_video_buffer,
                                          .start_frame      = ff_null_start_frame_keep_ref,
                                          .end_frame        = ff_null_end_frame, },
                                        { .name = NULL}},

    .outputs   = (const AVFilterPad[]) {{ .name             = "default",
                                          .type             = AVMEDIA_TYPE_VIDEO, },
                                        { .name = NULL}},
};
