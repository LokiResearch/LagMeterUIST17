/*
 *
 * LagMeterQt.cpp --
 *
 * Authors: Nicolas Roussel, Géry Casiez
 *
 * Copyright © Inria
 *
 */

#include "LagMeterQt.h"

#include <QApplication>
#include <QtGui>

#if NO_LIBPOINTING
#include "utils/TimeStamp.h"
#else
#include <pointing/utils/TimeStamp.h>
#include <pointing/output/DisplayDevice.h>
#endif
using namespace pointing ;

#if __APPLE__
#include <sys/time.h>
#include <mach/mach_time.h>
#endif

#include <fstream>

ConfigDict LagMeterQt::cfg ;

LagMeterQt::LagMeterQt(QApplication */*app*/, QWidget *parent, QStringList args) : QWidget(parent) {
  int nbargs = args.size() ;
  for (int i=1; i<nbargs; ++i) {
    const QString &arg = args.at(i) ;
    if (arg=="-c") {
      if (i+1>=nbargs) continue ;
      LagMeterQt::cfg.loadFrom(args.at(++i).toStdString()) ;
    } else if (arg=="-d") {
      if (i+1>=nbargs) continue ;
      LagMeterQt::cfg.set("libpointing-input", args.at(++i).toStdString()) ;
    } else if (arg=="-o") {
      if (i+1>=nbargs) continue ;
      LagMeterQt::cfg.set("meter-logfile", args.at(++i).toStdString()) ;      
    }
  }

  LagMeterQt::cfg.setdefault("libpointing-input", "any:?debugLevel=1") ;
  LagMeterQt::cfg.setdefault("meter-logfile", "") ;
  LagMeterQt::cfg.setdefault("meter-immediate-refresh", true) ;  

  // ----------------------------------------------------------------

  immediate_refresh = LagMeterQt::cfg.get<bool>("meter-immediate-refresh") ;
    
  std::string logfile = LagMeterQt::cfg.get<std::string>("meter-logfile") ;
  if (logfile.empty())
    logstream = &std::cout ;
  else
    logstream = new std::ofstream(logfile.c_str()) ;

  // ----------------------------------------------------------------

  // TurboHID observer
  try {
    turboHID = new TurboHID ;
    turboHID->setTurboHIDCallback(turboHIDCallback, this) ;
    turboHID->startThread() ;
  } catch (std::exception &e) {
    turboHID = 0 ;
  }
  
  // ----------------------------------------------------------------
  
  // HID traffic observer
#ifndef NO_LIBPOINTING
  DisplayDevice *display = DisplayDevice::create("any:") ;
  LagMeterQt::cfg.set("libpointing-output", display->getURI(true).asString()) ;
  delete display ;

  pdev = PointingDevice::create(LagMeterQt::cfg.get<std::string>("libpointing-input")) ;
  for (TimeStamp reftime, now;
       !pdev->isActive() && now-reftime<15*TimeStamp::one_second; 
       now.refresh())
    PointingDevice::idle(500) ;
  pdev->setPointingCallback(pointingCallback, this) ;
  LagMeterQt::cfg.set("libpointing-input", pdev->getURI(true,true).asString()) ;
#endif

  // ----------------------------------------------------------------
  
  // System cursor observer
#if __APPLE__
  CGEventTapLocation taploc = kCGSessionEventTap ;
  CGEventTapPlacement tapplace = kCGHeadInsertEventTap ;
  CGEventTapOptions tapopt = kCGEventTapOptionListenOnly ;
  CGEventMask eventMask = CGEventMaskBit(kCGEventLeftMouseDown) ; // | CGEventMaskBit(kCGEventRightMouseDown) | CGEventMaskBit(kCGEventOtherMouseDown) ;
  cgtap = CGEventTapCreate(taploc, tapplace, tapopt, eventMask, cgCallback, this) ;
  if (!cgtap)
    std::cerr << TimeStamp::createAsInt() << " CGEventTapCreate failed..." << std::endl ;
  else {
    cgtapsource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, cgtap, 0) ;
    CFRunLoopAddSource(CFRunLoopGetCurrent(), cgtapsource, kCFRunLoopCommonModes) ;
    CGEventTapEnable(cgtap, true) ;
  }
#endif

  // ----------------------------------------------------------------
  
  // Screen update observer
#if __APPLE__
  CGRegisterScreenRefreshCallback(screenCallback, this) ;
#endif
  
  // ----------------------------------------------------------------

  *logstream << std::endl
	     << "# -- begin config ------------------------------------------" << std::endl
	     << LagMeterQt::cfg.dump("\n", ": ") << std::endl
	     << "# -- end config --------------------------------------------" << std::endl
	     << std::endl ;

  // ----------------------------------------------------------------

  resize(600, 800) ;
  setAutoFillBackground(true) ;
  setBackground(Qt::black) ;  
  show() ;
}

// ---------------------------------------------------------------------------------

void
LagMeterQt::setBackground(QColor color) {
  QPalette pal = palette() ;
  pal.setColor(QPalette::Background, color) ;
  setPalette(pal) ;
}

// ----------------------------------------------------------------------------------------

void
LagMeterQt::turboHIDCallback(void *context,
			     TimeStamp::inttime timestamp, int nbBytesReceived, unsigned char *data) {
  std::stringstream logline ;
  logline << timestamp << " leonardo " << nbBytesReceived << " " ;
  for (int i = 0; i < nbBytesReceived; i++) logline << (int)data[i] << " ";
  logline << std::endl ;
  LagMeterQt *self = (LagMeterQt*)context ;
  *self->logstream << logline.str() ;
}

// ---------------------------------------------------------------------------------

#ifndef NO_LIBPOINTING
void
LagMeterQt::pointingCallback(void *context, TimeStamp::inttime timestamp,
			     int /*input_dx*/, int /*input_dy*/, int buttons) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  static int lastbuttons = 0 ;
  if (buttons==1 && lastbuttons==0) {
    LagMeterQt *self = (LagMeterQt*)context ;
    *self->logstream << now << " libpointing " << timestamp << " " << std::endl ;
  }
  lastbuttons = buttons ;
}
#endif

// ----------------------------------------------------------------------------------------

#if __APPLE__
CGEventRef
LagMeterQt::cgCallback(CGEventTapProxy /*proxy*/, CGEventType /*type*/, CGEventRef event, void *context) {
  // Convert the low-level timestamp BEFORE checking the current time to make sure we preserve temporal order...
  static TimeStamp::inttime epoch ;
  static uint64_t epoch_mach ;
  static mach_timebase_info_data_t sTimebaseInfo ;
  TimeStamp::inttime timestamp = TimeStamp::undef ;  
  if (sTimebaseInfo.denom == 0) {
    struct timeval stamp ;
    gettimeofday(&stamp, 0) ;
    (void) mach_timebase_info(&sTimebaseInfo) ;    
    epoch_mach = mach_absolute_time() ;
    timestamp = epoch = (TimeStamp::inttime)stamp.tv_sec*TimeStamp::one_second + (TimeStamp::inttime)stamp.tv_usec*TimeStamp::one_microsecond ;

  } else {
    uint64_t elapsedNano = (mach_absolute_time()-epoch_mach) * sTimebaseInfo.numer / sTimebaseInfo.denom ;
    timestamp = epoch + elapsedNano*TimeStamp::one_nanosecond ;
  }

  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  
  LagMeterQt *self = (LagMeterQt*)context ;
  CGPoint cursor = CGEventGetLocation(event) ;
  *self->logstream << now << " system " << cursor.x << "," << cursor.y << " " << timestamp << std::endl ;
  return event ;
}
#endif

// ----------------------------------------------------------------------------------------

void
LagMeterQt::mousePressEvent(QMouseEvent *event) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;

  if (event->buttons()==Qt::LeftButton) {
    *logstream << now
	       << " toolkit "
	       << event->globalX() << "," << event->globalY()
	       << std::endl ;

    setBackground(Qt::white) ;
    if (immediate_refresh)
      repaint() ;
    else
      update() ;
  }
}

void
LagMeterQt::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button()==Qt::LeftButton) {  
    setBackground(Qt::black) ;
    if (immediate_refresh)
      repaint() ;
    else
      update() ;
  }
}

// ---------------------------------------------------------------------------------

void
LagMeterQt::paintEvent(QPaintEvent */*event*/) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  if (QApplication::mouseButtons()==Qt::LeftButton)
    *logstream << now << " repaint" << std::endl ;
}

// ---------------------------------------------------------------------------------

#if __APPLE__
void 
LagMeterQt::screenCallback(CGRectCount count, const CGRect *rectArray, void *context) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  if (QApplication::mouseButtons()!=Qt::LeftButton) return ;
  LagMeterQt *self = (LagMeterQt*)context ;
  CGRect win = self->geometry().toCGRect() ;
  for (CGRectCount i=0; i<count; ++i) {
    if (CGRectIntersectsRect(win, rectArray[i])) {
      *self->logstream << now << " screen " << std::endl ;
      break ;
    }
  }
}
#endif

// ---------------------------------------------------------------------------------

LagMeterQt::~LagMeterQt(void) {
  if (turboHID) {
    turboHID->stopThread() ;
    delete turboHID ;
  }
  if (logstream!=(&std::cout)) delete logstream ;
}
