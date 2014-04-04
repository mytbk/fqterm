
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

#ifndef PICTUREFLOW_H
#define PICTUREFLOW_H

#include <QWidget>

namespace FQTerm {

class PictureFlowPrivate;

/*!
  Class PictureFlow implements an image show widget with animation effect 
  like Apple's CoverFlow (in iTunes and iPod). Images are arranged in form 
  of slides, one main slide is shown at the center with few slides on 
  the left and right sides of the center slide. When the next or previous 
  slide is brought to the front, the whole slides flow to the right or 
  the right with smooth animation effect; until the new slide is finally 
  placed at the center.

 */ 
class PictureFlow : public QWidget
{
Q_OBJECT

  Q_PROPERTY(int slideCount READ slideCount WRITE setSlideCount)
  Q_PROPERTY(int currentSlide READ currentSlide WRITE setCurrentSlide)
  Q_PROPERTY(QSize slideSize READ slideSize WRITE setSlideSize)
  Q_PROPERTY(int zoomFactor READ zoomFactor WRITE setZoomFactor)

public:
  /*!
    Creates a new PictureFlow widget.
  */  
  PictureFlow(QWidget* parent = 0);

  /*!
    Destroys the widget.
  */
  ~PictureFlow();

  /*!
    Returns the total number of slides.
  */
  int slideCount() const;

  /*!
    Sets the total number of slides.
  */
  void setSlideCount(int count);

  /*!
    Returns the dimension of each slide (in pixels).
  */  
  QSize slideSize() const;

  /*!
    Sets the dimension of each slide (in pixels).
  */  
  void setSlideSize(QSize size);

  /*!
    Sets the zoom factor (in percent).
  */ 
  void setZoomFactor(int zoom);

  /*!
    Returns the zoom factor (in percent).
  */
  int zoomFactor() const;

  /*!
    Returns QImage of specified slide.
    This function will be called only whenever necessary, e.g. the 100th slide
    will not be retrived when only the first few slides are visible.
  */  
  virtual QImage slide(int index) const;

  /*!
    Sets an image for specified slide. If the slide already exists,
    it will be replaced.
  */  
  virtual void setSlide(int index, const QImage& image);

  /*!
    Sets a pixmap for specified slide. If the slide already exists,
    it will be replaced.
  */  
  virtual void setSlide(int index, const QPixmap& pixmap);

  /*!
    Returns the index of slide currently shown in the middle of the viewport.
  */  
  int currentSlide() const;

signals:
	void slideSelected (int index);

public slots:

  /*!
    Sets slide to be shown in the middle of the viewport. No animation 
    effect will be produced, unlike using showSlide.
  */  
  void setCurrentSlide(int index);

  /*!
    Clears images of all slides.
  */
  void clear();

  /*!
    Rerender the widget. Normally this function will be automatically invoked
    whenever necessary, e.g. during the transition animation.
  */
  void render();

  /*!
    Shows previous slide using animation effect.
  */
  void showPrevious();

  /*!
    Shows next slide using animation effect.
  */
  void showNext();

  /*!
    Go to specified slide using animation effect.
  */
  void showSlide(int index);


  /*!
    Go to slide at specified position using animation effect.
  */
  void showSlideAt(int x, int y);
	
protected:
  void mouseReleaseEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void wheelEvent (QWheelEvent *event);
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);
  void timerEvent(QTimerEvent *event);

private:
  PictureFlowPrivate* d;
};

} // namespace FQTerm

#endif // PICTUREFLOW_H
