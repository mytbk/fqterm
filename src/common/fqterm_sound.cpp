/***************************************************************************
 *   fqterm, a terminal emulator for both BBS and *nix.                    *
 *   Copyright (C) 2008 fqterm development group.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.               *
 ***************************************************************************/

#include <QSound>
#include <QFile>
#include <QMessageBox>
#include <stdio.h>
#include <stdlib.h>

/*******************************************************/
/********** OSS code is from OpenSound System **********/
/*******************************************************/
#ifdef AUDIO_OSS
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(_OS_FREEBSD_) || defined(_OS_LINUX_)
#include <sys/soundcard.h>
#define FQ_AUDIO_DEV    "/dev/dsp"
#else
#include <machine/soundcard.h>
#define FQ_AUDIO_DEV    "/dev/audio"
#endif

#define WAVE_FORMAT_PCM         0x0001
#define WAVE_FORMAT_ADPCM       0x0002
#define WAVE_FORMAT_ALAW        0x0006
#define WAVE_FORMAT_MULAW       0x0007
#define WAVE_FORMAT_IMA_ADPCM   0x0011

#ifndef AFMT_S32_LE
#define AFMT_S32_LE 0x00001000  /* Little endian signed 32-bit */
#endif

typedef struct {
  int coeff1, coeff2;
} adpcm_coeff;

#endif /* AUDIO_OSS */

/* 2008.07.06, ecore.cn@gmail.com, alsa sound for FQTerm ... */
#ifdef AUDIO_ALSA
#include <alsa/asoundlib.h>
#include <stdint.h>
typedef struct tag_FQ_WAVE_FILE_HEADER {
  uint32_t rid;     /* RIFF ID */
  uint32_t rsz;     /* RIFF SIZE */
  uint32_t wid;     /* WAVE ID */
}
FQWFH;

typedef struct tag_FQ_WAVE_FORMAT_HEADER {
  uint32_t fid;     /* FORMAT ID */
  uint32_t fsz;     /* FORMAT SIZE */
  uint16_t aid;     /* AUDIO ID */
  uint16_t ach;     /* AUDIO CHANNELS */
  uint32_t asr;     /* AUDIO SAMPLE RATE */
  uint32_t abr;     /* AUDIO BYTE RATE */
  uint16_t aba;     /* AUDIO BLOCK ALIGNMENT */
  uint16_t abps;    /* AUDIO BITS PER SAMPLE */
}
FQWMH;

typedef struct tag_FQ_WAVE_DATA_HEADER {
  uint32_t did;     /* DATA ID */
  uint32_t dsz;     /* DATA SIZE */
}
FQWDH;

#endif /* AUDIO_ALSA */

#include "common.h"
#include "fqterm_sound.h"

namespace FQTerm {

FQTermSound::FQTermSound(const QString &filename, QObject *parent,
  const char *name)
  : QThread(parent), soundFile_(filename) {
  connect(this, SIGNAL(finished()), SLOT(deleteInstance()));
  connect(this, SIGNAL(terminated()), SLOT(deleteInstance()));
}

FQTermSound::~FQTermSound() {
}

void FQTermSound::deleteInstance() {
  FQ_TRACE("sound", 3) << "sound instance [" << (void*)this << "] is to be deleted!";
  delete this;
}

void FQTermSound::run() {
  if (QFile::exists(soundFile_)) {
#if !defined(AUDIO_OSS) && !defined(AUDIO_ALSA)
    QSound::play(soundFile_);
#else
    this->play();
#endif
  }
}

void FQTermExternalSound::setPlayer(const QString &playername) {
  playerName_ = playername;
}

void FQTermExternalSound::play() {
  if (QFile::exists(soundFile_)) {
    runProgram(playerName_, soundFile_, false);
  }
}

/*******************************************************/
/********** OSS code is from OpenSound System **********/
/*******************************************************/
#ifdef AUDIO_OSS
static int le_int (unsigned char *p, int l) {

  int i, val;
  val = 0;

  for (i = l - 1; i >= 0; i--) {
    val = (val << 8) | p[i];
  }

  return val;
}

static int setupDevice(int audiofd, int channels, int bits, int speed) {

  int tmp;

  tmp = bits;
  if (ioctl(audiofd, SNDCTL_DSP_SETFMT, &tmp) == -1) {
    return 0;
  }

  if (tmp != bits) {
    return 0;
  }

  tmp = channels;
  if (ioctl(audiofd, SNDCTL_DSP_CHANNELS, &tmp) == -1) {
    return 0;
  }

  if (tmp != channels) {
    return 0;
  }

  tmp = speed;
  if (ioctl(audiofd, SNDCTL_DSP_SPEED, &tmp) == -1) {
    return 0;
  }

  return 1;
}

static void dumpData(int audiofd, int soundfd, int filesize) {

  int bsize, l;
  unsigned char buf[1024];
  bsize = sizeof(buf);

  while (filesize) {

    l = bsize;

    if (l > filesize) {
      l = filesize;
    }

    if ((l = read(soundfd, buf, l)) <= 0) {
      return;
    }

    if (write(audiofd, buf, l) == -1) {
      return;
    }

    filesize -= l;
  }
}

static void dumpData24 (int audiofd, int soundfd, int filesize) {

  int bsize, i, l;
  unsigned char buf[1024];
  int outbuf[1024], outlen=0;
  int sample_s32;
  bsize = sizeof(buf);

  filesize -= filesize % 3;

  while(filesize >= 3) {

    l = bsize - bsize % 3;
    if (l > filesize) {
      l = filesize;
    }

    if(l < 3) {
      break;
    }

    if ((l = read(soundfd, buf, l)) <= 0) {
      return;
    }

    outlen=0;

    for(i=0; i<l; i+= 3) {

      unsigned int *u32 = (unsigned int *)&sample_s32; /* Alias */

      /* Read the litle endian input samples */
      *u32 = (buf[i] << 8) | (buf[i+1] << 16) | (buf[i+2]<<24);
      outbuf[outlen++]=sample_s32;

    }

    if (write(audiofd, outbuf, outlen*sizeof(int)) == -1) {
      return;
    }

    filesize -= l;
  }
}

static void playAdpcmWave(int audiofd, int soundfd, unsigned char *hdr, int l) {

  int i, n, dataleft, x;

  adpcm_coeff coeff[32];
  static int AdaptionTable[] = { 230, 230, 230, 230, 307, 409, 512, 614,
    768, 614, 512, 409, 307, 230, 230, 230
  };

  unsigned char buf[4096];

  int channels = 1;
  int bits = 8;
  int speed = 11025;
  int p = 12, max, outp;
  int nBlockAlign, wSamplesPerBlock, wNumCoeff;
  int fmt = -1;
  int nib;

  unsigned char outbuf[64 * 1024];

  while (p < l - 16 && memcmp(&hdr[p], "data", 4) != 0) {

    n = le_int(&hdr[p + 4], 4);
    if (memcmp(&hdr[p], "fmt ", 4) == 0) {

      fmt = le_int(&hdr[p + 8], 2);
      channels = le_int(&hdr[p + 10], 2);
      speed = le_int(&hdr[p + 12], 4);
      nBlockAlign = le_int(&hdr[p + 20], 2);
      bits = AFMT_S16_LE;
      wSamplesPerBlock = le_int(&hdr[p + 26], 2);
      wNumCoeff = le_int(&hdr[p + 28], 2);

      x = p + 30;

      for (i = 0; i < wNumCoeff; i++) {

        coeff[i].coeff1 = (short)le_int(&hdr[x], 2);
        x += 2;
        coeff[i].coeff2 = (short)le_int(&hdr[x], 2);
        x += 2;
      }
    }

    p += n + 8;
  }

  if (p < l - 16 && memcmp(&hdr[p], "data", 4) == 0) {

    dataleft = n = le_int(&hdr[p + 4], 4);
    p += 8;

    if (lseek(soundfd, p, SEEK_SET) == -1) {
      return;
    }

    if (fmt != WAVE_FORMAT_ADPCM) {
      return;
    }

    if (!setupDevice(audiofd, channels, bits, speed)) {
      return;
    }

#define OUT_SAMPLE(s) { \
    if (s>32767)s=32767;else if(s<-32768)s=-32768; \
    outbuf[outp++] = (unsigned char)(s & 0xff); \
    outbuf[outp++] = (unsigned char)((s>>8) & 0xff); \
    n+=2; \
    if (outp>=max){write(audiofd, outbuf, outp);outp=0;}\
    }

#define GETNIBBLE \
    ((nib==0) ? \
        (buf[x + nib++] >> 4) & 0x0f : \
        buf[x++ + --nib] & 0x0f \
    )

    max = 1024;
    outp = 0;

    while (dataleft > nBlockAlign &&
         read(soundfd, buf, nBlockAlign) == nBlockAlign) {

      int predictor[2], delta[2], samp1[2], samp2[2];
      int x = 0;
      dataleft -= nBlockAlign;

      nib = 0;
      n = 0;

      for (i = 0; i < channels; i++) {
        predictor[i] = buf[x];
        x++;
      }

      for (i = 0; i < channels; i++) {
        delta[i] = (short)le_int(&buf[x], 2);
        x += 2;
      }

      for (i = 0; i < channels; i++) {
        samp1[i] = (short)le_int(&buf[x], 2);
        x += 2;
        OUT_SAMPLE(samp1[i]);
      }

      for (i = 0; i < channels; i++) {
        samp2[i] = (short)le_int(&buf[x], 2);
        x += 2;
        OUT_SAMPLE(samp2[i]);
      }

      while (n < (wSamplesPerBlock * 2 * channels)) {

        for (i = 0; i < channels; i++) {

          int pred, new_pred, error_delta, i_delta;

          pred = ((samp1[i] * coeff[predictor[i]].coeff1)
                + (samp2[i] * coeff[predictor[i]].coeff2)) / 256;
          i_delta = error_delta = GETNIBBLE;

          if (i_delta & 0x08) {
            i_delta -= 0x10;    /* Convert to signed */
          }

          new_pred = pred + (delta[i] * i_delta);
          OUT_SAMPLE(new_pred);

          delta[i] = delta[i] * AdaptionTable[error_delta] / 256;
          if (delta[i] < 16) {
            delta[i] = 16;
          }

          samp2[i] = samp1[i];
          samp1[i] = new_pred;
        }
      }
    }
  }

  if (outp > 0) {
    write (audiofd, outbuf, outp /*(outp+3) & ~3 */ );
  }
}

static void playWave(int audiofd, int soundfd, unsigned char *hdr, int l) {

  int filelen;
  int n;
  int channels = 1;
  int bits = 8;
  int file_bits = 8;
  int speed = 11025;
  int p = 12;
  int fmt = -1;

  filelen = le_int(&hdr[4], 4);

  while(p < l - 16 && memcmp(&hdr[p], "data", 4) != 0) {

    n = le_int(&hdr[p + 4], 4);

    if (memcmp(&hdr[p], "fmt ", 4) == 0) {

      fmt = le_int(&hdr[p + 8], 2);
      channels = le_int(&hdr[p + 10], 2);
      speed = le_int(&hdr[p + 12], 4);
      bits = le_int(&hdr[p + 22], 2);

      if (fmt == WAVE_FORMAT_ADPCM) {
        playAdpcmWave(audiofd, soundfd, hdr, l);
        return;
      }

      p += n + 8;
    } else {
      break;
    }
  }

  if (p < l - 16 && memcmp(&hdr[p], "data", 4) == 0) {

    n = le_int(&hdr[p + 4], 4);
    p += 8;

    if (lseek(soundfd, p, SEEK_SET) == -1) {
      return;
    }

    if (fmt != WAVE_FORMAT_PCM) {
      switch (fmt) {
        case WAVE_FORMAT_ALAW:
          bits = AFMT_A_LAW;
          break;
        case WAVE_FORMAT_MULAW:
          bits = AFMT_MU_LAW;
          break;
        case WAVE_FORMAT_IMA_ADPCM:
          bits = AFMT_IMA_ADPCM;
          break;
        default:
          return;
      }
    }

    file_bits = bits;

    if (bits == 32) {
      bits = AFMT_S32_LE;
    } else if (bits == 24) {
      bits = AFMT_S32_LE;
    }

    if (!setupDevice(audiofd, channels, bits, speed)) {
      return;
    }

    if (file_bits != 24) {
      dumpData (audiofd, soundfd, n);
    } else {
      dumpData24 (audiofd, soundfd, n);
    }
  }
}
#endif /* AUDIO_OSS */

void FQTermSystemSound::play() {

#ifdef AUDIO_OSS
  int audio_fd = -1;
  if ((audio_fd = open(FQ_AUDIO_DEV, O_WRONLY, 0)) == -1) {
    FQ_TRACE("sound", 0) << "Error opening device: " << FQ_AUDIO_DEV;
    return;
  }

  int sound_fd = -1;    
  QFile soundFile(soundFile_);

  if (soundFile.open(QFile::ReadOnly)) {
    sound_fd = soundFile.handle();
  } else {
    FQ_TRACE("sound", 0) << "Error opening file: " << soundFile_;
    close(audio_fd);
    return;
  }

  int l;
  unsigned char sound_buf[1024];
 
  if ((l = read(sound_fd, sound_buf, sizeof(sound_buf))) == -1) {
    FQ_TRACE("sound", 0) << "Error reading file into buffer." << soundFile_;
    soundFile.close();
    close(audio_fd);
    return;
  }

  if (l == 0) {
    FQ_TRACE("sound", 0) << "Empty file: " << soundFile_;
    soundFile.close();
    close(audio_fd);
    return;
  }

  lseek(sound_fd, 0, SEEK_SET); /* Start from the beginning */

  if (l > 16 &&
      memcmp(&sound_buf[0], "RIFF", 4) == 0 && memcmp(&sound_buf[8], "WAVE", 4) == 0) {

      playWave(audio_fd, sound_fd, sound_buf, l);
    } else {
      FQ_TRACE("sound", 0) << "We play WAVE files only: " << soundFile_;
    }

  soundFile.close();
  close(audio_fd);

#undef  OUTSAMPLE
#undef  GETNIBBLE
#endif /* AUDIO_OSS */

/* 2008.07.06, ecore.cn@gmail.com, alsa sound for FQTerm ... */
#ifdef AUDIO_ALSA
  FILE *fp;
  FQWFH wfh;
  FQWMH wmh;
  FQWDH wdh;
  int endian, sz, len, count, ret;
  unsigned int val;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_format_t format;
  snd_pcm_uframes_t csz, vsz;
  void *data;

#define WAVE_UINT16(_x)   (endian? ( \
  (((_x) & 0x00FF) << 8) | \
  (((_x) & 0xFF00) >> 8)): \
  (_x))

#define WAVE_UINT32(_x)   (endian? ( \
  (((_x) & 0x000000FF) << 24) | \
  (((_x) & 0x0000FF00) << 8) | \
  (((_x) & 0x00FF0000) >> 8) | \
  (((_x) & 0xFF000000) >> 24)): \
  (_x))

  do {
    if ((fp = fopen(this->soundFile_.toLocal8Bit(), "r")) == NULL) {
      continue;
    }

    if (fread(&wfh, sizeof(FQWFH), 1, fp) != 1) {
      continue;
    }

    switch (wfh.rid) {
      case 0x52494646:
        endian = 1; /* Big-Endian */
        break;
      case 0x46464952:
        endian = 0; /* Small-Endian */
        break;
      default:
        continue;
    }

    if (WAVE_UINT32(wfh.wid) != 0x45564157) { /* "WAVE" */
      continue;
    }

    if (fread(&wmh, sizeof(FQWMH), 1, fp) != 1) {
      continue;
    }

    if (WAVE_UINT32(wmh.fid) != 0x20746d66) { /* "FMT " */
      continue;
    }

    if (WAVE_UINT32(wmh.fsz) != 16 || WAVE_UINT16(wmh.aid) != 1) { /* PCM */
      continue;
    }

    if (fread(&wdh, sizeof(FQWDH), 1, fp) != 1) {
      continue;
    }

    if (WAVE_UINT32(wdh.did) != 0x61746164) { /* "DATA" */
      continue;
    }

    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
      continue;
    }

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    if (snd_pcm_hw_params_any(handle, hwparams) < 0) {
      continue;
    }

    if (snd_pcm_hw_params_set_access(handle, hwparams,
      SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
      continue;
    }

    switch(WAVE_UINT16(wmh.abps)) {
      case 8:
        format = SND_PCM_FORMAT_U8;
        break;
      case 16:
        format = SND_PCM_FORMAT_S16_LE;
        break;
      case 24:
        switch (WAVE_UINT16(wmh.aba) / WAVE_UINT16(wmh.ach)) {
          case 3:
            format = SND_PCM_FORMAT_S24_3LE;
            break;
          case 4:
            format = SND_PCM_FORMAT_S24_LE;
            break;
          default:
            continue;
        }

        break;
      case 32:
        format = SND_PCM_FORMAT_S32_LE;
        break;
      default:
        continue;
    }

    if (snd_pcm_hw_params_set_format(handle, hwparams, format) < 0) {
      continue;
    }

    if (snd_pcm_hw_params_set_channels(handle, hwparams,
      WAVE_UINT16(wmh.ach)) < 0) {
      continue;
    }

    val = WAVE_UINT32(wmh.asr);

    if (snd_pcm_hw_params_set_rate_near(handle, hwparams, &val, NULL) < 0) {
      continue;
    }

    if (snd_pcm_hw_params(handle, hwparams) < 0) {
      continue;
    }

    snd_pcm_hw_params_get_period_size(hwparams, &csz, NULL);
    snd_pcm_hw_params_get_buffer_size(hwparams, &vsz);

    if (csz == vsz) {
      continue;
    }

    snd_pcm_sw_params_current(handle, swparams);

    if (snd_pcm_sw_params_set_avail_min(handle, swparams, csz) < 0) {
      continue;
    }

    if (snd_pcm_sw_params_set_start_threshold(handle, swparams, vsz) < 0) {
      continue;
    }

    if (snd_pcm_sw_params_set_stop_threshold(handle, swparams, vsz) < 0) {
      continue;
    }

    if (snd_pcm_sw_params(handle, swparams) < 0) {
      continue;
    }

    sz = csz * snd_pcm_format_physical_width(format)
      * WAVE_UINT16(wmh.ach) / 8;

    if ((data = malloc(sz)) == NULL) {
      continue;
    }

    len = WAVE_UINT32(wdh.dsz);

    while (len > 0) {
      count = (len < sz)? len: sz;

      ret = fread(data, 1, count, fp);

      len -= ret;

      if (ret != count) {
        continue;
      }

      count = ret * csz / sz;

      ret = snd_pcm_writei(handle, data, count);

      if (ret == -EAGAIN || (ret >= 0 && ret < count)) {
        snd_pcm_wait(handle, 200);
      }
    }

    free(data);

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    fclose(fp);
  }
  while (0);

#undef WAVE_UINT16
#undef WAVE_UINT32
#endif /* AUDIO_ALSA */
}

}  // namespace FQTerm

#include "fqterm_sound.moc"
