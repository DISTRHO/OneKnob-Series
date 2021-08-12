/*
 * (c) Copyright 2016, Sean Connelly (@velipso), https://sean.cm
 * MIT License
 * Project Home: https://github.com/velipso/sndfilter
 * dynamics compressor based on WebAudio specification:
 *   https://webaudio.github.io/web-audio-api/#the-dynamicscompressornode-interface
 * Adapted on 2021 by Jan Janssen <jan@moddevices.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <math.h>
#include <string.h>

// samples per update; the compressor works by dividing the input chunks into even smaller sizes,
// and performs heavier calculations after each mini-chunk to adjust the final envelope
#define SF_COMPRESSOR_SPU        32

typedef struct {
	float threshold;
	float knee;
	float linearpregain;
	float linearthreshold;
	float slope;
	float attacksamplesinv;
	float satreleasesamplesinv;
	float k;
	float kneedboffset;
	float linearthresholdknee;
	float mastergain;
	float a; // adaptive release polynomial coefficients
	float b;
	float c;
	float d;
	float detectoravg;
	float compgain;
	float maxcompdiffdb;
	int samplerate;
	float ang90;
	float ang90inv;
} sf_compressor_state_st;

static inline float lin2db(float lin){ // linear to dB
	return 20.0f * log10f(lin);
}

static inline float cmop_db2lin(float db){ // dB to linear
	return powf(10.0f, 0.05f * db);
}

// for more information on the knee curve, check out the compressor-curve.html demo + source code
// included in this repo
static inline float kneecurve(float x, float k, float linearthreshold){
	return linearthreshold + (1.0f - expf(-k * (x - linearthreshold))) / k;
}

static inline float kneeslope(float x, float k, float linearthreshold){
	return k * x / ((k * linearthreshold + 1.0f) * expf(k * (x - linearthreshold)) - 1);
}

static inline float compcurve(float x, float k, float slope, float linearthreshold,
	float linearthresholdknee, float threshold, float knee, float kneedboffset){
	if (x < linearthreshold)
		return x;
	if (knee <= 0.0f) // no knee in curve
		return cmop_db2lin(threshold + slope * (lin2db(x) - threshold));
	if (x < linearthresholdknee)
		return kneecurve(x, k, linearthreshold);
	return cmop_db2lin(kneedboffset + slope * (lin2db(x) - threshold - knee));
}

// for more information on the adaptive release curve, check out adaptive-release-curve.html demo +
// source code included in this repo
static inline float adaptivereleasecurve(float x, float a, float b, float c, float d){
	// a*x^3 + b*x^2 + c*x + d
	float x2 = x * x;
	return a * x2 * x + b * x2 + c * x + d;
}

static inline float clampf(float v, float min, float max){
	return v < min ? min : (v > max ? max : v);
}

static inline float fixf(float v, float def){
	if (isnan(v) || isinf(v))
		return def;
	return v;
}

static void compressor_init(sf_compressor_state_st *state, int samplerate)
{
	state->samplerate = samplerate;
	state->detectoravg = 0.0f;
	state->compgain = 1.0f;
	state->maxcompdiffdb = -1.0f;

	state->ang90 = (float)M_PI * 0.5f;
	state->ang90inv = 2.0f / (float)M_PI;
}

// this is the main initialization function
// it does a bunch of pre-calculation so that the inner loop of signal processing is fast
static void compressor_set_params(sf_compressor_state_st *state, float threshold,
	float knee, float ratio, float attack, float release, float makeup)
{
	// useful values
	float linearthreshold = cmop_db2lin(threshold);
	float slope = 1.0f / ratio;
	float attacksamples = state->samplerate * attack;
	float attacksamplesinv = 1.0f / attacksamples;
	float releasesamples = state->samplerate * release;
	float satrelease = 0.0025f; // seconds
	float satreleasesamplesinv = 1.0f / ((float)state->samplerate * satrelease);

	// calculate knee curve parameters
	float k = 5.0f; // initial guess
	float kneedboffset = 0.0f;
	float linearthresholdknee = 0.0f;
	if (knee > 0.0f){ // if a knee exists, search for a good k value
		float xknee = cmop_db2lin(threshold + knee);
		float mink = 0.1f;
		float maxk = 10000.0f;
		// search by comparing the knee slope at the current k guess, to the ideal slope
		for (int i = 0; i < 15; i++){
			if (kneeslope(xknee, k, linearthreshold) < slope)
				maxk = k;
			else
				mink = k;
			k = sqrtf(mink * maxk);
		}
		kneedboffset = lin2db(kneecurve(xknee, k, linearthreshold));
		linearthresholdknee = cmop_db2lin(threshold + knee);
	}

	// calculate a master gain based on what sounds good
	float fulllevel = compcurve(1.0f, k, slope, linearthreshold, linearthresholdknee,
		threshold, knee, kneedboffset);
	float mastergain = cmop_db2lin(makeup) * powf(1.0f / fulllevel, 0.6f);

	// calculate the adaptive release curve parameters
	// solve a,b,c,d in `y = a*x^3 + b*x^2 + c*x + d`
	// interescting points (0, y1), (1, y2), (2, y3), (3, y4)
	float y1 = releasesamples * 0.090f;
	float y2 = releasesamples * 0.160f;
	float y3 = releasesamples * 0.420f;
	float y4 = releasesamples * 0.980f;
	float a = (-y1 + 3.0f * y2 - 3.0f * y3 + y4) / 6.0f;
	float b = y1 - 2.5f * y2 + 2.0f * y3 - 0.5f * y4;
	float c = (-11.0f * y1 + 18.0f * y2 - 9.0f * y3 + 2.0f * y4) / 6.0f;
	float d = y1;

	// save everything
	state->threshold            = threshold;
	state->knee                 = knee;
	state->linearthreshold      = linearthreshold;
	state->slope                = slope;
	state->attacksamplesinv     = attacksamplesinv;
	state->satreleasesamplesinv = satreleasesamplesinv;
	state->k                    = k;
	state->kneedboffset         = kneedboffset;
	state->linearthresholdknee  = linearthresholdknee;
	state->mastergain           = mastergain;
	state->a                    = a;
	state->b                    = b;
	state->c                    = c;
	state->d                    = d;
}

static void compressor_process(sf_compressor_state_st *state, int size,
                               const float *input_L, const float *input_R,
                               float *output_L, float *output_R)
{
	// pull out the state into local variables
	float threshold            = state->threshold;
	float knee                 = state->knee;
	float linearthreshold      = state->linearthreshold;
	float slope                = state->slope;
	float attacksamplesinv     = state->attacksamplesinv;
	float satreleasesamplesinv = state->satreleasesamplesinv;
	float k                    = state->k;
	float kneedboffset         = state->kneedboffset;
	float linearthresholdknee  = state->linearthresholdknee;
	float mastergain           = state->mastergain;
	float a                    = state->a;
	float b                    = state->b;
	float c                    = state->c;
	float d                    = state->d;
	float detectoravg          = state->detectoravg;
	float compgain             = state->compgain;
	float maxcompdiffdb        = state->maxcompdiffdb;

	int chunks = size > SF_COMPRESSOR_SPU ? size / SF_COMPRESSOR_SPU : 1;
	int spu = size > SF_COMPRESSOR_SPU ? SF_COMPRESSOR_SPU : size;
	int samplepos = 0;

	for (int ch = 0; ch < chunks; ch++)
	{
		detectoravg = fixf(detectoravg, 1.0f);
		float desiredgain = detectoravg;
		float scaleddesiredgain = asinf(desiredgain) * state->ang90inv;
		float compdiffdb = lin2db(compgain / scaleddesiredgain);

		// calculate envelope rate based on whether we're attacking or releasing
		float enveloperate;
		if (compdiffdb < 0.0f){ // compgain < scaleddesiredgain, so we're releasing
			compdiffdb = fixf(compdiffdb, -1.0f);
			maxcompdiffdb = -1; // reset for a future attack mode
			// apply the adaptive release curve
			// scale compdiffdb between 0-3
			float releasesamples = adaptivereleasecurve(((clampf(compdiffdb, -12.0f, 0.0f) + 12.0f) * 0.25f), a, b, c, d);
			enveloperate = cmop_db2lin(5.0f / releasesamples);
		}
		else{ // compresorgain > scaleddesiredgain, so we're attacking
			compdiffdb = fixf(compdiffdb, 1.0f);
			if (maxcompdiffdb == -1 || maxcompdiffdb < compdiffdb)
				maxcompdiffdb = compdiffdb;
			float attenuate = maxcompdiffdb;
			if (attenuate < 0.5f)
				attenuate = 0.5f;
			enveloperate = 1.0f - powf(0.25f / attenuate, attacksamplesinv);
		}

		// process the chunk
		for (int chi = 0; chi < spu; chi++, samplepos++)
		{
			float inputmax;
			inputmax = fabs(input_L[samplepos]) > fabs(input_R[samplepos]) ? fabs(input_L[samplepos]) : fabs(input_R[samplepos]);

			float attenuation;
			if (inputmax < 0.0001f)
			{
				attenuation = 1.0f;
			}
			else
			{
				float inputcomp = compcurve(inputmax, k, slope, linearthreshold,
					linearthresholdknee, threshold, knee, kneedboffset);
				attenuation = inputcomp / inputmax;
			}

			float rate;
			if (attenuation > detectoravg)
			{ // if releasing
				float attenuationdb = -lin2db(attenuation);
				if (attenuationdb < 2.0f)
					attenuationdb = 2.0f;
				float dbpersample = attenuationdb * satreleasesamplesinv;
				rate = cmop_db2lin(dbpersample) - 1.0f;
			}
			else
				rate = 1.0f;

			detectoravg += (attenuation - detectoravg) * rate;
			if (detectoravg > 1.0f)
				detectoravg = 1.0f;
			detectoravg = fixf(detectoravg, 1.0f);

			if (enveloperate < 1) // attack, reduce gain
			{
				compgain += (scaleddesiredgain - compgain) * enveloperate;
			}
			else
			{ // release, increase gain
				compgain *= enveloperate;
				if (compgain > 1.0f)
					compgain = 1.0f;
			}

			// apply the gain
			output_L[samplepos] = input_L[samplepos] * mastergain * sinf(state->ang90 * compgain);
			output_R[samplepos] = input_R[samplepos] * mastergain * sinf(state->ang90 * compgain);
		}
	}

	state->detectoravg   = detectoravg;
	state->compgain      = compgain;
	state->maxcompdiffdb = maxcompdiffdb;
}
