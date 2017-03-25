#include "ionlib/log.h"
#include "ionlib/net.h"
#include "ionlib/serial.h"
#include "ionlib/time.h"
#include "ionlib/math.h"
#include <opencv2/highgui.hpp>
#include <vector>
#include <float.h>

#define NUM_PIXELS (150)
#define PI (3.141592653)

namespace ion
{
	typedef uint16_t color_t;
	struct CRGB
	{
		CRGB()
		{
			r = 0;
			g = 0;
			b = 0;
		}
		CRGB(uint32_t hex_color)
		{
			r = hex_color >> 16;
			g = (hex_color >> 8) & 0xFF;
			b = (hex_color) & 0xFF;
		}
		CRGB& operator+= (const CRGB& rhs)
		{
			if (this->r + rhs.r <= UINT8_MAX)
			{
				this->r += rhs.r;
			} else
			{
				this->r = UINT8_MAX;
			}
			if (this->g + rhs.g <= UINT8_MAX)
			{
				this->g += rhs.g;
			} else
			{
				this->g = UINT8_MAX;
			}
			if (this->b + rhs.b <= UINT8_MAX)
			{
				this->b += rhs.b;
			} else
			{
				this->b = UINT8_MAX;
			}
			return *this;
		}
		CRGB operator+ (const CRGB& rhs)
		{
			ion::CRGB result = *this;;
			result.r += rhs.r;
			result.g += rhs.g;
			result.b += rhs.b;
			return result;
		}
		CRGB operator* (const float scale)
		{
			ion::CRGB result = *this;
			result.r = (color_t)(result.r * scale + 0.5f); //0.5 added for rounding
			result.g = (color_t)(result.g * scale + 0.5f); //0.5 added for rounding
			result.b = (color_t)(result.b * scale + 0.5f); //0.5 added for rounding
			return result;
		}
		color_t r;
		color_t g;
		color_t b;
		
	};

	class ColorMap
	{
	public:
		ColorMap(int8_t max_stops)
		{
			stop_pos_ = new float[max_stops];
			color_ = new ion::CRGB[max_stops];
			num_stops_ = 0;
			max_stops_ = max_stops;
		}
		ColorMap(const ColorMap& rhs)
		{
			this->max_stops_ = rhs.max_stops_;
			this->stop_pos_ = new float[max_stops_];
			this->color_ = new ion::CRGB[max_stops_];
			this->num_stops_ = rhs.num_stops_;
			for (uint8_t stop_index = 0; stop_index < rhs.num_stops_; ++stop_index)
			{
				this->stop_pos_[stop_index] = rhs.stop_pos_[stop_index];
				this->color_[stop_index] = rhs.color_[stop_index];
			}
		}
		ColorMap& operator=(const ColorMap& rhs)
		{
			this->max_stops_ = rhs.max_stops_;
			this->stop_pos_ = new float[max_stops_];
			this->color_ = new ion::CRGB[max_stops_];
			this->num_stops_ = rhs.num_stops_;
			for (uint8_t stop_index = 0; stop_index < rhs.num_stops_; ++stop_index)
			{
				this->stop_pos_[stop_index] = rhs.stop_pos_[stop_index];
				this->color_[stop_index] = rhs.color_[stop_index];
			}
			return *this;
		}
		~ColorMap()
		{
			delete[] stop_pos_;
			delete[] color_;
		}
		//stops must be provided in order
		void addStop(float pos, const ion::CRGB& color)
		{
			if (num_stops_ > 0 && pos <= stop_pos_[num_stops_ - 1])
			{
				LOGFATAL("Invalid stop");
			}
			if (num_stops_ >= max_stops_)
			{
				LOGFATAL("Too many stops");
			}
			stop_pos_[num_stops_] = pos;
			color_[num_stops_] = color;
			++num_stops_;
		}
		ion::CRGB getAtLinear(float this_pos)
		{
			//iterate the list to find the stradling positions
			uint8_t start_index = 0;
			//be sure we didnt' request a stop before the beginning
			if (num_stops_ < 2)
			{
				LOGFATAL("Stops not set");
			}
			if (this_pos <= stop_pos_[0])
			{
				//For positions before the start of the string: the first color is returned
				return color_[0];
			}
			if (this_pos > stop_pos_[num_stops_ - 1])
			{
				//For positions before the start of the string: the last color is returned
				return color_[num_stops_ - 1];
			}
			for (start_index = 0; start_index < num_stops_ - 1; ++start_index)
			{
				if (this_pos > stop_pos_[start_index] && this_pos <= stop_pos_[start_index + 1])
				{
					//This is the starting stop
					break;
				}
			}
			//get the color at the start and end of this segment
			ion::CRGB start_color = color_[start_index];
			ion::CRGB end_color = color_[start_index + 1];
			//interpolate between these colors
			float alpha = (this_pos - stop_pos_[start_index]) / (stop_pos_[start_index + 1] - stop_pos_[start_index]);
			ion::CRGB output;
			output.r = static_cast<color_t>(start_color.r * (1.0 - alpha) + end_color.r * alpha);
			output.g = static_cast<color_t>(start_color.g * (1.0 - alpha) + end_color.g * alpha);
			output.b = static_cast<color_t>(start_color.b * (1.0 - alpha) + end_color.b * alpha);
			return output;
		}
		float* stop_pos_;
		ion::CRGB* color_;
		uint8_t num_stops_;
		uint8_t max_stops_;

	};

	struct PixelGroupBase
	{
		virtual ~PixelGroupBase()
		{
		}
		virtual float getIntensityAt(float pos) = 0;
		virtual PixelGroupBase* clone() const = 0;
	};

	struct PixelGroupLinear : PixelGroupBase
	{
		PixelGroupLinear(float intensity)
		{
			intensity_ = intensity;
		}
		float getIntensityAt(float pos)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}
			return intensity_;
		}
		virtual PixelGroupLinear* clone() const
		{
			return new PixelGroupLinear(*this);
		}
		float intensity_;
	};

	struct PixelGroupCompletelyRandom : PixelGroupBase
	{
		PixelGroupCompletelyRandom(float intensity, float probability)
		{
			intensity_ = intensity;
			probability_ = probability;
		}
		float getIntensityAt(float pos)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}

			return (ion::randlf(0.0, 1.0) < probability_) ? intensity_ : 0;
		}
		virtual PixelGroupCompletelyRandom* clone() const
		{
			return new PixelGroupCompletelyRandom(*this);
		}
		float intensity_;
		float probability_ = probability_;
	};

	struct PixelGroupConstantRandom : PixelGroupBase
	{
		PixelGroupConstantRandom(float intensity, float probability)
		{
			intensity_ = intensity;
			probability_ = probability;

		}
		float getIntensityAt(float pos)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}

			return (ion::randlf(0.0, 1.0) < probability_) ? intensity_ : 0;
		}
		virtual PixelGroupConstantRandom* clone() const
		{
			return new PixelGroupConstantRandom(*this);
		}
		float intensity_;
		float probability_ = probability_;
	};

	struct PixelGroupGaussian : PixelGroupBase
	{
		PixelGroupGaussian(float mean, float std_dev)
		{
			mean_ = mean;
			std_dev_ = std_dev;
			normalizer_ = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));
		}
		float getIntensityAt(float pos)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}
			float frac = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));
			float exponent = -0.5f * ((pos - mean_) * (pos - mean_))/ (std_dev_*std_dev_);
			float result = frac * exp(exponent);
			return result/normalizer_;
		}
		virtual PixelGroupGaussian* clone() const
		{
			return new PixelGroupGaussian(*this);
		}
		float mean_;
		float std_dev_;
		float normalizer_;
	};

	struct PixelMap
	{
		PixelMap(float start, float end, const PixelGroupBase& stencil)
		{
			stencil_ = stencil.clone();
			start_ = start;
			end_ = end;
		}
		PixelMap(const PixelMap& rhs)
		{
			this->stencil_ = rhs.stencil_->clone();
			start_ = rhs.start_;
			end_ = rhs.end_;
		}
		PixelMap& operator=(const PixelMap& rhs)
		{
			this->end_ = rhs.end_;
			this->start_ = rhs.start_;
			this->stencil_ = rhs.stencil_->clone();
			return *this;
		}
		~PixelMap()
		{
			delete this->stencil_;
		}
		PixelGroupBase* stencil_;
		float start_;
		float end_;
	};

	class KeyFrame
	{
	public:
		KeyFrame(float tov)
		{
			tov_ = tov;
		}
		void addLayer(ColorMap& colors, PixelMap& pixels)
		{
			color_map_.push_back(colors);
			pixel_map_.push_back(pixels);
		}

		static KeyFrame Black(float tov)
		{
			KeyFrame frame(tov);
			ColorMap color_map(2);
			color_map.addStop(0.0, CRGB(0x000000));
			color_map.addStop(1.0, CRGB(0x000000));
			PixelGroupLinear linear_group(0.0f);
			PixelMap pixel_map(0.0f,1.0f,linear_group);
			frame.color_map_.push_back(color_map);
			frame.pixel_map_.push_back(pixel_map);
			frame.tov_ = tov;
			return frame;
		}
		ion::CRGB getTotalValueAt(float norm_pixel_position)
		{
			//compute the desired color per the previous keyframe
			ion::CRGB total_value;

			for (uint32_t map_index = 0; map_index < color_map_.size(); ++map_index)
			{
				ion::ColorMap *this_color_map = &color_map_[map_index];
				ion::PixelMap *this_pixel_map = &pixel_map_[map_index];
				float pixel_map_extent = (this_pixel_map->end_ - this_pixel_map->start_);
				float stencil_position = (norm_pixel_position - this_pixel_map->start_ )/ pixel_map_extent;
				float intensity = this_pixel_map->stencil_->getIntensityAt(stencil_position);
				total_value += this_color_map->getAtLinear(stencil_position) * intensity;
			}
			return  total_value;
		}
		float tov_;
		std::vector<ColorMap> color_map_;
		std::vector<PixelMap> pixel_map_;
	};
	class Channel
	{
	public:
		void addFrame(const KeyFrame& frame)
		{
			frames_.push_back(frame);
		}
		std::vector<KeyFrame> frames_;
	};
	class Program
	{
	public:
		void addChannel(const Channel& channel)
		{
			channels_.push_back(channel);
		}
		std::vector<Channel> channels_;
	};

	class OutputFrame
	{
	public:
		OutputFrame()
		{
			for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
			{
				pixels_[pixel_index].b = 0;
				pixels_[pixel_index].g = 0;
				pixels_[pixel_index].r = 0;
			}
		}
		OutputFrame(const OutputFrame &rhs)
		{
			for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
			{
				pixels_[pixel_index] = rhs.pixels_[pixel_index];
			}
		}
		void asBytes(byte_t buffer[NUM_PIXELS * 3])
		{
			for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
			{
				buffer[pixel_index * 3 + 0] = (byte_t)pixels_[pixel_index].r;
				buffer[pixel_index * 3 + 1] = (byte_t)pixels_[pixel_index].g;
				buffer[pixel_index * 3 + 2] = (byte_t)pixels_[pixel_index].b;
			}
		}
		ion::CRGB pixels_[NUM_PIXELS];
	};
}
std::vector<ion::OutputFrame> buildAnimation(float duration, const ion::Program& program, uint32_t fps)
{
	uint32_t num_frames = static_cast<uint32_t>(duration * fps)+1;
	std::vector<ion::OutputFrame> output_frames(num_frames);

	//for each channel
	for (uint32_t channel_index = 0; channel_index < program.channels_.size(); ++channel_index)
	{
		const ion::Channel *this_channel = &program.channels_[channel_index];
		ion::KeyFrame previous_keyframe = ion::KeyFrame::Black(0.0);
		ion::KeyFrame next_keyframe = this_channel->frames_[0];
		uint32_t next_keyframe_index = 0;
		LOGASSERT(previous_keyframe.color_map_.size() == previous_keyframe.pixel_map_.size());
		LOGASSERT(next_keyframe.color_map_.size() == next_keyframe.pixel_map_.size());

		for (uint32_t frame_index = 0; frame_index < num_frames; ++frame_index)
		{
			float frame_tov = (float)frame_index / fps;
			//check if we need to advance keyframes
			while (frame_tov > next_keyframe.tov_)
			{
				previous_keyframe = next_keyframe;
				next_keyframe_index++;
				if (next_keyframe_index != this_channel->frames_.size())
				{
					next_keyframe = this_channel->frames_[next_keyframe_index];
				} else
				{
					//Go to black for the rest of the sequence
					next_keyframe = ion::KeyFrame::Black(duration);
				}
			}
			float t_now = (float)frame_index / fps;
			float alpha_t = (t_now - previous_keyframe.tov_) / (next_keyframe.tov_ - previous_keyframe.tov_);
			//Generate each pixel
			for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
			{
				//compute the desired color per the previous keyframe
				ion::CRGB previous_value = previous_keyframe.getTotalValueAt((float)pixel_index / NUM_PIXELS);
				ion::CRGB next_value = next_keyframe.getTotalValueAt((float)pixel_index / NUM_PIXELS);
				output_frames[frame_index].pixels_[pixel_index] += previous_value * (1.0f - alpha_t) + next_value * alpha_t;
			}
		}
	}
	return output_frames;
}

#define SUNRISE_SKY_BLUE ion::CRGB(0x1B4F72)//CRGB(0x1B4F72)
#define SUNRISE_SKY_ORANGE ion::CRGB(0xBA4A00)
#define SUNRISE_SUN ion::CRGB(0xF1C40F)
#define SUNRISE_BLACK ion::CRGB(0x000000)
#define SUNRISE_WHITE ion::CRGB(0xFFFFFF)

int main(int argc, char* argv[])
{
	ion::LogInit("composer.log");
	ion::PixelGroupGaussian sun_stencil(0.5f, 0.5f);
	ion::PixelMap sun_pix(0.5f, 1.0f, sun_stencil);
	ion::PixelMap test = sun_pix;
	ion::ColorMap sun_palett(5);
	sun_palett.addStop(0.0, SUNRISE_SKY_BLUE);
	sun_palett.addStop(0.25, SUNRISE_SKY_ORANGE);
	sun_palett.addStop(0.5, SUNRISE_SUN);
	sun_palett.addStop(0.75, SUNRISE_SKY_ORANGE);
	sun_palett.addStop(1.0, SUNRISE_SKY_BLUE);
	ion::KeyFrame frame(10.0f);
	frame.addLayer(sun_palett, sun_pix);
	ion::Channel sun_channel;
	sun_channel.addFrame(frame);
	ion::Program sunrise;
	sunrise.addChannel(sun_channel);
	const uint32_t frame_rate = 17;

	std::vector<ion::OutputFrame> output_frames = buildAnimation(20.0f, sunrise, frame_rate);
	
	//stream the frames to the target
	ion::Serial serial(6, ion::Serial::Baud::RATE_115200);
	for (std::vector<ion::OutputFrame>::iterator it = output_frames.begin(); it != output_frames.end(); ++it)
	{
		cv::Mat visual(50, NUM_PIXELS*3, CV_8UC3);
		for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
		{
			for (uint32_t row = 0; row < 50; ++row)
			{
				for (uint32_t col = 0; col < 3; ++col)
				{
					visual.at<cv::Vec3b>(row, pixel_index*3+col) = cv::Vec3b(it->pixels_[pixel_index].b, it->pixels_[pixel_index].g, it->pixels_[pixel_index].r);
				}
			}
		}
		cv::imshow("visual", visual);
		cv::waitKey(1);
		byte_t buffer[NUM_PIXELS * 3];
		it->asBytes(buffer);
		serial.Write(buffer, 3 * NUM_PIXELS);
	}
	return 0;
}