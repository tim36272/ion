#include "..\inc\ffwrapper\write.h"
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
#include <libavutil/channel_layout.h>
#include <libavutil/avassert.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
} //extern "C" {
#endif

#include "ffwrapper\write.h"

#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define STREAM_FRAME_RATE 5 /* 25 images/s */
#define SCALE_FLAGS SWS_BICUBIC


namespace ion
{
	struct FFWriteImpl
	{
		AVFormatContext *fmt_ctx_;
		AVOutputFormat *fmt;
		AVStream *video_stream_;
		AVCodecContext *video_codec_context_;
		AVFrame *frame_;
		AVFrame *temp_frame_;
		int64_t next_pts;
		SwsContext *sws_context_;
		int32_t width_;
		int32_t height_;





		int video_stream_index_;
		int32_t num_channels_;

		AVPixelFormat pix_fmt_;
		int video_dst_bufsize_;
		int      video_dst_linesize_[4];

		uint8_t *video_dst_data_[4];
		AVPacket pkt;
		uint8_t* rgb_data_;
	};
	/* Add an output stream. */
	static void add_stream(FFWriteImpl *impl,
						   AVCodec **codec)
	{
		AVCodecContext *c;
		AVRational time_base;

		/* find the encoder */
		*codec = avcodec_find_encoder(impl->fmt->video_codec);
		if (!(*codec))
		{
			LOGFATAL("Could not find encoder for '%s'",
					avcodec_get_name(impl->fmt->video_codec));
		}

		impl->video_stream_ = avformat_new_stream(impl->fmt_ctx_, NULL);
		if (!impl->video_stream_)
		{
			LOGFATAL("Could not allocate stream");
		}
		impl->video_stream_->id = impl->fmt_ctx_->nb_streams - 1;
		c = avcodec_alloc_context3(*codec);
		if (!c)
		{
			LOGFATAL("Could not alloc an encoding context");
		}
		impl->video_codec_context_ = c;

		switch ((*codec)->type)
		{
			//case AVMEDIA_TYPE_AUDIO:
			//	c->sample_fmt = (*codec)->sample_fmts ?
			//		(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
			//	c->bit_rate = 64000;
			//	c->sample_rate = 44100;
			//	if ((*codec)->supported_samplerates)
			//	{
			//		c->sample_rate = (*codec)->supported_samplerates[0];
			//		for (i = 0; (*codec)->supported_samplerates[i]; i++)
			//		{
			//			if ((*codec)->supported_samplerates[i] == 44100)
			//				c->sample_rate = 44100;
			//		}
			//	}
			//	c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
			//	c->channel_layout = AV_CH_LAYOUT_STEREO;
			//	if ((*codec)->channel_layouts)
			//	{
			//		c->channel_layout = (*codec)->channel_layouts[0];
			//		for (i = 0; (*codec)->channel_layouts[i]; i++)
			//		{
			//			if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
			//				c->channel_layout = AV_CH_LAYOUT_STEREO;
			//		}
			//	}
			//	c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
			//	time_base.num = 1;
			//	time_base.den = c->sample_rate;
			//	impl->video_stream_->time_base = time_base;
			//	break;

			case AVMEDIA_TYPE_VIDEO:
				c->codec_id = impl->fmt_ctx_->oformat->video_codec;

				c->bit_rate = 400000;
				/* Resolution must be a multiple of two. */
				c->width = impl->width_;
				c->height = impl->height_;
				/* timebase: This is the fundamental unit of time (in seconds) in terms
				* of which frame timestamps are represented. For fixed-fps content,
				* timebase should be 1/framerate and timestamp increments should be
				* identical to 1. */
				time_base.num = 1;
				time_base.den = STREAM_FRAME_RATE;
				impl->video_stream_->time_base = time_base;
				c->time_base = impl->video_stream_->time_base;

				c->gop_size = 12; /* emit one intra frame every twelve frames at most */
				c->pix_fmt = STREAM_PIX_FMT;
				if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
				{
					/* just for testing, we also add B-frames */
					c->max_b_frames = 2;
				}
				if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
				{
					/* Needed to avoid using macroblocks in which some coeffs overflow.
					* This does not happen with normal video, it just happens here as
					* the motion of the chroma plane does not match the luma plane. */
					c->mb_decision = 2;
				}
				break;

			default:
				LOGFATAL("Invalid stream type");
				break;
		}

		/* Some formats want stream headers to be separate. */
		if (impl->fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
	{
		AVFrame *picture;
		int ret;

		picture = av_frame_alloc();
		if (!picture)
			return NULL;

		picture->format = pix_fmt;
		picture->width = width;
		picture->height = height;

		/* allocate the buffers for the frame data */
		ret = av_frame_get_buffer(picture, 32);
		if (ret < 0)
		{
			LOGFATAL("Could not allocate frame data.");
		}

		return picture;
	}

	static void open_video(FFWriteImpl* impl, AVCodec *codec, AVDictionary *opt_arg)
	{
		int ret;
		AVCodecContext *c = impl->video_codec_context_;
		AVDictionary *opt = NULL;

		av_dict_copy(&opt, opt_arg, 0);

		/* open the codec */
		ret = avcodec_open2(c, codec, &opt);
		av_dict_free(&opt);
		if (ret < 0)
		{
			char error_str[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, ret);
			LOGFATAL("Could not open video codec: %s", error_str);
		}

		/* allocate and init a re-usable frame */
		impl->frame_ = alloc_picture(c->pix_fmt, c->width, c->height);
		if (!impl->frame_)
		{
			LOGFATAL("Could not allocate video frame");
		}

		/* If the output format is not YUV420P, then a temporary YUV420P
		* picture is needed too. It is then converted to the required
		* output format. */
		impl->temp_frame_ = NULL;
		if (c->pix_fmt != AV_PIX_FMT_BGR24)
		{
			impl->temp_frame_ = alloc_picture(AV_PIX_FMT_BGR24, c->width, c->height);
			if (!impl->temp_frame_)
			{
				LOGFATAL("Could not allocate temporary picture");
			}
		}

		/* copy the stream parameters to the muxer */
		ret = avcodec_parameters_from_context(impl->video_stream_->codecpar, c);
		if (ret < 0)
		{
			LOGFATAL("Could not copy the stream parameters");
		}
	}

	static AVFrame *get_video_frame(FFWriteImpl* impl, const ion::Image& img)
	{

		/* check if we want to generate more frames */
		AVRational end_frame;
		end_frame.den = 1;
		end_frame.num = 1;

		/* when we pass a frame to the encoder, it may keep a reference to it
		* internally; make sure we do not overwrite it here */
		LOGASSERT(av_frame_make_writable(impl->frame_) >= 0);

		if (impl->video_codec_context_->pix_fmt != AV_PIX_FMT_BGR24)
		{
			/* as we only generate a BGR picture, we must convert it
			* to the codec pixel format if needed */
			LOGASSERT(impl->video_codec_context_->height == img.rows() && impl->video_codec_context_->width == img.cols());
			img.Memcpy(impl->temp_frame_->data[0]);
			sws_scale(impl->sws_context_,
				(const uint8_t * const *)impl->temp_frame_->data, impl->temp_frame_->linesize,
						0, impl->video_codec_context_->height, impl->frame_->data, impl->frame_->linesize);
		} else
		{
			img.Memcpy(impl->frame_->data[0]);
		}

		impl->frame_->pts = impl->next_pts++;

		return impl->frame_;
	}
	//static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
	//{
	//	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

	//	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
	//		   av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
	//		   av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
	//		   av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
	//		   pkt->stream_index);
	//}
	static int write_frame(FFWriteImpl* impl, /*AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st,*/ AVPacket *pkt)
	{
		/* rescale output packet timestamp values from codec to stream timebase */
		av_packet_rescale_ts(pkt, impl->video_codec_context_->time_base, impl->video_stream_->time_base);
		pkt->stream_index = impl->video_stream_->index;

		/* Write the compressed frame to the media file. */
		//log_packet(impl->fmt_ctx_, pkt);
		return av_interleaved_write_frame(impl->fmt_ctx_, pkt);
	}


	FFWriter::FFWriter(std::string uri, uint32_t rows, uint32_t cols) : video_open_(false)
	{
		AVCodec* video_codec = NULL;
		int have_video = 0, have_audio = 0;
		int encode_video = 0, encode_audio = 0;
		AVDictionary *opt = NULL;
		int ret;
		impl = new FFWriteImpl;
		memset(impl, 0, sizeof(FFWriteImpl));
		impl->height_ = rows;
		impl->width_ = cols;

		av_register_all();

		/* allocate the output media context */
		avformat_alloc_output_context2(&impl->fmt_ctx_, NULL, NULL, uri.c_str());
		if (!impl->fmt_ctx_)
		{
			printf("Could not deduce output format from file extension: using MPEG.\n");
			avformat_alloc_output_context2(&impl->fmt_ctx_, NULL, "mpeg", uri.c_str());
		}
		if (!impl->fmt_ctx_)
		{
			LOGFATAL("Failed to initialize output format contex")
		}
			

		impl->fmt = impl->fmt_ctx_->oformat;

		/* Add the audio and video streams using the default format codecs
		* and initialize the codecs. */
		if (impl->fmt->video_codec != AV_CODEC_ID_NONE)
		{
			add_stream(impl, &video_codec);
			have_video = 1;
			encode_video = 1;
		}
		//if (impl->fmt->audio_codec != AV_CODEC_ID_NONE)
		//{
		//	add_stream(&audio_st, impl->fmt_ctx_, &audio_codec, impl->fmt->audio_codec);
		//	have_audio = 1;
		//	encode_audio = 1;
		//}

		/* Now that all the parameters are set, we can open the audio and
		* video codecs and allocate the necessary encode buffers. */
		if (have_video)
		{
			open_video(impl, video_codec, opt);
		}

		//if (have_audio)
		//{
		//	open_audio(impl->fmt_ctx_, audio_codec, &audio_st, opt);
		//}

		av_dump_format(impl->fmt_ctx_, 0, uri.c_str(), 1);

		/* open the output file, if needed */
		if (!(impl->fmt->flags & AVFMT_NOFILE))
		{
			ret = avio_open(&impl->fmt_ctx_->pb, uri.c_str(), AVIO_FLAG_WRITE);
			if (ret < 0)
			{
				char error_str[AV_ERROR_MAX_STRING_SIZE];
				av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, ret);
				fprintf(stderr, "Could not open '%s': %s\n", uri.c_str(),
						error_str);
				LOGFATAL("See prior");
			}
		}

		/* Write the stream header, if any. */
		ret = avformat_write_header(impl->fmt_ctx_, &opt);
		if (ret < 0)
		{
			char error_str[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, ret);
			fprintf(stderr, "Error occurred when opening output file: %s\n",
					error_str);
			LOGFATAL("See prior");
		}

		if (impl->video_codec_context_->pix_fmt != AV_PIX_FMT_BGR24)
		{
			impl->sws_context_ = sws_getContext(impl->video_codec_context_->width, impl->video_codec_context_->height,
												AV_PIX_FMT_BGR24,
												impl->video_codec_context_->width, impl->video_codec_context_->height,
												impl->video_codec_context_->pix_fmt,
												SCALE_FLAGS, NULL, NULL, NULL);
			if (!impl->sws_context_)
			{
				LOGFATAL("Could not initialize the conversion context\n");
			}
		}

		video_open_ = true;
	}
	FFWriter::~FFWriter()
	{
		Close();
		delete impl;
	}
	void FFWriter::WriteFrame(const ion::Image & img)
	{
		int ret;
		AVFrame *frame;
		int got_packet = 0;
		AVPacket pkt = { 0 };

		frame = get_video_frame(impl, img);
		LOGASSERT(frame);

		av_init_packet(&pkt);

		/* encode the image */
		ret = avcodec_encode_video2(impl->video_codec_context_, &pkt, frame, &got_packet);
		if (ret < 0)
		{
			char error_str[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, ret);
			LOGFATAL("Error encoding video frame: %s\n", error_str);
		}

		if (got_packet)
		{
			ret = write_frame(impl, &pkt);
		} else
		{
			ret = 0;
		}

		if (ret < 0)
		{
			char error_str[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(error_str, AV_ERROR_MAX_STRING_SIZE, ret);
			LOGFATAL("Error while writing video frame: %s\n", error_str);

		}
		av_free_packet(&pkt);
	}
	void FFWriter::Close()
	{
		if (video_open_)
		{
			/* Write the trailer, if any. The trailer must be written before you
			* close the CodecContexts open when you wrote the header; otherwise
			* av_write_trailer() may try to use memory that was freed on
			* av_codec_close(). */
			av_write_trailer(impl->fmt_ctx_);
			video_open_ = false;
		}
	}
};