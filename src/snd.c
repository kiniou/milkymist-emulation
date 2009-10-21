/*
 * Milkymist Emulation Libraries
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>

#include <hal/snd.h>

void snd_init()
{
	printf("SND: ALSA-based emulation\n");
}

void snd_isr_crrequest()
{
	printf("snd_isr_crrequest() should not be called in emulation mode\n");
}

void snd_isr_crreply()
{
	printf("snd_isr_crreply() should not be called in emulation mode\n");
}

void snd_isr_dmar()
{
	printf("snd_isr_dmar() should not be called in emulation mode\n");
}

void snd_isr_dmaw()
{
	printf("snd_isr_dmaw() should not be called in emulation mode\n");
}

unsigned int snd_ac97_read(unsigned int addr)
{
	return 0;
}

void snd_ac97_write(unsigned int addr, unsigned int value)
{
	/* do nothing */
}

static snd_pcm_t *playback_handle;

void snd_play_empty()
{
}

int snd_play_refill(short *buffer)
{
}

/*
 * http://www.cs.utk.edu/~vose/linux/equalarea.com/alsa-audio.html
 * http://www.alsa-project.org/alsa-doc/alsa-lib/group___p_c_m.html
 */

void snd_play_start(snd_callback callback, unsigned int nsamples, void *user)
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;

	if((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "cannot open audio device (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_set_rate(playback_handle, hw_params, 48000, 0)) < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
		exit(1);
	}

	snd_pcm_hw_params_free(hw_params);

	if((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		fprintf(stderr, "cannot allocate software parameters structure (%s)\n", snd_strerror(err));
		exit(1);
	}
	if((err = snd_pcm_sw_params_current(playback_handle, sw_params)) < 0) {
		fprintf(stderr, "cannot initialize software parameters structure (%s)\n", snd_strerror(err));
		exit(1);
	}
	if((err = snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, nsamples)) < 0) {
		fprintf(stderr, "cannot set minimum available count (%s)\n", snd_strerror(err));
		exit(1);
	}
	if((err = snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, 0U)) < 0) {
		fprintf(stderr, "cannot set start mode (%s)\n", snd_strerror(err));
		exit(1);
	}
	if((err = snd_pcm_sw_params(playback_handle, sw_params)) < 0) {
		fprintf(stderr, "cannot set software parameters (%s)\n", snd_strerror(err));
		exit(1);
	}

	if((err = snd_pcm_prepare(playback_handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		exit(1);
	}
}

void snd_play_stop()
{
}

int snd_play_active()
{
}

void snd_record_empty()
{
}

int snd_record_refill(short *buffer)
{
}

void snd_record_start(snd_callback callback, unsigned int nsamples, void *user)
{
}

void snd_record_stop()
{
}

int snd_record_active()
{
}

