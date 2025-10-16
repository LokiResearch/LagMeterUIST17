//		Gery Casiez
//		Ocotber 2016

#ifndef _TURBOHID_H_
#define _TURBOHID_H_

#include <iostream>
#include <hidapi/hidapi.h>
#include <pointing/pointing.h>
#include <exception>

#ifdef WIN32
  #include <windows.h> 
#else
  #include <pthread.h>
  #include <unistd.h>
#endif

using namespace pointing;

class TurboHID{
public :
  typedef void (*TurboHIDCallback)(void *context, 
           TimeStamp::inttime timestamp, 
                  int nbBytesReceived, unsigned char* data) ;
private :
  hid_device *handle;
  unsigned char data[64];
  TurboHIDCallback callback ;
  void *callback_context ;

  bool run; // for the Loop thread

  bool threadFinished;

#ifdef WIN32
  HANDLE hThreads[1];
  DWORD dwThreadId;
  DWORD dwThreadParam;
#else
    pthread_t thread ;
#endif  

public :
  TurboHID();
  ~TurboHID();
  void Send() const;
  int SendData(unsigned char* data, int size) const;
  int Receive() const;
  int ReceiveData(unsigned char* data) const;
  void startThread();
  void stopThread();
#ifdef WIN32
  static DWORD WINAPI eventloop(LPVOID lpvThreadParam);
#else
  static void *eventloop(void *self) ;
#endif
  void setTurboHIDCallback(TurboHIDCallback callback, void *context) ;
};

class ExceptionInitHIDAPI: public std::exception
{
  virtual const char* what() const throw() { return "Failed to initialize hidapi"; }
};

class ExceptionNotFound: public std::exception
{
  virtual const char* what() const throw() { return "Arduino Leonardo not found"; }
};

class ExceptionIOError: public std::exception
{
  virtual const char* what() const throw() { return "Failed to communicate with Arduino Leonardo"; }
};

#endif
