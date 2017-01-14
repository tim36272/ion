/*
This file is part of Ionlib.  Copyright (C) 2016  Tim Sweet

Ionlib is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ionlib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ionlib.If not, see <http://www.gnu.org/licenses/>.
*/

//Library includes
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
} //extern "C" {
#endif
//Project includes
#include "ffwrapper/read.h"
#include "ionlib/log.h"


#define INBUF_SIZE 4096

	namespace ion
	{
		struct FFReadImpl
		{
			AVFormatContext *fmt_ctx_;
			AVCodecContext *video_dec_ctx_;
			AVStream *video_stream_;
			int video_stream_index_;

			int32_t width_;
			int32_t height_;
			int32_t num_channels_;
			AVPixelFormat pix_fmt_;
			int video_dst_bufsize_;
			int      video_dst_linesize_[4];

			uint8_t *video_dst_data_[4];
			AVFrame *frame_;
			AVPacket pkt;
			SwsContext *sws_context_;
			uint8_t* rgb_data_;
		};

		static int open_codec_context(int *stream_idx,
									  AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
		{
			int ret, stream_index;
			AVStream *st;
			AVCodec *dec = NULL;
			AVDictionary *opts = NULL;

			ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
			if (ret < 0)
			{
				LOGFATAL("Could not find %s stream in input file", av_get_media_type_string(type));
				return ret;
			} else
			{
				stream_index = ret;
				st = fmt_ctx->streams[stream_index];

				/* find decoder for the stream */
				dec = avcodec_find_decoder(st->codecpar->codec_id);
				if (!dec)
				{
					fprintf(stderr, "Failed to find %s codec\n",
							av_get_media_type_string(type));
					return AVERROR(EINVAL);
				}

				/* Allocate a codec context for the decoder */
				*dec_ctx = avcodec_alloc_context3(dec);
				if (!*dec_ctx)
				{
					fprintf(stderr, "Failed to allocate the %s codec context\n",
							av_get_media_type_string(type));
					return AVERROR(ENOMEM);
				}

				/* Copy codec parameters from input stream to output codec context */
				if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
				{
					fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
							av_get_media_type_string(type));
					return ret;
				}

				/* Init the decoders, with or without reference counting */
				av_dict_set(&opts, "refcounted_frames", "0", 0);
				if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
				{
					fprintf(stderr, "Failed to open %s codec\n",
							av_get_media_type_string(type));
					return ret;
				}
				*stream_idx = stream_index;
			}

			return 0;
		}

		static int decode_packet(int *got_frame, FFReadImpl* impl)
		{
			int ret = 0;
			int decoded = impl->pkt.size;

			*got_frame = 0;

			if (impl->pkt.stream_index == impl->video_stream_index_)
			{
				/* decode video frame */
				ret = avcodec_decode_video2(impl->video_dec_ctx_, impl->frame_, got_frame, &impl->pkt);
				if (ret < 0)
				{
					char error_string[AV_ERROR_MAX_STRING_SIZE];
					av_make_error_string(error_string, AV_ERROR_MAX_STRING_SIZE, ret);
					fprintf(stderr, "Error decoding video frame (%s)\n", error_string);
					return ret;
				}

				if (*got_frame)
				{

					if (impl->frame_->width != impl->width_ || impl->frame_->height != impl->height_ ||
						impl->frame_->format != impl->pix_fmt_)
					{
						/* To handle this change, one could call av_image_alloc again and
						* decode the following frames into another rawvideo file. */
						fprintf(stderr, "Error: Width, height and pixel format have to be "
								"constant in a rawvideo file, but the width, height or "
								"pixel format of the input video changed:\n"
								"old: width = %d, height = %d, format = %s\n"
								"new: width = %d, height = %d, format = %s\n",
								impl->width_, impl->height_, av_get_pix_fmt_name(impl->pix_fmt_),
								impl->frame_->width, impl->frame_->height,
								av_get_pix_fmt_name((AVPixelFormat)impl->frame_->format));
						return -1;
					}

					/* copy decoded frame to destination buffer:
					* this is required since rawvideo expects non aligned data */
					av_image_copy(impl->video_dst_data_, impl->video_dst_linesize_,
						(const uint8_t **)(impl->frame_->data), impl->frame_->linesize,
								  impl->pix_fmt_, impl->width_, impl->height_);
				}
			} else
			{
				LOGDEBUG("Got non-video packet");
			}

			return decoded;
		}

		FFReader::FFReader(std::string uri)
		{
			int result;
			impl = new FFReadImpl;
			memset(impl, 0, sizeof(FFReadImpl));

			/* register all formats and codecs */
			av_register_all();

			avformat_network_init();

			/* open input file, and allocate format context */
			if (avformat_open_input(&impl->fmt_ctx_, uri.c_str(), NULL, NULL) < 0)
			{
				fprintf(stderr, "Could not open source file %s\n", uri.c_str());
				exit(1);
			}

			/* retrieve stream information */
			if (avformat_find_stream_info(impl->fmt_ctx_, NULL) < 0)
			{
				fprintf(stderr, "Could not find stream information\n");
				exit(1);
			}

			if (open_codec_context(&impl->video_stream_index_, &impl->video_dec_ctx_, impl->fmt_ctx_, AVMEDIA_TYPE_VIDEO) >= 0)
			{
				impl->video_stream_ = impl->fmt_ctx_->streams[impl->video_stream_index_];

				/* allocate image where the decoded image will be put */
				impl->width_ = impl->video_dec_ctx_->width;
				impl->height_ = impl->video_dec_ctx_->height;
				impl->pix_fmt_ = impl->video_dec_ctx_->pix_fmt;
				result = av_image_alloc(impl->video_dst_data_, impl->video_dst_linesize_,
										impl->width_, impl->height_, impl->pix_fmt_, 1);
				if (result < 0)
				{
					LOGFATAL("Could not allocate raw video buffer");
				}
				impl->num_channels_ = result;
				for (uint32_t line_index = 0; line_index < 4; ++line_index)
				{
					if (impl->video_dst_linesize_[line_index] != 0)
					{
						impl->num_channels_ /= impl->video_dst_linesize_[line_index];
					} else
					{
						break;
					}
				}

				impl->video_dst_bufsize_ = result;
			}

			/* dump input information to stderr */
			av_dump_format(impl->fmt_ctx_, 0, uri.c_str(), 0);

			if (!impl->video_stream_)
			{
				LOGFATAL("Could not find video stream in the input, aborting");
			}

			impl->frame_ = av_frame_alloc();
			if (!impl->frame_)
			{
				LOGFATAL("Could not allocate frame");
			}

			/* initialize packet, set data to NULL, let the demuxer fill it */
			av_init_packet(&impl->pkt);
			impl->pkt.data = NULL;
			impl->pkt.size = 0;

			//initialize sws context
			//BGR conversion from https://ffmpeg.zeranoe.com/forum/viewtopic.php?t=805
			impl->sws_context_ = sws_getContext(impl->width_, impl->height_, impl->pix_fmt_, impl->width_, impl->height_, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, 0, 0, 0);

			//allocate RGB data
			impl->rgb_data_ = new uint8_t[impl->width_*impl->height_ * 3];
		}
		FFReader::~FFReader()
		{
			delete impl;
		}
		ion::Image FFReader::ReadFrame()
		{
			int result;
			int got_frame = 0;
			/* read frames from the file */
			while (!got_frame && av_read_frame(impl->fmt_ctx_, &impl->pkt) >= 0)
			{
				AVPacket orig_pkt = impl->pkt;
				do
				{
					result = decode_packet(&got_frame, impl);
					if (result < 0)
						break;
					impl->pkt.data += result;
					impl->pkt.size -= result;
				} while (impl->pkt.size > 0);
				av_packet_unref(&orig_pkt);
			}
			int stride = impl->width_ * 3;
			sws_scale(impl->sws_context_, impl->frame_->data, impl->frame_->linesize, 0, impl->height_, &impl->rgb_data_, &stride);
			ion::Image temp(impl->height_,impl->width_,3, impl->rgb_data_);
			return temp;
		}
	};
