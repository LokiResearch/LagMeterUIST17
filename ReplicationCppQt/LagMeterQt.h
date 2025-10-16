/* -*- mode: c++ -*-
 *
 * LagMeterQt.h --
 *
 * Authors: Nicolas Roussel
 *
 * Copyright © Inria
 *
 */

#ifndef LagMeterQt_h
#define LagMeterQt_h

#include <QWidget>

#include "TurboHID.h"

#if NO_LIBPOINTING
#include "utils/ConfigDict.h"
#else
#include <pointing/utils/ConfigDict.h>
#include <pointing/input/PointingDevice.h>
#endif
using namespace pointing ;

#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

class LagMeterQt : public QWidget {

  Q_OBJECT ;

  std::ostream *logstream ;

  TurboHID *turboHID ;  
  static void turboHIDCallback(void *context, TimeStamp::inttime timestamp, int nbBytesReceived, unsigned char* data) ;
  
#ifndef NO_LIBPOINTING
  PointingDevice *pdev ;
  static void pointingCallback(void *context, TimeStamp::inttime timestamp,
			       int input_dx, int input_dy, int buttons) ;
#endif

#if __APPLE__
  CFMachPortRef cgtap ;
  CFRunLoopSourceRef cgtapsource ;
  static CGEventRef cgCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) ;

  static void screenCallback(CGRectCount count, const CGRect *rectArray, void *refcon) ;
#endif

  void setBackground(QColor color) ;
  
  void mousePressEvent(QMouseEvent *event) ;
  void mouseReleaseEvent(QMouseEvent *event) ;

  bool immediate_refresh ;
  void paintEvent(QPaintEvent *event) ;
  
public:

  static ConfigDict cfg ;

  LagMeterQt(QApplication *app, QWidget *parent, QStringList args) ;
  ~LagMeterQt(void) ;

} ;

#endif
