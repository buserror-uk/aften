/**
 * Aften: A/52 audio encoder
 * Copyright (c) 2006 Justin Ruggles
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
 * @file pcmfile.c
 * raw PCM decoder
 */

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pcmfile.h"

int
pcmfile_init(PcmFile *pf, FILE *fp, enum PcmSampleFormat read_format,
             int file_format)
{
    int i;

    if(pf == NULL || fp == NULL) {
        fprintf(stderr, "null input to pcmfile_init()\n");
        return -1;
    }

    pf->read_to_eof = 0;
    pf->file_format = file_format;
    pf->read_format = read_format;

    // attempt to get file size
    pf->file_size = 0;
    pf->seekable = 0;
#ifdef _WIN32
    // in Windows, don't try to detect seeking support for stdin
    if(fp != stdin) {
        pf->seekable = !fseek(fp, 0, SEEK_END);
    }
#else
    pf->seekable = !fseek(fp, 0, SEEK_END);
#endif
    if(pf->seekable) {
        // TODO: portable 64-bit ftell
        long fs = ftell(fp);
        // ftell should return an error if value cannot fit in return type
        if(fs < 0) {
            fprintf(stderr, "Warning, unsupported file size.\n");
            pf->file_size = 0;
        } else {
            pf->file_size = (uint64_t)fs;
        }
        fseek(fp, 0, SEEK_SET);
    }
    pf->filepos = 0;
    if(byteio_init(&pf->io, fp)) {
        fprintf(stderr, "error initializing byte buffer\n");
        return -1;
    }

    // detect file format if not specified by the user
    if(pf->file_format == PCM_FORMAT_UNKNOWN) {
        uint8_t probe_data[12];
        int probe_scores[2], probe_max;
        byteio_peek(probe_data, 12, &pf->io);
        probe_max = 0;
        for(i=0; i<2; i++) {
            switch(i) {
                case PCM_FORMAT_RAW:
                    probe_scores[i] = pcmfile_probe_raw(probe_data, 12);
                    break;
                case PCM_FORMAT_WAVE:
                    probe_scores[i] = pcmfile_probe_wave(probe_data, 12);
                    break;
            }
            if(probe_scores[i] > probe_scores[probe_max])
                probe_max = i;
        }
        pf->file_format = probe_max;
    }

    // initialize format
    switch(pf->file_format) {
        case PCM_FORMAT_RAW:
            if(pcmfile_init_raw(pf))
                return -1;
            break;
        case PCM_FORMAT_WAVE:
            if(pcmfile_init_wave(pf))
                return -1;
            break;
        default:
            fprintf(stderr, "unknown file format\n");
            return -1;
    }

    return 0;
}

void
pcmfile_close(PcmFile *pf)
{
    byteio_close(&pf->io);
}

void
pcmfile_print(PcmFile *pf, FILE *st)
{
    char *type, *chan, *fmt, *order;
    if(st == NULL || pf == NULL) return;
    type = "?";
    chan = "?-channel";
    fmt = "unknown";
    order = "?-endian";
    if(pf->sample_type == PCM_SAMPLE_TYPE_INT) {
        if(pf->source_format == PCM_SAMPLE_FMT_U8) type = "Unsigned";
        else type = "Signed";
    } else if(pf->sample_type == PCM_SAMPLE_TYPE_FLOAT) {
        type = "Floating-point";
    } else {
        type = "[unsupported type]";
    }
    if(pf->ch_mask & 0x08) {
        switch(pf->channels-1) {
            case 1: chan = "1.1-channel"; break;
            case 2: chan = "2.1-channel"; break;
            case 3: chan = "3.1-channel"; break;
            case 4: chan = "4.1-channel"; break;
            case 5: chan = "5.1-channel"; break;
            default: chan = "multi-channel with LFE"; break;
        }
    } else {
        switch(pf->channels) {
            case 1: chan = "mono"; break;
            case 2: chan = "stereo"; break;
            case 3: chan = "3-channel"; break;
            case 4: chan = "4-channel"; break;
            case 5: chan = "5-channel"; break;
            case 6: chan = "6-channel"; break;
            default: chan = "multi-channel"; break;
        }
    }
    switch(pf->file_format) {
        case PCM_FORMAT_RAW:  fmt = "RAW"; break;
        case PCM_FORMAT_WAVE: fmt = "WAVE"; break;
    }
    switch(pf->order) {
        case PCM_BYTE_ORDER_LE: order = "little-endian"; break;
        case PCM_BYTE_ORDER_BE: order = "big-endian"; break;
    }
    fprintf(st, "%s %s %d-bit %s %d Hz %s\n", fmt, type, pf->bit_width, order,
            pf->sample_rate, chan);
}

int
pcm_get_default_ch_mask(int channels)
{
    static const int nch_to_mask[6] = {
        0x04,   // mono         (1/0)
        0x03,   // stereo       (2/0)
        0x103,  // 3.0 surround (2/1)
        0x107,  // 3/1 surround (3/1)
        0x37,   // 5.0 surround (3/2)
        0x3F    // 5.1 surround (3/2+LFE)
    };
    if(channels < 1 || channels > 6)
        return 0;
    return nch_to_mask[channels-1];
}