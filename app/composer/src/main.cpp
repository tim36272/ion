#include "ionlib/log.h"
#include "ionlib/net.h"
#include "ionlib/serial.h"
#include "ionlib/time.h"
#include "ionlib/math.h"
#include "ionlib/timer.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <float.h>

#define NUM_PIXELS (150)
//This is kind of a hack: in order to make everything work when
//	the color is forced off, you *must* supply a non-zero intensity.
//	In this case the color is off but the scale is 0.0 when the outside the
//	stencil, which is interpreted as a "don't care" color instead of "force 
//	to off". This is so that pixels outside the stencil don't get set to off.
//	Note that it is not sufficient to just supply this intensity: you must also
//	define the "off" color in you color map.
#define INTENSITY_FORCE_OFF (1.0f)
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
			flag_off = false;
		}
		CRGB(uint32_t hex_color)
		{
			r = hex_color >> 16;
			g = (hex_color >> 8) & 0xFF;
			b = (hex_color) & 0xFF;
			flag_off = false;
		}
		CRGB& operator+= (const CRGB& rhs)
		{
			if (this->flag_off || rhs.flag_off)
			{
				*this = ion::CRGB::Off();
				return *this;
			}
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
			if (this->flag_off || rhs.flag_off)
			{
				return ion::CRGB::Off();
			}
			ion::CRGB result = *this;
			result.r += rhs.r;
			result.g += rhs.g;
			result.b += rhs.b;
			return result;
		}
		CRGB operator* (const float scale)
		{
			//See note at top of this file regarding flag_off and 0 intensity
			if (this->flag_off && scale != 0.0f)
			{
				return ion::CRGB::Off();
			}
			ion::CRGB result = *this;
			result.r = (color_t)(result.r * scale + 0.5f); //0.5 added for rounding
			result.g = (color_t)(result.g * scale + 0.5f); //0.5 added for rounding
			result.b = (color_t)(result.b * scale + 0.5f); //0.5 added for rounding
			result.flag_off = false;
			return result;
		}
		static CRGB Off()
		{
			ion::CRGB result;
			result.flag_off = true;
			result.r = 0;
			result.g = 0;
			result.b = 0;
			return result;
		}

		color_t r;
		color_t g;
		color_t b;
		bool flag_off : 1;
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
		ion::CRGB getAtLinear(float this_pos) const
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
				// with the off flag unset (so that the entire string doesn't get set to off)
				ion::CRGB result = color_[0];
				if (this_pos < stop_pos_[0])
				{
					result.flag_off = false;
				}
				return result;
			}
			if (this_pos > stop_pos_[num_stops_ - 1])
			{
				//For positions before the start of the string: the last color is returned
				// with the off flag unset (so that the entire string doesn't get set to off)
				ion::CRGB result = color_[num_stops_ - 1];
				result.flag_off = false;
				return result;
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
			output.flag_off = start_color.flag_off | end_color.flag_off;
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
		virtual float getIntensityAt(float pos, float tov) = 0;
		virtual PixelGroupBase* clone() const = 0;
	};

	struct PixelGroupLinear : PixelGroupBase
	{
		PixelGroupLinear(float intensity)
		{
			intensity_ = intensity;
		}
		float getIntensityAt(float pos, float)
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
		float getIntensityAt(float pos, float)
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

	struct PixelGroupLFSRRandom : PixelGroupBase
	{
		PixelGroupLFSRRandom(float intensity, float probability, uint8_t seed)
		{
			intensity_ = intensity;
			probability_ = (uint8_t)(probability * 255);
			seed_ = seed;
			LOGASSERT(seed_ != 0);
		}
		float getIntensityAt(float pos, float)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}
			//Figure out which index this position is at
			uint16_t pixel_index = (uint16_t)(pos * NUM_PIXELS + 0.5)+1;
			//Pump the LFSR for pixel_index iterations
			uint8_t state = seed_;
			for (; pixel_index > 0; --pixel_index)
			{
				uint8_t and_result = state & bit_mask_;
				//sum the number of 1's in the result
				uint8_t num_ones = 0;
				for (uint8_t bit_index = 0; bit_index < 8; ++bit_index)
				{
					num_ones += and_result & 0x1;
					and_result = and_result >> 1;
				}
				num_ones %= 2;
				state = state >> 1;
				state |= num_ones << 7;
			}
			//interpret the state as a randomly-generated value and check if it is less than probability_

			if (state < probability_)
			{
				return intensity_;
			} else
			{
				return 0;
			}
		}
		virtual PixelGroupLFSRRandom* clone() const
		{
			return new PixelGroupLFSRRandom(*this);
		}
		float intensity_;
		uint8_t probability_;
		uint8_t seed_;
		static const uint8_t bit_mask_ = 0b10101010;
	};

	struct PixelGroupGaussian : PixelGroupBase
	{
		PixelGroupGaussian(float mean, float std_dev)
		{
			mean_ = mean;
			std_dev_ = std_dev;
			normalizer_ = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));
		}
		float getIntensityAt(float pos, float)
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
	struct PixelGroupLinearTranslating : PixelGroupBase
	{
		PixelGroupLinearTranslating(float intensity, float start_tov, float stop_tov, float width)
		{
			intensity_ = intensity;
			start_tov_ = start_tov;
			stop_tov_ = stop_tov;
			width_ = width;
			if (width_ < 0.0f)
			{
				width_ *= -1.0f;
				reversed_ = true;
			} else
			{
				reversed_ = false;
			}
		}
		float getIntensityAt(float pos, float tov)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}
			if (reversed_)
			{
				pos = 1.0f - pos;
			}
			if (tov < start_tov_ || tov > stop_tov_)
			{
				return 0.0f;
			}
			//normalize the tov to the start/stop range
			tov -= start_tov_;
			tov /= (stop_tov_ - start_tov_);
			//TOV is now 0 at start_tov_ and 1.0 at stop_tov
			tov *= 1.0f + width_;
			tov -= width_ / 2.0f;

			//compute where the middle of the segment is at this tov
			float center = tov;
			if (pos > center - width_ / 2 && pos < center + width_ / 2)
			{
				return intensity_;
			} else
			{
				return 0.0f;
			}
		}
		virtual PixelGroupLinearTranslating* clone() const
		{
			return new PixelGroupLinearTranslating(*this);
		}
		float intensity_;
		float start_tov_;
		float stop_tov_;
		float width_;
		bool reversed_; //go right to left instead of left to right
	};

	//This group looks like:
	//For the first half of the animation, a single gaussian in the center grows from 0 intensity to maximum
	//For the second half, that gaussian splits into two which translate away from the center, ramping down in maximum intensity to 0 at the end
	struct PixelGroupSplash : PixelGroupBase
	{
		PixelGroupSplash(float intensity, float std_dev, float start_tov, float relative_impact_tov, float stop_tov)
		{
			intensity_ = intensity;
			start_tov_ = start_tov;
			impact_tov_ = relative_impact_tov;
			stop_tov_ = stop_tov;
			std_dev_ = std_dev;
			normalizer_ = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));

		}
		float getIntensityAt(float pos, float tov_in)
		{
			//clamp the result
			if (pos < 0.0f || pos > 1.0f)
			{
				return 0.0f;
			}
			if (tov_in < start_tov_ || tov_in > stop_tov_)
			{
				return 0.0f;
			}
			//normalize the tov_in to the start/stop range
			float tov = (tov_in - start_tov_) / (stop_tov_ - start_tov_);
			//tov is now a number in [0.0,1.0] representing where we are in the animation
			if (tov < impact_tov_)
			{
				//ramping up gaussian
				float frac = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));
				float exponent = -0.5f * ((pos - 0.5f) * (pos - 0.5f)) / (std_dev_*std_dev_);
				float result = frac * exp(exponent);
				return result / normalizer_ * intensity_ * tov / impact_tov_;
			} else
			{
				LOGSANITY(tov <= 1.0f);
				//splitting gaussian
				//Compute the position of the peak (on one side) in the range [0.0,1.0-impact_tov_]
				float peak_position = ((1.0f - impact_tov_) - (tov - impact_tov_))/(1.0f-impact_tov_)/2.0f;
				//compute the normalized pos (i.e. distance from peak)
				if (pos > 0.5f)
				{
					pos = 1.0f - pos;
				}
				float distance_from_peak = fabs(peak_position - pos);
				//compute the gaussian at this position
				float frac = 1.0f / (sqrtf(2.0f * (float)PI * std_dev_ * std_dev_));
				float exponent = -0.5f * (distance_from_peak * distance_from_peak) / (std_dev_*std_dev_);
				float result = frac * exp(exponent) / normalizer_;
				//fade it out towards the edges by computing how far we are until the end
				result *= (1.0f - tov) / (1.0f - impact_tov_);
				LOGASSERT(result <= 1.0f);
				return result * intensity_;
			}
		}
		virtual PixelGroupSplash* clone() const
		{
			return new PixelGroupSplash(*this);
		}
		float intensity_;
		float std_dev_;
		float normalizer_;
		float start_tov_;
		float impact_tov_;
		float stop_tov_;
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
		KeyFrame()
		{
			is_all_black_ = false;
		}
		KeyFrame(float tov)
		{
			tov_ = tov;
			is_all_black_ = false;
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
			frame.is_all_black_ = true;
			return frame;
		}
		ion::CRGB getTotalValueAt(float norm_pixel_position, float tov) const
		{
			//compute the desired color per the previous keyframe
			ion::CRGB total_value;

			for (uint32_t map_index = 0; map_index < color_map_.size(); ++map_index)
			{
				const ion::ColorMap *this_color_map = &color_map_[map_index];
				const ion::PixelMap *this_pixel_map = &pixel_map_[map_index];
				float pixel_map_extent = (this_pixel_map->end_ - this_pixel_map->start_);
				LOGASSERT(pixel_map_extent >= 0.0f && pixel_map_extent <= 1.0f);
				float stencil_position = (norm_pixel_position - this_pixel_map->start_ )/ pixel_map_extent;
				float intensity = this_pixel_map->stencil_->getIntensityAt(stencil_position, tov);
				LOGASSERT(intensity <= 1.0f && intensity >= 0.0f);
				total_value += this_color_map->getAtLinear(stencil_position) * intensity;
			}
			return total_value;
		}
		float tov_;
		std::vector<ColorMap> color_map_;
		std::vector<PixelMap> pixel_map_;
		bool is_all_black_;
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
#define NUM_EXTRA_FRAMES (0) //use this to force black at the end
std::vector<ion::OutputFrame> buildAnimation(float duration, const ion::Program& program, float fps)
{
	uint32_t num_frames = static_cast<uint32_t>(duration * fps)+ NUM_EXTRA_FRAMES; //+NUM_EXTRA_FRAMES to ensure we actually end at black
	std::vector<ion::OutputFrame> output_frames(num_frames);
	ion::KeyFrame previous_black = ion::KeyFrame::Black(0.0);
	ion::KeyFrame next_black = ion::KeyFrame::Black(duration);

	for (uint32_t frame_index = 0; frame_index < num_frames - NUM_EXTRA_FRAMES; ++frame_index)
	{
		float frame_tov = (float)frame_index / fps;
		//for each channel
		for (uint32_t channel_index = 0; channel_index < program.channels_.size(); ++channel_index)
		{
			const ion::Channel *this_channel = &program.channels_[channel_index];
			const ion::KeyFrame* previous_keyframe = &previous_black;
			const ion::KeyFrame* next_keyframe = &this_channel->frames_[0];
			uint32_t next_keyframe_index = 0;

			//check if we need to advance keyframes
			while (frame_tov > next_keyframe->tov_)
			{
				previous_keyframe = next_keyframe;
				next_keyframe_index++;
				if (next_keyframe_index < this_channel->frames_.size())
				{
					next_keyframe = &this_channel->frames_[next_keyframe_index];
				} else
				{
					//Go to black for the rest of the sequence
					next_keyframe = &next_black;
				}
			}

			//if both previous and next are black then continue
			if (previous_keyframe->is_all_black_ && next_keyframe->is_all_black_)
			{
				continue;
			}

			float alpha_t = (frame_tov - previous_keyframe->tov_) / (next_keyframe->tov_ - previous_keyframe->tov_);
			//Generate each pixel
			for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
			{
				//compute the desired color per the previous keyframe
				ion::CRGB previous_value = previous_keyframe->getTotalValueAt((float)pixel_index / NUM_PIXELS, frame_tov);
				ion::CRGB next_value = next_keyframe->getTotalValueAt((float)pixel_index / NUM_PIXELS, frame_tov);
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
#define SUNRISE_DIRT ion::CRGB(0x663300)
#define SUNRISE_GOURD_RED ion::CRGB(0x881100)

#define STAR_DURATION (10.0f)

//positions
#define HORIZON_START (0.0f)
#define HORIZON_END (1.0f)

#define SONG_BEGIN (0.0f)
#define SONG_END (236.0f)
#define INTRO_BEGIN SONG_BEGIN
#define INTRO_END (51.0f)
#define VERSE1_BEGIN INTRO_END
#define MUSIC_INTENSITIES (97.0) // "It's the circle of life" from the first verse
#define VERSE1_END (142.0f) //music calms
#define VERSE2_BEGIN (187.0f)
#define VERSE2_END (234.0f)

#define STARS_BEGIN (SONG_BEGIN)
#define STARS_END (INTRO_END)
#define STARS_DURATION (STARS_END - STARS_BEGIN)

#define WATER_DROPLET_DURATION (0.5f)
#define WATER_DROPLET_IMPACT (0.5)
#define PEBBLE_DURATION (0.2f)
#define PEBBLE_IMPACT (0.5f)
#define ANIMAL_WIDTH (0.1f)
#define LIGHT_WIDTH (0.05f)
struct discrete_event_t
{
	float start_time;
	float duration;
	float start_pos;
	float end_pos;
	float end_time()
	{
		return start_time + duration;
	}
	float mid()
	{
		return start_time + duration / 2.0f;
	}
	ion::CRGB color;
	uint8_t channel; //if needed, channel offset
	//Splash: percentage of the way through the event where the gaussian peaks
	//Linear translating: width
	float implementation_param;
};

discrete_event_t droplets_g[] = {
	//Start  |Durtion                |start position        |end position          |color              |c|Impact
	{ 22.3f,  WATER_DROPLET_DURATION, HORIZON_START,         HORIZON_START + 0.3f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT },  //pelican walking in water
	{ 23.3f,  WATER_DROPLET_DURATION, HORIZON_START+0.3,     HORIZON_START + 0.6f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //pelican walking in water
	{ 26.0f,  WATER_DROPLET_DURATION, HORIZON_START+0.6,     HORIZON_START + 0.9f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //pelican walking in water
	{ 88.1f,  WATER_DROPLET_DURATION, HORIZON_START,         HORIZON_START + 0.1f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 88.35f, WATER_DROPLET_DURATION, HORIZON_START + 0.1f,  HORIZON_START + 0.2f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 88.60f, WATER_DROPLET_DURATION, HORIZON_START + 0.2f,  HORIZON_START + 0.3f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 88.85f, WATER_DROPLET_DURATION, HORIZON_START + 0.3f,  HORIZON_START + 0.4f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 89.10f, WATER_DROPLET_DURATION, HORIZON_START + 0.5f,  HORIZON_START + 0.6f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 89.35f, WATER_DROPLET_DURATION, HORIZON_START + 0.6f,  HORIZON_START + 0.7f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 90.50f, WATER_DROPLET_DURATION, HORIZON_START + 0.7f,  HORIZON_START + 0.8f,  SUNRISE_SKY_BLUE,  0, WATER_DROPLET_IMPACT }, //zebras running through water
	{ 151.0f, PEBBLE_DURATION,        HORIZON_START + 0.7f,  HORIZON_START + 0.8f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 151.3f, PEBBLE_DURATION + 0.05, HORIZON_START + 0.8f,  HORIZON_START + 0.9f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 151.6f, PEBBLE_DURATION + 0.1,  HORIZON_START + 0.6f,  HORIZON_START + 0.7f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 151.9f, PEBBLE_DURATION + 0.15, HORIZON_START + 0.85f, HORIZON_START + 0.95f, SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 152.2f, PEBBLE_DURATION + 0.2,  HORIZON_START + 0.7f,  HORIZON_START + 0.8f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 153.4f, PEBBLE_DURATION,        HORIZON_START + 0.8f,  HORIZON_START + 0.9f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 153.5f, PEBBLE_DURATION + 0.05, HORIZON_START + 0.6f,  HORIZON_START + 0.7f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 153.8f, PEBBLE_DURATION + 0.1,  HORIZON_START + 0.8f,  HORIZON_START + 0.9f,  SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 154.4f, PEBBLE_DURATION,        HORIZON_START + 0.75f, HORIZON_START + 0.85f, SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 155.6f, PEBBLE_DURATION + 0.1,  HORIZON_START + 0.65f, HORIZON_START + 0.75f, SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 155.9f, PEBBLE_DURATION,        HORIZON_START + 0.85f, HORIZON_START + 0.95f, SUNRISE_DIRT,      0, PEBBLE_IMPACT },        //Rafiki shaking gourd
	{ 158.0f, 1.0,                    HORIZON_START + 0.5f,  HORIZON_END,           SUNRISE_GOURD_RED, 0, 0.2 },        //Rafiki breaks gourd
	{ 169.4f, 1.1,                    HORIZON_START,         HORIZON_END,           SUNRISE_SKY_BLUE,  0, 0.65 },        //Simba sneezes
	{ 206.5f, 1.5,                    HORIZON_START,         HORIZON_END,           SUNRISE_GOURD_RED, 0, 0.2 },        //Love
};
static const uint8_t num_droplets_g = sizeof(droplets_g) / sizeof(discrete_event_t);

discrete_event_t animals_passing_g[] = {
	//Start time|Durtion|start position|end position
	{ 46.0f,      0.75f  ,HORIZON_START + 0.5,HORIZON_START + 0.75, ion::CRGB::Off(), ANIMAL_WIDTH}, //wings flap
	{ 74.0f,      2.0f   ,HORIZON_START + 0.5,HORIZON_START + 0.75, ion::CRGB::Off(), ANIMAL_WIDTH}, //Animals running
	{ 83.5f,      1.0f   ,HORIZON_START + 0.5,HORIZON_START + 0.75, ion::CRGB::Off(), ANIMAL_WIDTH}, //Animals running
	{ 192.2f,     0.5f   ,HORIZON_START,      HORIZON_END,          ion::CRGB::Off(), 0.4 },          //Elephant yelling
	{ 194.5f,     0.3f   ,HORIZON_START + 0.1,HORIZON_START + 0.2, ion::CRGB::Off(), 0.3 },          //Baboons yelling
	{ 194.8f,     0.3f   ,HORIZON_START + 0.2,HORIZON_START + 0.3, ion::CRGB::Off(), -0.3 },          //Baboons yelling
	{ 195.4f,     0.3f   ,HORIZON_START + 0.1,HORIZON_START + 0.2, ion::CRGB::Off(), 0.3 },          //Baboons yelling
	{ 195.7f,     0.3f   ,HORIZON_START + 0.3,HORIZON_START + 0.4, ion::CRGB::Off(), -0.3 },          //Baboons yelling
	{ 197.0f,     0.3f   ,HORIZON_START + 0.2,HORIZON_START + 0.3, ion::CRGB::Off(), 0.3 },          //Baboons yelling
	{ 197.3f,     0.1f   ,HORIZON_START + 0.2,HORIZON_START + 0.3, ion::CRGB::Off(), -0.3 },          //Baboons yelling
	{ 197.9f,     0.1f   ,HORIZON_START + 0.1,HORIZON_START + 0.2, ion::CRGB::Off(), 0.3 },          //Baboons yelling
	{ 198.1f,     0.1f   ,HORIZON_START + 0.3,HORIZON_START + 0.4, ion::CRGB::Off(), -0.3 },         //Baboons yelling
	{ 198.3f,     0.1f   ,HORIZON_START + 0.3,HORIZON_START + 0.4, ion::CRGB::Off(), 0.3 },          //Baboons yelling
	{ 198.5f,     0.1f   ,HORIZON_START + 0.1,HORIZON_START + 0.2, ion::CRGB::Off(), -0.3 },         //Baboons yelling
	{ 199.0f,     0.25f   ,HORIZON_START + 0.2,HORIZON_START + 0.3, ion::CRGB::Off(), 0.3 },         //Baboons yelling
	{ 199.25f,     0.25f   ,HORIZON_START + 0.3,HORIZON_START + 0.4, ion::CRGB::Off(), -0.3 },       //Baboons yelling
	{ 199.50f,     0.25f   ,HORIZON_START + 0.1,HORIZON_START + 0.2, ion::CRGB::Off(), 0.3},          //Baboons yelling
};
static const uint8_t num_animals_g = sizeof(animals_passing_g) / sizeof(discrete_event_t);

discrete_event_t circle_of_light_g[] = {
	//Start  |Durtion                |start position        |end position          |color              |c|Impact

	{ 96.5f,      1.5f,   HORIZON_START, HORIZON_END, SUNRISE_WHITE, 0, LIGHT_WIDTH },
	{ 132.6f,     1.5f,   HORIZON_START, HORIZON_END, SUNRISE_WHITE, 0, LIGHT_WIDTH },
	{ 189.5f,     1.5f,   HORIZON_START, HORIZON_END, SUNRISE_WHITE, 0, LIGHT_WIDTH * -1.0f },
	{ 224.5f,    5.0f,    HORIZON_START, HORIZON_END, SUNRISE_WHITE, 0, LIGHT_WIDTH },
	{ 224.5f,    5.0f,    HORIZON_START, HORIZON_END, SUNRISE_WHITE, 1, LIGHT_WIDTH * -1.0f },
};
static const uint8_t num_light_circles_g = sizeof(circle_of_light_g) / sizeof(discrete_event_t);

//#define BIRDS_CALLING1_BEGIN (26.0f) //flock of birds flying
//#define BIRDS_CALLING1_END (31.0f)
//
//#define BIRDS_CALLING2_BEGIN (38.0f) //flock of birds flying
//#define BIRDS_CALLING2_END (45.0f)
//
//#define BIRDS_CALLING3_BEGIN (76.0f) //Dodos running
//#define BIRDS_CALLING3_END (81.0f)
//
//#define BIRDS_CALLING4_BEGIN (84.0f) //Single dodo runs very close
//#define BIRDS_CALLING4_END (85.5f)

#define SUN_ENABLE
#define STARS_ENABLE
#define ANIMAL_ENABLE
//#define FINAL_LIGHT_ENABLE
#define DROPLET_ENABLE
#define PEBBLES_ENABLE
#define CIRCLE_OF_LIGHT_ENABLE



int main(int argc, char* argv[])
{
	ion::LogInit("composer.log");
	ion::InitSockets();
	//log how many times I have run this
	{
		std::ifstream fin("num_runs.txt");
		int num_runs;
		fin >> num_runs;
		fin.close();
		std::ofstream fout("num_runs.txt");
		fout << num_runs + 1;
		fout.close();
	}
	ion::Program sunrise;
	ion::UdpSocket sock;
	sock.Create();
	ion::IpPort tx_ipport(5002, ion::IpPort::Order::HOST);
	ion::IpPort rx_ipport(5003, ion::IpPort::Order::HOST);
	sock.Bind(rx_ipport);
	float rx_timestamp;

	const float frame_rate = 32.091f;
	const float duration = SONG_END;

#ifdef SUN_ENABLE
	///////////////////////////////////////////////////////////////////////////
	//	sun channel
	///////////////////////////////////////////////////////////////////////////
	ion::PixelMap sun_pix(HORIZON_START, HORIZON_END, ion::PixelGroupGaussian(0.5f, 0.5f));
	ion::ColorMap sun_palett(5);
	sun_palett.addStop(0.0, SUNRISE_SKY_BLUE);
	sun_palett.addStop(0.25, SUNRISE_SKY_ORANGE);
	sun_palett.addStop(0.5, SUNRISE_SUN);
	sun_palett.addStop(0.75, SUNRISE_SKY_ORANGE);
	sun_palett.addStop(1.0, SUNRISE_SKY_BLUE);

	ion::KeyFrame sun_frame(SONG_END);
	sun_frame.addLayer(sun_palett, sun_pix);

	ion::Channel sun_channel;
	sun_channel.addFrame(sun_frame);
	sunrise.addChannel(sun_channel);
#endif

#ifdef STARS_ENABLE
	///////////////////////////////////////////////////////////////////////////
	//	Stars channel
	///////////////////////////////////////////////////////////////////////////
	ion::ColorMap stars_palett(2);
	stars_palett.addStop(0.0, SUNRISE_WHITE);
	stars_palett.addStop(1.0, SUNRISE_WHITE);
	for (uint32_t stars_layer = 0; stars_layer < 20; ++stars_layer)
	{
		ion::Channel stars_channel;
		float frame_duration = (float)ion::randlf(STARS_DURATION/12, STARS_DURATION/6.0);
		float tov = (float)ion::randlf(STARS_BEGIN + frame_duration/2.0f, STARS_END - frame_duration/2.0f);
		float star_intensity = 0.5f * (1.0f - ((tov - STARS_BEGIN) / STARS_DURATION));
		ion::PixelMap stars_pix(HORIZON_START, HORIZON_END, ion::PixelGroupLFSRRandom(star_intensity, 0.05f, (uint8_t)ion::randull(1,255)));
		ion::KeyFrame start_black = ion::KeyFrame::Black(tov - frame_duration /2.0f);
		ion::KeyFrame stars_frame((float)tov);
		ion::KeyFrame end_black = ion::KeyFrame::Black(tov + frame_duration /2.0f);
		stars_frame.addLayer(stars_palett, stars_pix);

		stars_channel.addFrame(start_black);
		stars_channel.addFrame(stars_frame);
		stars_channel.addFrame(end_black);
		sunrise.addChannel(stars_channel);
	}
#endif

#ifdef ANIMAL_ENABLE
	///////////////////////////////////////////////////////////////////////////
	//animal channel
	///////////////////////////////////////////////////////////////////////////
	ion::Channel animals;
	for (uint32_t animal_index = 0; animal_index < num_animals_g; ++animal_index)
	{
		ion::PixelMap animal_pix(animals_passing_g[animal_index].start_pos,
								 animals_passing_g[animal_index].end_pos,
								 ion::PixelGroupLinearTranslating(INTENSITY_FORCE_OFF,
																  animals_passing_g[animal_index].start_time,
																  animals_passing_g[animal_index].end_time(),
																  animals_passing_g[animal_index].implementation_param)); //width
		ion::ColorMap animal_palett(2);
		animal_palett.addStop(0.0, animals_passing_g[animal_index].color);
		animal_palett.addStop(1.0, animals_passing_g[animal_index].color);
		ion::KeyFrame animal_start = ion::KeyFrame::Black(animals_passing_g[animal_index].start_time);
		ion::KeyFrame animal(animals_passing_g[animal_index].mid());
		ion::KeyFrame animal_end = ion::KeyFrame::Black(animals_passing_g[animal_index].end_time());
		animal.addLayer(animal_palett, animal_pix);
		animals.addFrame(animal_start);
		animals.addFrame(animal);
		animals.addFrame(animal_end);
	}
	sunrise.addChannel(animals);
#endif

#ifdef CIRCLE_OF_LIGHT_ENABLE
	///////////////////////////////////////////////////////////////////////////
	//Light band channel
	///////////////////////////////////////////////////////////////////////////
	ion::Channel circle_of_light[2];
	ion::ColorMap light_palett(2);
	light_palett.addStop(0.0, SUNRISE_WHITE);
	light_palett.addStop(1.0, SUNRISE_WHITE);
	for (uint32_t light_index = 0; light_index < num_light_circles_g; ++light_index)
	{
		ion::PixelMap light_pix(circle_of_light_g[light_index].start_pos,
								 circle_of_light_g[light_index].end_pos,
								 ion::PixelGroupLinearTranslating(1.0,
																  circle_of_light_g[light_index].start_time,
																  circle_of_light_g[light_index].end_time(),
																  circle_of_light_g[light_index].implementation_param)); //width--*

		ion::KeyFrame light_start = ion::KeyFrame::Black(circle_of_light_g[light_index].start_time);
		ion::KeyFrame light(circle_of_light_g[light_index].mid());
		ion::KeyFrame light_end = ion::KeyFrame::Black(circle_of_light_g[light_index].end_time());
		light.addLayer(light_palett, light_pix);
		circle_of_light[circle_of_light_g[light_index].channel].addFrame(light_start);
		circle_of_light[circle_of_light_g[light_index].channel].addFrame(light);
		circle_of_light[circle_of_light_g[light_index].channel].addFrame(light_end);
	}
	sunrise.addChannel(circle_of_light[0]);
	sunrise.addChannel(circle_of_light[1]);
#endif

#ifdef FINAL_LIGHT_ENABLE
	///////////////////////////////////////////////////////////////////////////
	//Final light channel
	///////////////////////////////////////////////////////////////////////////
	ion::Channel final_light;
	ion::PixelMap final_light_pix(HORIZON_START, HORIZON_END, ion::PixelGroupLinear(1.0));
	ion::ColorMap final_light_palett(2);
	final_light_palett.addStop(0.0, SUNRISE_SKY_BLUE);
	final_light_palett.addStop(1.0, SUNRISE_SKY_BLUE);
	ion::KeyFrame final_light_frame(SONG_END);
	final_light_frame.addLayer(final_light_palett, final_light_pix);
	final_light.addFrame(final_light_frame);
#endif

#ifdef DROPLET_ENABLE
	ion::Channel droplet;
	for (uint32_t drop_index = 0; drop_index < num_droplets_g; ++drop_index)
	{
		ion::PixelMap droplet_pix(droplets_g[drop_index].start_pos,
								droplets_g[drop_index].end_pos,
								ion::PixelGroupSplash(1.0f, //intensity
													  0.1f, //std_dev
													  droplets_g[drop_index].start_time,
													  droplets_g[drop_index].implementation_param, //impact time
													  droplets_g[drop_index].end_time()));
		ion::ColorMap droplet_palett(2);
		droplet_palett.addStop(0.0f, droplets_g[drop_index].color);
		droplet_palett.addStop(1.0f, droplets_g[drop_index].color);
		ion::KeyFrame droplet_start = ion::KeyFrame::Black(droplets_g[drop_index].start_time);
		ion::KeyFrame droplet_frame(droplets_g[drop_index].mid());
		ion::KeyFrame droplet_end = ion::KeyFrame::Black(droplets_g[drop_index].end_time());
		droplet_frame.addLayer(droplet_palett, droplet_pix);
		droplet.addFrame(droplet_start);
		droplet.addFrame(droplet_frame);
		droplet.addFrame(droplet_end);
	}
	sunrise.addChannel(droplet);
#endif

	//ion::Channel testbed;
	//ion::PixelMap testbed_pix(HORIZON_START, HORIZON_START + 0.1, ion::PixelGroupSplash(1.0, 0.15, 0.0, 0.2, 0.5));
	//ion::ColorMap testbed_palett(2);
	//water_palett.addStop(0.0, SUNRISE_SKY_BLUE);
	//water_palett.addStop(1.0, SUNRISE_SKY_BLUE);
	//ion::KeyFrame testbed_start = ion::KeyFrame::Black(0.0);
	//ion::KeyFrame testbed_frame(0.25);
	//ion::KeyFrame testbed_end = ion::KeyFrame::Black(0.5);
	//water_frame.addLayer(water_palett, water_pix);
	//water.addFrame(water_start);
	//water.addFrame(water_frame);
	//water.addFrame(water_end);
	//sunrise.addChannel(water);
	///////////////////////////////////////////////////////////////////////////
	//Generate program
	///////////////////////////////////////////////////////////////////////////
	std::vector<ion::OutputFrame> output_frames = buildAnimation(duration, sunrise, frame_rate);
	ion::Error result = sock.SendTo("circleOfLife.wav", sizeof("circleOfLife.wav")-1, ion::IpAddress("192.168.16.108"), tx_ipport);
	if (!result.success())
	{
		LOGFATAL("Failed to start music");
	}
	result = sock.Recv((byte_t*)&rx_timestamp, sizeof(rx_timestamp), 5000, nullptr, nullptr);
	if (result == ion::Error::Get(ion::Error::TIMEOUT))
	{
		//Something went wrong, don't start the stream
		LOGFATAL("Didn't receive start music ack within five seconds");
	}
	LOGINFO("Starting output");
	ion::Timer fps;
	//send a request to the music player to start the music
	//stream the frames to the target
	ion::Serial serial(6, (ion::Serial::Baud)1066666);
	double tov_time_origin = ion::TimeGet(); //this is the time we last resync'd and thus the time we are trying to stay relative to
	uint32_t tov_frame_origin = 0;
	for (uint32_t frame_index = 0; frame_index < output_frames.size(); ++frame_index)
	{
		float tov = frame_index / (float)frame_rate;
		std::vector<ion::OutputFrame>::iterator it = output_frames.begin() + frame_index;
		cv::Mat visual(50, NUM_PIXELS*3, CV_8UC3);
		for (uint32_t pixel_index = 0; pixel_index < NUM_PIXELS; ++pixel_index)
		{
			for (uint32_t row = 0; row < 50; ++row)
			{
				for (uint32_t col = 0; col < 3; ++col)
				{
					visual.at<cv::Vec3b>(row, pixel_index*3+col) = cv::Vec3b((uint8_t)it->pixels_[pixel_index].b, (uint8_t)it->pixels_[pixel_index].g, (uint8_t)it->pixels_[pixel_index].r);
				}
			}
		}
		byte_t buffer[NUM_PIXELS * 3];
		it->asBytes(buffer);
		fps.PeriodBegin();
		serial.Write(buffer, 3 * NUM_PIXELS);
		//Check if a timestamp update is available
		result = sock.Recv((byte_t*)&rx_timestamp, sizeof(rx_timestamp), 0, nullptr, nullptr);
		if (result != ion::Error::Get(ion::Error::TIMEOUT))
		{
			//compute the frame index of this time
			uint32_t measured_frame_index = (uint32_t)(rx_timestamp * frame_rate);
			if (measured_frame_index != frame_index)
			{
				//jump to that frame
				LOGINFO("Jumping from frame %u to frame %u", frame_index, measured_frame_index);
				frame_index = measured_frame_index;
				tov_time_origin = ion::TimeGet();
				tov_frame_origin = measured_frame_index;
			}
		}
		//Get the tov
		std::stringstream tov_str;
		tov_str << tov;
		cv::putText(visual, tov_str.str().c_str(), cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255));
		cv::imshow("visual", visual);
		cv::waitKey(1);
		//maintain the requested frame rate. Compute what time it should be at the end of this frame
		float desired_time = (frame_index + 1 - tov_frame_origin)/frame_rate + tov_time_origin;
		//Get current time
		float now = ion::TimeGet();
		if (now < desired_time)
		{
			ion::ThreadSleep((desired_time - now) * 1000.0);
		}
	}
	LOGINFO("%lf", 1.0/fps.GetMean());
	LOGINFO("Finished output");

	return 0;
}