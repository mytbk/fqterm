// SPDX-License-Identifier: GPL-2.0-or-later

/*
  PictureFlow - animated image show widget
  http://pictureflow.googlecode.com

  Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "pictureflow.h"

#include <QBasicTimer>
#include <QMouseEvent>
#include <QCache>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include <QWidget>

// uncomment this to enable bilinear filtering for texture mapping
// gives much better rendering, at the cost of memory space
// #define PICTUREFLOW_BILINEAR_FILTER

// for fixed-point arithmetic, we need minimum 32-bit long
// long long (64-bit) might be useful for multiplication and division
typedef long PFreal;
#define PFREAL_SHIFT 10
#define PFREAL_FACTOR (1 << PFREAL_SHIFT)
#define PFREAL_ONE (1 << PFREAL_SHIFT)
#define PFREAL_HALF (PFREAL_ONE >> 1)

inline PFreal fmul(PFreal a, PFreal b)
{
  return ((long long)(a))*((long long)(b)) >> PFREAL_SHIFT;
}

inline PFreal fdiv(PFreal num, PFreal den)
{
  long long p = (long long)(num) << (PFREAL_SHIFT*2);
  long long q = p / (long long)den;
  long long r = q >> PFREAL_SHIFT;

  return r;
}

#define IANGLE_MAX 1024
#define IANGLE_MASK 1023

// warning: regenerate the table if IANGLE_MAX and PFREAL_SHIFT are changed!
static const PFreal sinTable[IANGLE_MAX] = {
     3,      9,     15,     21,     28,     34,     40,     47, 
    53,     59,     65,     72,     78,     84,     90,     97, 
   103,    109,    115,    122,    128,    134,    140,    147, 
   153,    159,    165,    171,    178,    184,    190,    196, 
   202,    209,    215,    221,    227,    233,    239,    245, 
   251,    257,    264,    270,    276,    282,    288,    294, 
   300,    306,    312,    318,    324,    330,    336,    342, 
   347,    353,    359,    365,    371,    377,    383,    388, 
   394,    400,    406,    412,    417,    423,    429,    434, 
   440,    446,    451,    457,    463,    468,    474,    479, 
   485,    491,    496,    501,    507,    512,    518,    523, 
   529,    534,    539,    545,    550,    555,    561,    566, 
   571,    576,    581,    587,    592,    597,    602,    607, 
   612,    617,    622,    627,    632,    637,    642,    647, 
   652,    656,    661,    666,    671,    675,    680,    685, 
   690,    694,    699,    703,    708,    712,    717,    721, 
   726,    730,    735,    739,    743,    748,    752,    756, 
   760,    765,    769,    773,    777,    781,    785,    789, 
   793,    797,    801,    805,    809,    813,    816,    820, 
   824,    828,    831,    835,    839,    842,    846,    849, 
   853,    856,    860,    863,    866,    870,    873,    876, 
   879,    883,    886,    889,    892,    895,    898,    901, 
   904,    907,    910,    913,    916,    918,    921,    924, 
   927,    929,    932,    934,    937,    939,    942,    944, 
   947,    949,    951,    954,    956,    958,    960,    963, 
   965,    967,    969,    971,    973,    975,    977,    978, 
   980,    982,    984,    986,    987,    989,    990,    992, 
   994,    995,    997,    998,    999,   1001,   1002,   1003, 
  1004,   1006,   1007,   1008,   1009,   1010,   1011,   1012, 
  1013,   1014,   1015,   1015,   1016,   1017,   1018,   1018, 
  1019,   1019,   1020,   1020,   1021,   1021,   1022,   1022, 
  1022,   1023,   1023,   1023,   1023,   1023,   1023,   1023, 
  1023,   1023,   1023,   1023,   1023,   1023,   1023,   1022, 
  1022,   1022,   1021,   1021,   1020,   1020,   1019,   1019, 
  1018,   1018,   1017,   1016,   1015,   1015,   1014,   1013, 
  1012,   1011,   1010,   1009,   1008,   1007,   1006,   1004, 
  1003,   1002,   1001,    999,    998,    997,    995,    994, 
   992,    990,    989,    987,    986,    984,    982,    980, 
   978,    977,    975,    973,    971,    969,    967,    965, 
   963,    960,    958,    956,    954,    951,    949,    947, 
   944,    942,    939,    937,    934,    932,    929,    927, 
   924,    921,    918,    916,    913,    910,    907,    904, 
   901,    898,    895,    892,    889,    886,    883,    879, 
   876,    873,    870,    866,    863,    860,    856,    853, 
   849,    846,    842,    839,    835,    831,    828,    824, 
   820,    816,    813,    809,    805,    801,    797,    793, 
   789,    785,    781,    777,    773,    769,    765,    760, 
   756,    752,    748,    743,    739,    735,    730,    726, 
   721,    717,    712,    708,    703,    699,    694,    690, 
   685,    680,    675,    671,    666,    661,    656,    652, 
   647,    642,    637,    632,    627,    622,    617,    612, 
   607,    602,    597,    592,    587,    581,    576,    571, 
   566,    561,    555,    550,    545,    539,    534,    529, 
   523,    518,    512,    507,    501,    496,    491,    485, 
   479,    474,    468,    463,    457,    451,    446,    440, 
   434,    429,    423,    417,    412,    406,    400,    394, 
   388,    383,    377,    371,    365,    359,    353,    347, 
   342,    336,    330,    324,    318,    312,    306,    300, 
   294,    288,    282,    276,    270,    264,    257,    251, 
   245,    239,    233,    227,    221,    215,    209,    202, 
   196,    190,    184,    178,    171,    165,    159,    153, 
   147,    140,    134,    128,    122,    115,    109,    103, 
    97,     90,     84,     78,     72,     65,     59,     53, 
    47,     40,     34,     28,     21,     15,      9,      3, 
    -4,    -10,    -16,    -22,    -29,    -35,    -41,    -48, 
   -54,    -60,    -66,    -73,    -79,    -85,    -91,    -98, 
  -104,   -110,   -116,   -123,   -129,   -135,   -141,   -148, 
  -154,   -160,   -166,   -172,   -179,   -185,   -191,   -197, 
  -203,   -210,   -216,   -222,   -228,   -234,   -240,   -246, 
  -252,   -258,   -265,   -271,   -277,   -283,   -289,   -295, 
  -301,   -307,   -313,   -319,   -325,   -331,   -337,   -343, 
  -348,   -354,   -360,   -366,   -372,   -378,   -384,   -389, 
  -395,   -401,   -407,   -413,   -418,   -424,   -430,   -435, 
  -441,   -447,   -452,   -458,   -464,   -469,   -475,   -480, 
  -486,   -492,   -497,   -502,   -508,   -513,   -519,   -524, 
  -530,   -535,   -540,   -546,   -551,   -556,   -562,   -567, 
  -572,   -577,   -582,   -588,   -593,   -598,   -603,   -608, 
  -613,   -618,   -623,   -628,   -633,   -638,   -643,   -648, 
  -653,   -657,   -662,   -667,   -672,   -676,   -681,   -686, 
  -691,   -695,   -700,   -704,   -709,   -713,   -718,   -722, 
  -727,   -731,   -736,   -740,   -744,   -749,   -753,   -757, 
  -761,   -766,   -770,   -774,   -778,   -782,   -786,   -790, 
  -794,   -798,   -802,   -806,   -810,   -814,   -817,   -821, 
  -825,   -829,   -832,   -836,   -840,   -843,   -847,   -850, 
  -854,   -857,   -861,   -864,   -867,   -871,   -874,   -877, 
  -880,   -884,   -887,   -890,   -893,   -896,   -899,   -902, 
  -905,   -908,   -911,   -914,   -917,   -919,   -922,   -925, 
  -928,   -930,   -933,   -935,   -938,   -940,   -943,   -945, 
  -948,   -950,   -952,   -955,   -957,   -959,   -961,   -964, 
  -966,   -968,   -970,   -972,   -974,   -976,   -978,   -979, 
  -981,   -983,   -985,   -987,   -988,   -990,   -991,   -993, 
  -995,   -996,   -998,   -999,  -1000,  -1002,  -1003,  -1004, 
 -1005,  -1007,  -1008,  -1009,  -1010,  -1011,  -1012,  -1013, 
 -1014,  -1015,  -1016,  -1016,  -1017,  -1018,  -1019,  -1019, 
 -1020,  -1020,  -1021,  -1021,  -1022,  -1022,  -1023,  -1023, 
 -1023,  -1024,  -1024,  -1024,  -1024,  -1024,  -1024,  -1024, 
 -1024,  -1024,  -1024,  -1024,  -1024,  -1024,  -1024,  -1023, 
 -1023,  -1023,  -1022,  -1022,  -1021,  -1021,  -1020,  -1020, 
 -1019,  -1019,  -1018,  -1017,  -1016,  -1016,  -1015,  -1014, 
 -1013,  -1012,  -1011,  -1010,  -1009,  -1008,  -1007,  -1005, 
 -1004,  -1003,  -1002,  -1000,   -999,   -998,   -996,   -995, 
  -993,   -991,   -990,   -988,   -987,   -985,   -983,   -981, 
  -979,   -978,   -976,   -974,   -972,   -970,   -968,   -966, 
  -964,   -961,   -959,   -957,   -955,   -952,   -950,   -948, 
  -945,   -943,   -940,   -938,   -935,   -933,   -930,   -928, 
  -925,   -922,   -919,   -917,   -914,   -911,   -908,   -905, 
  -902,   -899,   -896,   -893,   -890,   -887,   -884,   -880, 
  -877,   -874,   -871,   -867,   -864,   -861,   -857,   -854, 
  -850,   -847,   -843,   -840,   -836,   -832,   -829,   -825, 
  -821,   -817,   -814,   -810,   -806,   -802,   -798,   -794, 
  -790,   -786,   -782,   -778,   -774,   -770,   -766,   -761, 
  -757,   -753,   -749,   -744,   -740,   -736,   -731,   -727, 
  -722,   -718,   -713,   -709,   -704,   -700,   -695,   -691, 
  -686,   -681,   -676,   -672,   -667,   -662,   -657,   -653, 
  -648,   -643,   -638,   -633,   -628,   -623,   -618,   -613, 
  -608,   -603,   -598,   -593,   -588,   -582,   -577,   -572, 
  -567,   -562,   -556,   -551,   -546,   -540,   -535,   -530, 
  -524,   -519,   -513,   -508,   -502,   -497,   -492,   -486, 
  -480,   -475,   -469,   -464,   -458,   -452,   -447,   -441, 
  -435,   -430,   -424,   -418,   -413,   -407,   -401,   -395, 
  -389,   -384,   -378,   -372,   -366,   -360,   -354,   -348, 
  -343,   -337,   -331,   -325,   -319,   -313,   -307,   -301, 
  -295,   -289,   -283,   -277,   -271,   -265,   -258,   -252, 
  -246,   -240,   -234,   -228,   -222,   -216,   -210,   -203, 
  -197,   -191,   -185,   -179,   -172,   -166,   -160,   -154, 
  -148,   -141,   -135,   -129,   -123,   -116,   -110,   -104, 
   -98,    -91,    -85,    -79,    -73,    -66,    -60,    -54, 
   -48,    -41,    -35,    -29,    -22,    -16,    -10,     -4 
};

// this is the program the generate the above table
#if 0
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PFREAL_ONE 1024
#define IANGLE_MAX 1024

int main(int, char**)
{
  FILE*f = fopen("table.c","wt");
  fprintf(f,"PFreal sinTable[] = {\n");
  for(int i = 0; i < 128; ++i)
  {
    for(int j = 0; j < 8; ++j)
    {
      int iang = j+i*8;
      double ii = (double)iang + 0.5;
      double angle = ii * 2 * M_PI / IANGLE_MAX;
      double sinAngle = sin(angle);
      fprintf(f,"%6d, ", (int)(floor(PFREAL_ONE*sinAngle)));
    }
    fprintf(f,"\n");
  }
  fprintf(f,"};\n");
  fclose(f);

  return 0;
}
#endif

inline PFreal fsin(int iangle)
{
  while(iangle < 0)
    iangle += IANGLE_MAX;
  return sinTable[iangle & IANGLE_MASK];
}  

inline PFreal fcos(int iangle)
{
  // quarter phase shift
  return fsin(iangle + (IANGLE_MAX >> 2));
}

namespace FQTerm {

struct SlideInfo
{
  int slideIndex;
  int angle;
  PFreal cx;
  PFreal cy;
};

class PictureFlowPrivate
{
public:
  PictureFlowPrivate(PictureFlow* widget);

  int slideCount() const;
  void setSlideCount(int count);

  QSize slideSize() const;
  void setSlideSize(QSize size);

  int zoomFactor() const;
  void setZoomFactor(int z);

  QImage slide(int index) const;
  void setSlide(int index, const QImage& image);

  int currentSlide() const;
  void setCurrentSlide(int index);

  void showPrevious();
  void showNext();
  void showSlide(int index);

  void resize(int w, int h);

  void render();
  void startAnimation();
  void updateAnimation();

  QImage buffer;
  QBasicTimer animateTimer;
  QVector<QRect> slidesRect;

private:
  PictureFlow* widget;

  int slideWidth;
  int slideHeight;
  int zoom;

  QVector<QImage> slideImages;
  int centerIndex;

  SlideInfo centerSlide;
  QVector<SlideInfo> leftSlides;
  QVector<SlideInfo> rightSlides;

  QVector<PFreal> rays;
  int itilt;
  int spacing;
  PFreal offsetX;
  PFreal offsetY;

  QImage blankSurface;
  QCache<int, QImage> surfaceCache;
  QTimer triggerTimer;

  int slideFrame;
  int step;
  int target;
  int fade;

  void recalc(int w, int h);
  QRect renderSlide(const SlideInfo &slide, int alpha=256, int col1=-1, int col=-1);
  QImage* surface(int slideIndex);
  void triggerRender();
  void resetSlides();
};

PictureFlowPrivate::PictureFlowPrivate(PictureFlow* w)
{
  widget = w;

  slideWidth = 200;
  slideHeight = 200;
  zoom = 100;

  centerIndex = 0;

  slideFrame = 0;
  step = 0;
  target = 0;
  fade = 256;

  triggerTimer.setSingleShot(true);
  triggerTimer.setInterval(0);
  QObject::connect(&triggerTimer, SIGNAL(timeout()), widget, SLOT(render()));
  
  recalc(200, 200);
  resetSlides();
}

int PictureFlowPrivate::slideCount() const
{
  return slideImages.count();
}

void PictureFlowPrivate::setSlideCount(int count)
{
  slideImages.resize(count);
  surfaceCache.clear();
  resetSlides();
  triggerRender();
}

QSize PictureFlowPrivate::slideSize() const
{
  return QSize(slideWidth, slideHeight);
}

void PictureFlowPrivate::setSlideSize(QSize size)
{
  slideWidth = size.width();
  slideHeight = size.height();
  recalc(buffer.width(), buffer.height());
  triggerRender();
}

int PictureFlowPrivate::zoomFactor() const
{
  return zoom;
}

void PictureFlowPrivate::setZoomFactor(int z)
{
  if(z <= 0)
    return;

  zoom = z;
  recalc(buffer.width(), buffer.height());
  triggerRender();
}

QImage PictureFlowPrivate::slide(int index) const
{
  return slideImages[index];
}

void PictureFlowPrivate::setSlide(int index, const QImage& image)
{
  if((index >= 0) && (index < slideImages.count()))
  {
    slideImages[index] = image;
    surfaceCache.remove(index);
    triggerRender();
  }  
}

int PictureFlowPrivate::currentSlide() const
{
  return centerIndex;
} 

void PictureFlowPrivate::setCurrentSlide(int index)
{
  step = 0;
  centerIndex = qBound(index, 0, slideImages.count()-1);
  target = centerIndex;
  slideFrame = index << 16;
  resetSlides();
  triggerRender();
}

void PictureFlowPrivate::showPrevious()
{
  if(step >= 0)
  {
    if(centerIndex > 0)
    {
      target = centerIndex - 1;
      startAnimation();
    }
  }
  else
  {
    target = qMax(0, centerIndex - 2);
  }
}

void PictureFlowPrivate::showNext()
{
  if(step <= 0)
  {
    if(centerIndex < slideImages.count()-1)
    {
      target = centerIndex + 1;
      startAnimation();
    }
  }
  else
  {
    target = qMin(centerIndex + 2, slideImages.count()-1);
  }
}

void PictureFlowPrivate::showSlide(int index)
{
  index = qMax(index, 0);
  index = qMin(slideImages.count()-1, index);

  target = index;
  startAnimation();
}

void PictureFlowPrivate::resize(int w, int h)
{
  recalc(w, h);
  resetSlides();
  triggerRender();
}


// adjust slides so that they are in "steady state" position
void PictureFlowPrivate::resetSlides()
{
  centerSlide.angle = 0;
  centerSlide.cx = 0;
  centerSlide.cy = 0;
  centerSlide.slideIndex = centerIndex;

  leftSlides.clear();
  leftSlides.resize(6);
  for(int i = 0; i < leftSlides.count(); ++i)
  {
    SlideInfo& si = leftSlides[i];
    si.angle = itilt;
    si.cx = -(offsetX + spacing*i*PFREAL_ONE);
    si.cy = offsetY;
    si.slideIndex = centerIndex-1-i;
  }

  rightSlides.clear();
  rightSlides.resize(6);
  for(int i = 0; i < rightSlides.count(); ++i)
  {
    SlideInfo& si = rightSlides[i];
    si.angle = -itilt;
    si.cx = offsetX + spacing*i*PFREAL_ONE;
    si.cy = offsetY;
    si.slideIndex = centerIndex+1+i;
  }
}

#define BILINEAR_STRETCH_HOR 4
#define BILINEAR_STRETCH_VER 4

static QImage scaleImage (QImage image, int w, int h) {
	// I don't have to scale it
	if (w == image.width() && h == image.height())
		return(image);

	QImage output(w, h, QImage::Format_ARGB32_Premultiplied);
	output.fill(0);

	double scaleHeight = w / (double) image.height();
	double scaleWidth = h / (double) image.width();

	double s = qMin(scaleWidth, scaleHeight);

	int sw = (int) qRound(s * image.width());
	int sh = (int) qRound(s * image.height());

	// Scale Image
	QImage imgScaled = image.scaled(sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	int _x = (w - sw) / 2;
	int _y = (h - sh);

	for (int x = 0; x < imgScaled.width(); ++x) {
		for (int y = 0; y < imgScaled.height(); ++y) {
			output.setPixel(x + _x, y + _y, imgScaled.pixel(x, y));
		}
	}

	return(output);
}

static QImage prepareSurface(QImage img, int w, int h)
{
  img = scaleImage(img, w, h);

  // slightly larger, to accomodate for the reflection
  int hs = h * 2;
  int hofs = h / 3;

  // offscreen buffer: black is sweet
  QImage result(hs, w, QImage::Format_RGB32);  
  result.fill(0);

  // transpose the image, this is to speed-up the rendering
  // because we process one column at a time
  // (and much better and faster to work row-wise, i.e in one scanline)
  for(int x = 0; x < w; x++)
    for(int y = 0; y < h; y++)
      result.setPixel(hofs + y, x, img.pixel(x, y));

  // create the reflection
  int ht = hs - h - hofs;
  int hte = ht;
  for(int x = 0; x < w; x++)
    for(int y = 0; y < ht; y++)
    {
      QRgb color = img.pixel(x, img.height()-y-1);
      int a = qAlpha(color);
      int r = qRed(color)   * a / 256 * (hte - y) / hte * 3/5;
      int g = qGreen(color) * a / 256 * (hte - y) / hte * 3/5;
      int b = qBlue(color)  * a / 256 * (hte - y) / hte * 3/5;
      result.setPixel(h+hofs+y, x, qRgb(r, g, b));
    }

#ifdef PICTUREFLOW_BILINEAR_FILTER
  int hh = BILINEAR_STRETCH_VER*hs;
  int ww = BILINEAR_STRETCH_HOR*w;
  result = scaleImage(result, hh, ww);
#endif

  return result;
}


// get transformed image for specified slide
// if it does not exist, create it and place it in the cache
QImage* PictureFlowPrivate::surface(int slideIndex)
{
  if(slideIndex < 0)
    return 0;
  if(slideIndex >= slideImages.count())
    return 0;

  if(surfaceCache.contains(slideIndex))
    return surfaceCache[slideIndex];

  QImage img = widget->slide(slideIndex);
  if(img.isNull())
  {
    if(blankSurface.isNull())
    {
      blankSurface = QImage(slideWidth, slideHeight, QImage::Format_RGB32);

      QPainter painter(&blankSurface);
      QPoint p1(slideWidth*4/10, 0);
      QPoint p2(slideWidth*6/10, slideHeight);
      QLinearGradient linearGrad(p1, p2);
      linearGrad.setColorAt(0, Qt::black);
      linearGrad.setColorAt(1, Qt::white); 
      painter.setBrush(linearGrad);
      painter.fillRect(0, 0, slideWidth, slideHeight, QBrush(linearGrad));

      painter.setPen(QPen(QColor(64,64,64), 4));
      painter.setBrush(QBrush());
      painter.drawRect(2, 2, slideWidth-3, slideHeight-3);
      painter.end();
      blankSurface = prepareSurface(blankSurface, slideWidth, slideHeight);
    }
    return &blankSurface;
  }

  surfaceCache.insert(slideIndex, new QImage(prepareSurface(img, slideWidth, slideHeight)));
  return surfaceCache[slideIndex];
}


// Schedules rendering the slides. Call this function to avoid immediate 
// render and thus cause less flicker.
void PictureFlowPrivate::triggerRender()
{
  triggerTimer.start();
}

// Render the slides. Updates only the offscreen buffer.
void PictureFlowPrivate::render()
{
  buffer.fill(Qt::black);

  int nleft = leftSlides.count();
  int nright = rightSlides.count();

  QRect r = renderSlide(centerSlide);
  int c1 = r.left();
  int c2 = r.right();

  if(step == 0)
  {
    slidesRect.clear();	
	slidesRect.resize(11);

    // no animation, boring plain rendering
    for(int index = 0; index < nleft-1; index++)
    {
      int alpha = (index < nleft-2) ? 256 : 128;
      QRect rs = renderSlide(leftSlides[index], alpha, 0, c1-1);
      slidesRect[index] = rs;
      if(!rs.isEmpty())
        c1 = rs.left();
    }  
    slidesRect[5] = r;
    for(int index = 0; index < nright-1; index++)
    {
      int alpha = (index < nright-2) ? 256 : 128;
      QRect rs = renderSlide(rightSlides[index], alpha, c2+1, buffer.width());
      slidesRect[6 + index] = rs;
      if(!rs.isEmpty())
        c2 = rs.right();
    }  
  }
  else
  {
    // the first and last slide must fade in/fade out
    for(int index = 0; index < nleft; index++)
    {
      int alpha = 256;
      if(index == nleft-1)
        alpha = (step > 0) ? 0 : 128-fade/2;
      if(index == nleft-2)
        alpha = (step > 0) ? 128-fade/2 : 256-fade/2;
      if(index == nleft-3)
        alpha = (step > 0) ? 256-fade/2 : 256;
      QRect rs = renderSlide(leftSlides[index], alpha, 0, c1-1);
      if(!rs.isEmpty())
        c1 = rs.left();
    }  
    for(int index = 0; index < nright; index++)
    {
      int alpha = (index < nright-2) ? 256 : 128;
      if(index == nright-1)
        alpha = (step > 0) ? fade/2 : 0;
      if(index == nright-2)
        alpha = (step > 0) ? 128+fade/2 : fade/2;
      if(index == nright-3)
        alpha = (step > 0) ? 256 : 128+fade/2;
      QRect rs = renderSlide(rightSlides[index], alpha, c2+1, buffer.width());
      if(!rs.isEmpty())
        c2 = rs.right();
    }  
  }
}

// Renders a slide to offscreen buffer. Returns a rect of the rendered area.
// alpha=256 means normal, alpha=0 is fully black, alpha=128 half transparent
// col1 and col2 limit the column for rendering.
QRect PictureFlowPrivate::renderSlide(const SlideInfo &slide, int alpha, 
int col1, int col2)
{
  QImage* src = surface(slide.slideIndex);
  if(!src)
    return QRect();

  QRect rect(0, 0, 0, 0);  
  
#ifdef PICTUREFLOW_BILINEAR_FILTER
  int sw = src->height() / BILINEAR_STRETCH_HOR;
  int sh = src->width() / BILINEAR_STRETCH_VER;
#else
  int sw = src->height();
  int sh = src->width();
#endif
  int h = buffer.height();
  int w = buffer.width();

  if(col1 > col2)
  {
    int c = col2;
    col2 = col1;
    col1 = c;
  }

  col1 = (col1 >= 0) ? col1 : 0;
  col2 = (col2 >= 0) ? col2 : w-1;
  col1 = qMin(col1, w-1);
  col2 = qMin(col2, w-1);

  int distance = h * 100 / zoom;
  PFreal sdx = fcos(slide.angle);
  PFreal sdy = fsin(slide.angle);
  PFreal xs = slide.cx - slideWidth * sdx/2;
  PFreal ys = slide.cy - slideWidth * sdy/2;
  PFreal dist = distance * PFREAL_ONE;

  int xi = qMax((PFreal)0, ((w*PFREAL_ONE/2) + fdiv(xs*h, dist+ys)) >> PFREAL_SHIFT);
  if(xi >= w)
    return rect;

  bool flag = false;
  rect.setLeft(xi);

  for(int x = qMax(xi, col1); x <= col2; x++)
  {
    PFreal hity = 0;
    PFreal fk = rays[x];
    if(sdy)
    {
      fk = fk - fdiv(sdx,sdy);
      hity = -fdiv((rays[x]*distance - slide.cx + slide.cy*sdx/sdy), fk);
    }

    dist = distance*PFREAL_ONE + hity;
    if(dist < 0)
      continue;

    PFreal hitx = fmul(dist, rays[x]);
    PFreal hitdist = fdiv(hitx - slide.cx, sdx);

#ifdef PICTUREFLOW_BILINEAR_FILTER
    int column = sw*BILINEAR_STRETCH_HOR/2 + (hitdist*BILINEAR_STRETCH_HOR >> PFREAL_SHIFT);
    if(column >= sw*BILINEAR_STRETCH_HOR)
      break;
#else
    int column = sw/2 + (hitdist >> PFREAL_SHIFT);
    if(column >= sw)
      break;
#endif
    if(column < 0)
      continue;

    rect.setRight(x);  
    if(!flag)
      rect.setLeft(x);
    flag = true;  

    int y1 = h/2;
    int y2 = y1+ 1;
    QRgb* pixel1 = (QRgb*)(buffer.scanLine(y1)) + x;
    QRgb* pixel2 = (QRgb*)(buffer.scanLine(y2)) + x;
    QRgb pixelstep = pixel2 - pixel1;

#ifdef PICTUREFLOW_BILINEAR_FILTER
    int center = (sh*BILINEAR_STRETCH_VER/2);
    int dy = dist*BILINEAR_STRETCH_VER / h;
#else
    int center = (sh/2);
    int dy = dist / h;
#endif
    int p1 = center*PFREAL_ONE - dy/2;
    int p2 = center*PFREAL_ONE + dy/2;

    const QRgb *ptr = (const QRgb*)(src->scanLine(column));
    if(alpha == 256)
      while((y1 >= 0) && (y2 < h) && (p1 >= 0))
      {
        *pixel1 = ptr[p1 >> PFREAL_SHIFT];
        *pixel2 = ptr[p2 >> PFREAL_SHIFT];
        p1 -= dy;
        p2 += dy;
        y1--;
        y2++;
        pixel1 -= pixelstep;
        pixel2 += pixelstep;
      }  
    else
      while((y1 >= 0) && (y2 < h) && (p1 >= 0))
      {
        QRgb c1 = ptr[p1 >> PFREAL_SHIFT];
        QRgb c2 = ptr[p2 >> PFREAL_SHIFT];

        int r1 = qRed(c1) * alpha/256;
        int g1 = qGreen(c1) * alpha/256;
        int b1 = qBlue(c1) * alpha/256;
        int r2 = qRed(c2) * alpha/256;
        int g2 = qGreen(c2) * alpha/256;
        int b2 = qBlue(c2) * alpha/256;

        *pixel1 = qRgb(r1, g1, b1);
        *pixel2 = qRgb(r2, g2, b2);
        p1 -= dy;
        p2 += dy;
        y1--;
        y2++;
        pixel1 -= pixelstep;
        pixel2 += pixelstep;
     }  
   }  

   rect.setTop(0);
   rect.setBottom(h-1);
   return rect;
}

// Updates look-up table and other stuff necessary for the rendering.
// Call this when the viewport size or slide dimension is changed.
void PictureFlowPrivate::recalc(int ww, int wh)
{
  int w = (ww+1)/2;
  int h = (wh+1)/2;
  buffer = QImage(ww, wh, QImage::Format_RGB32);
  buffer.fill(0);

  rays.resize(w*2);

  for(int i = 0; i < w; ++i)
  {
    PFreal gg = (PFREAL_HALF + i * PFREAL_ONE) / (2*h);
    rays[w-i-1] = -gg;
    rays[w+i] = gg;
  }


  itilt = 70 * IANGLE_MAX / 360;  // approx. 70 degrees tilted

  offsetX = slideWidth/2 * (PFREAL_ONE-fcos(itilt));
  offsetY = slideWidth/2 * fsin(itilt);
  offsetX += slideWidth * PFREAL_ONE;
  offsetY += slideWidth * PFREAL_ONE / 4;
  spacing = 40;

  surfaceCache.clear();
  blankSurface = QImage();
}

void PictureFlowPrivate::startAnimation()
{
#if 1
  if(!animateTimer.isActive())
  {
    step = (target < centerSlide.slideIndex) ? -1 : 1;
    animateTimer.start(30, widget);
  }
#else
  if (animateTimer.isActive())
    animateTimer.stop();

  step = (target < centerSlide.slideIndex) ? -1 : 1;
  animateTimer.start(30, widget);
#endif
}

// Updates the animation effect. Call this periodically from a timer.
void PictureFlowPrivate::updateAnimation()
{
  if(!animateTimer.isActive())
    return;
  if(step == 0)
    return;

  int speed = 16384;

  // deaccelerate when approaching the target
  if(true)
  {
    const int max = 2 * 65536;

    int fi = slideFrame;
    fi -= (target << 16);  
    if(fi < 0)
      fi = -fi;
    fi = qMin(fi, max);

    int ia = IANGLE_MAX * (fi-max/2) / (max*2);
    speed = 512 + 16384 * (PFREAL_ONE+fsin(ia))/PFREAL_ONE;
  }

  slideFrame += speed*step;

  int index = slideFrame >> 16;
  int pos = slideFrame & 0xffff;
  int neg = 65536 - pos;
  int tick = (step < 0) ? neg : pos;
  PFreal ftick = (tick * PFREAL_ONE) >> 16;  

  // the leftmost and rightmost slide must fade away
  fade = pos / 256;

  if(step < 0)
    index++;
  if(centerIndex != index)
  {
    centerIndex = index;
    slideFrame = index << 16;
    centerSlide.slideIndex = centerIndex;
    for(int i = 0; i < leftSlides.count(); ++i)
      leftSlides[i].slideIndex = centerIndex-1-i;
    for(int i = 0; i < rightSlides.count(); ++i)
      rightSlides[i].slideIndex = centerIndex+1+i;
  }

  centerSlide.angle = (step * tick * itilt) >> 16;
  centerSlide.cx = -step * fmul(offsetX, ftick);
  centerSlide.cy = fmul(offsetY, ftick);

  if(centerIndex == target)
  {
    resetSlides();
    animateTimer.stop();
    triggerRender();
    step = 0;
    fade = 256;
    return;
  }  

  for(int i = 0; i < leftSlides.count(); ++i)
  {
    SlideInfo& si = leftSlides[i];
    si.angle = itilt;
    si.cx = -(offsetX + spacing*i*PFREAL_ONE + step*spacing*ftick);
    si.cy = offsetY;
  }

  for(int i = 0; i < rightSlides.count(); ++i)
  {
    SlideInfo& si = rightSlides[i];
    si.angle = -itilt;
    si.cx = offsetX + spacing*i*PFREAL_ONE - step*spacing*ftick;
    si.cy = offsetY;
  }

  if(step > 0)
  {
    PFreal ftick = (neg * PFREAL_ONE) >> 16;
    rightSlides[0].angle = -(neg * itilt) >> 16;
    rightSlides[0].cx = fmul(offsetX, ftick);
    rightSlides[0].cy = fmul(offsetY, ftick);
  }
  else
  {
    PFreal ftick = (pos * PFREAL_ONE) >> 16;
    leftSlides[0].angle = (pos * itilt) >> 16;
    leftSlides[0].cx = -fmul(offsetX, ftick);
    leftSlides[0].cy = fmul(offsetY, ftick);
  } 

  // must change direction ?
  if(target < index) if(step > 0)
    step = -1;
  if(target > index) if(step < 0)
    step = 1;

  triggerRender();
}

// -----------------------------------------

PictureFlow::PictureFlow(QWidget* parent): QWidget(parent)
{
  d = new PictureFlowPrivate(this);

  setAttribute(Qt::WA_StaticContents, true);
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  setAttribute(Qt::WA_NoSystemBackground, true);
}

PictureFlow::~PictureFlow()
{
  delete d;
}  

int PictureFlow::slideCount() const
{
  return d->slideCount();
}

void PictureFlow::setSlideCount(int count)
{
  d->setSlideCount(count);
}

QSize PictureFlow::slideSize() const
{
  return d->slideSize();
}

void PictureFlow::setSlideSize(QSize size)
{
  d->setSlideSize(size);
}

int PictureFlow::zoomFactor() const
{
  return d->zoomFactor();
}

void PictureFlow::setZoomFactor(int z)
{
  d->setZoomFactor(z);
}

QImage PictureFlow::slide(int index) const
{
  return d->slide(index);
}

void PictureFlow::setSlide(int index, const QImage& image)
{
  d->setSlide(index, image);
}

void PictureFlow::setSlide(int index, const QPixmap& pixmap)
{
  d->setSlide(index, pixmap.toImage());
}

int PictureFlow::currentSlide() const
{
  return d->currentSlide();
}

void PictureFlow::setCurrentSlide(int index)
{
  d->setCurrentSlide(index);
}

void PictureFlow::clear()
{
  d->setSlideCount(0);
}

void PictureFlow::render()
{
  d->render();
  update();
}

void PictureFlow::showPrevious()
{
  d->showPrevious();
}

void PictureFlow::showNext()
{
  d->showNext();
}

void PictureFlow::showSlide(int index)
{
  d->showSlide(index);
}

void PictureFlow::showSlideAt (int x, int y) {
	if (d->slidesRect[5].contains(x, y))
		return;

	if (x < d->slidesRect[5].x()) {
		for (int i=0; i < 5; ++i) {
			if (d->slidesRect[i].contains(x, y)) {
				showSlide(d->currentSlide() - (i + 1));
				return;
			}
		}
	} else {
		for (int i=6; i < 11; ++i) {
			if (d->slidesRect[i].contains(x, y)) {
				showSlide(d->currentSlide() + (i - 5));
				return;
			}
		}
	}
}

void PictureFlow::keyPressEvent(QKeyEvent* event)
{

  switch (event->key()) {
    case Qt::Key_Left:
      if(event->modifiers() == Qt::AltModifier) {
        showSlide(currentSlide() - 5);
      } else {
        showPrevious();
      }
      event->accept();
      return;
    case Qt::Key_Right:
      if(event->modifiers() == Qt::AltModifier) {
        showSlide(currentSlide() + 5);
      } else {
        showNext();
      }
      event->accept();
      return;
    case Qt::Key_Up:
      showSlide(currentSlide() + 10);
      event->accept();
      return;
    case Qt::Key_Down:
      showSlide(currentSlide() - 10);
      event->accept();
      return;
    default:
      break;
  }

  event->ignore();
}

void PictureFlow::mouseReleaseEvent (QMouseEvent *event) {
	showSlideAt(event->x(), event->y());
}

void PictureFlow::mouseMoveEvent (QMouseEvent *event) {
	showSlideAt(event->x(), event->y());
}

void PictureFlow::wheelEvent (QWheelEvent *event) {
	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;

	d->showSlide(d->currentSlide() + numSteps);
	event->accept();
}

void PictureFlow::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.drawImage(QPoint(0,0), d->buffer);
}

void PictureFlow::resizeEvent(QResizeEvent* event)
{
  d->resize(width(), height());
  QWidget::resizeEvent(event);
}

void PictureFlow::timerEvent(QTimerEvent* event)
{
  if(event->timerId() == d->animateTimer.timerId())
    d->updateAnimation();
  else
    QWidget::timerEvent(event);
}

} // namespace FQTerm

#include "pictureflow.moc"
