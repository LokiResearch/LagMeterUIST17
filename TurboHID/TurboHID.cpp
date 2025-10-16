//      Gery Casiez
//      October 2016

#include "TurboHID.h"

//#define USE_FEATURE_REPORT
#define USE_OUTPUT_REPORT

#define SIZE 64

TurboHID::TurboHID()
{
    if (hid_init() != 0)
        throw ExceptionInitHIDAPI();

    if (!(handle = hid_open(0x2341, 0x8036, NULL)))
    {
        handle = NULL;
        throw ExceptionNotFound();
    }

    for (int i = 0; i < SIZE; i++)
        data[i] = i;
}

TurboHID::~TurboHID()
{

    run = false;
#ifdef WIN32
    WaitForSingleObject(hThreads[0], INFINITE);
#else
    while (!threadFinished)
        usleep(100000);
#endif
     if (!handle)
         hid_close(handle);
}

void TurboHID::Send() const
{
    if (!handle)
        throw ExceptionIOError();

  // Output report
#ifdef USE_OUTPUT_REPORT
    if (hid_write(handle, data, SIZE) < 0)
        throw ExceptionIOError();
#endif

  // Feature report
#ifdef USE_FEATURE_REPORT
    if (hid_send_feature_report(handle, data, SIZE) < 0)
        throw ExceptionIOError();
#endif
}

int TurboHID::SendData(unsigned char* data, int size) const {
    int res = -1;
    if (!handle)
        throw ExceptionIOError();

    // Output report
#ifdef USE_OUTPUT_REPORT
    res = hid_write(handle, data, size);
    if (res < 0)
          throw ExceptionIOError();
#endif
    return res;
}

int TurboHID::Receive() const
{
    int res=-1;
    if (!handle)
        throw ExceptionIOError();

    unsigned char d[SIZE];
    if ((res=hid_read(handle, d, sizeof(d))) < 0)
        throw ExceptionIOError();

    return res;
}

int TurboHID::ReceiveData(unsigned char* data) const
{
    int res=-1;
    if (!handle)
        throw ExceptionIOError();

    if ((res=hid_read(handle, data, SIZE)) < 0)
        throw ExceptionIOError();

    return res;
}

void TurboHID::startThread() {
    if (!handle)
        throw ExceptionIOError();
#ifdef WIN32
    hThreads[0]=CreateThread(NULL, NULL, eventloop, LPVOID(this), 0, &dwThreadId);
#else
    int ret = pthread_create(&thread, NULL, eventloop, (void*)this) ;
    if (ret<0) {
      perror("TurboHID::startThread") ;
      throw std::runtime_error("TurboHID: pthread_create failed") ;    
    }
    run = true;
#endif
}

void TurboHID::stopThread() {
    run = false;
    hid_close(handle);
    handle = NULL;
}

void
  TurboHID::setTurboHIDCallback(TurboHIDCallback cbck, void *ctx) {
    callback = cbck ;
    callback_context = ctx ;
  }

#ifdef WIN32
DWORD WINAPI TurboHID::eventloop(LPVOID context)
{
#else
  void *
  TurboHID::eventloop(void *context) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) ;
#endif
    int res = -1;
    unsigned char data[64];

    TurboHID *self = (TurboHID*)context ;

    while (self->run) {
        if (!self->handle) throw ExceptionIOError();
	memset(data, 0, SIZE) ;
        res=hid_read(self->handle, data, SIZE);
        TimeStamp::inttime now = TimeStamp::createAsInt();
        if (res > 0)
            self->callback(self->callback_context, now, res, data);
    }
    self->threadFinished = true;
    return 0;
}
