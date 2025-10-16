/*
 *
 * main.cpp --
 *
 * Authors: Nicolas Roussel
 *
 * Copyright © Inria
 *
 */

#include "TurboHID.h"

#include <pointing/pointing.h>
#include <pointing/utils/ConfigDict.h>

#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <GLUT/glut.h>
#include <sys/time.h>
#include <mach/mach_time.h>
#else
#include <GL/glut.h>
#endif

#include <fstream>

// --------------------------------------------------------------------------------

using namespace pointing ;

ConfigDict cfg ;
TurboHID *turboHID ;  
PointingDevice *pdev = 0 ;
bool buttonPressed = false ;
std::ostream *logstream = 0 ;

#define USE_DOUBLE_BUFFER 1
#define USE_FENCE         1

// ----------------------------------------------------------------------------------------

void
turboHIDCallback(void */*context*/,
		 TimeStamp::inttime timestamp, int nbBytesReceived, unsigned char* data) {
  std::stringstream logline ;
  logline << timestamp << " leonardo " << nbBytesReceived << " " ;
  for (int i = 0; i < nbBytesReceived; i++) logline << (int)data[i] << " ";
  logline << std::endl ;
  *logstream << logline.str() ;
}

// ----------------------------------------------------------------------------------------

#ifndef NO_LIBPOINTING
void
libpointingCallback(void */*context*/, TimeStamp::inttime timestamp,
			   int /*input_dx*/, int /*input_dy*/, int buttons) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  static int lastbuttons = 0 ;
  if (buttons==1 && lastbuttons==0)
    *logstream << now << " libpointing " << timestamp << " " << std::endl ;
  lastbuttons = buttons ;
}
#endif

// ----------------------------------------------------------------------------------------

#if __APPLE__
CGEventRef
systemCallback(CGEventTapProxy /*proxy*/, CGEventType /*type*/, CGEventRef event, void */*refcon*/) {
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
  
  CGPoint cursor = CGEventGetLocation(event) ;
  *logstream << now << " system " << cursor.x << "," << cursor.y << " " << timestamp
#if DEBUG_MODE
		     << " " << (now-timestamp)
#endif
		     << std::endl ;
  return event ;
}
#endif

// ----------------------------------------------------------------------------------------

void
toolkitCallback(int button, int state, int x, int y) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  if (button==GLUT_LEFT_BUTTON) {
    switch (state) {
    case GLUT_DOWN: {
      int winx = glutGet(GLUT_WINDOW_X) ;
      int winy = glutGet(GLUT_WINDOW_Y) ;
      *logstream << now << " toolkit " << winx+x << "," << winy+y << std::endl ;
      buttonPressed = true ;
    } break ;
    case GLUT_UP:
      buttonPressed = false ;
      break ;
    }
    glutPostRedisplay() ;
  }
}

// ----------------------------------------------------------------------------------------

void
displayCallback(void) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  
  if (buttonPressed) {
    (*logstream) << now << " repaint" << std::endl ;
    glClearColor(1.0, 1.0, 1.0, 1.0) ;    
  } else
    glClearColor(0.0, 0.0, 0.0, 1.0) ;
  
  glClear(GL_COLOR_BUFFER_BIT) ;

#if USE_DOUBLE_BUFFER
  glutSwapBuffers() ;
#endif
  
#if USE_FENCE
  GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0) ;
  glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1e9 /* nanoseconds */) ;
  glDeleteSync(sync) ;
#else
  glFinish() ;
#endif
}

// ----------------------------------------------------------------------------------------

#if __APPLE__
void 
screenCallback(CGRectCount count, const CGRect *rectArray, void */*userParameter*/) {
  TimeStamp::inttime now = TimeStamp::createAsInt() ;
  if (!buttonPressed) return ;  
  CGRect win = CGRectMake(glutGet(GLUT_WINDOW_X), glutGet(GLUT_WINDOW_Y), glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)) ;
  for (CGRectCount i=0; i<count; ++i)
    if (CGRectIntersectsRect(win, rectArray[i])) {
      *logstream << now << " screen " << std::endl ;
      break ;
    }
}
#endif

// ----------------------------------------------------------------------------------------

int
main(int argc, char** argv) {
  glutInit(&argc, argv) ;

  if (argc>1) cfg.loadFrom(argv[1]) ;
  cfg.setdefault("libpointing-input", "any:?debugLevel=1") ;  
  cfg.setdefault("meter-logfile", "") ;

  std::string logfile = cfg.get<std::string>("meter-logfile") ;
  if (logfile.empty())
    logstream = &std::cout ;
  else
    logstream = new std::ofstream(logfile.c_str()) ;

  // ------------------------------------------------------------------------

#if USE_DOUBLE_BUFFER
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE) ;
#else
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE) ; // GLUT_DOUBLE
#endif
  glutInitWindowSize(600, 800) ;
  glutCreateWindow("Lag Meter") ;
  glClearColor(0.0, 0.0, 0.0, 1.0) ;
  glutMouseFunc(toolkitCallback) ;
  glutDisplayFunc(displayCallback) ;

  // ------------------------------------------------------------------------

  // TurboHID observer
  try {
    turboHID = new TurboHID ;
    turboHID->setTurboHIDCallback(turboHIDCallback, 0) ;
    turboHID->startThread() ;
  } catch (std::exception &e) {
    turboHID = 0 ;
  }

  // ------------------------------------------------------------------------

  DisplayDevice *display = DisplayDevice::create("any:") ;
  cfg.set("libpointing-output", display->getURI(true).asString()) ;
  delete display ;
  
  // HID traffic observer
  pdev = PointingDevice::create(cfg.get<std::string>("libpointing-input")) ;
  for (TimeStamp reftime, now;
       !pdev->isActive() && now-reftime<15*TimeStamp::one_second; 
       now.refresh())
    PointingDevice::idle(500) ;
  pdev->setPointingCallback(libpointingCallback, 0) ;
  cfg.set("libpointing-input", pdev->getURI(true,true).asString()) ;

  // ------------------------------------------------------------------------

  // System cursor observer
#if __APPLE__
  CGEventTapLocation taploc = kCGSessionEventTap ;
  CGEventTapPlacement tapplace = kCGHeadInsertEventTap ;
  CGEventTapOptions tapopt = kCGEventTapOptionListenOnly ;
  CGEventMask eventMask = CGEventMaskBit(kCGEventLeftMouseDown) ; // | CGEventMaskBit(kCGEventRightMouseDown) | CGEventMaskBit(kCGEventOtherMouseDown) ;
  CFMachPortRef cgtap = CGEventTapCreate(taploc, tapplace, tapopt, eventMask, systemCallback, 0) ;
  if (!cgtap)
    std::cerr << TimeStamp::createAsInt() << " CGEventTapCreate failed..." << std::endl ;
  else {
    CFRunLoopSourceRef cgtapsource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, cgtap, 0) ;
    CFRunLoopAddSource(CFRunLoopGetCurrent(), cgtapsource, kCFRunLoopCommonModes) ;
    CGEventTapEnable(cgtap, true) ;
  }
#endif
  
  // ------------------------------------------------------------------------
  
  // Screen update observer
#if __APPLE__
  CGRegisterScreenRefreshCallback(screenCallback, NULL) ;
#endif
  
  // ------------------------------------------------------------------------
  
  *logstream << std::endl
	     << "# -- begin config ------------------------------------------" << std::endl
	     << cfg.dump("\n", ": ") << std::endl
	     << "# -- end config --------------------------------------------" << std::endl
	     << std::endl ;

  // ------------------------------------------------------------------------
  
  glutMainLoop() ;

  // ------------------------------------------------------------------------
  
  if (turboHID) {
    turboHID->stopThread() ;
    delete turboHID ;
  }
  if (logstream!=(&std::cout)) delete logstream ;
}
