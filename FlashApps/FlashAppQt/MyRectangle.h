/* -*- mode: c++ -*-
 *
 * PatternViewer.h --
 *
 * Authors: Géry Casiez
 *
 * Copyright © Inria
 *
 */

#ifndef MyRectangle_h
#define MyRectangle_h

#include <QWidget>
#include <QTimer>
#include <QFile>
#include <QTextStream>

#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif


class MyRectangle : public QWidget {

  Q_OBJECT ;

  bool white = false;
#ifdef ANDROID
  QFile *logfile;
  QTextStream *logstream;
#endif
  QString debug;

protected:

  void paintEvent(QPaintEvent *event) ;
  bool event(QEvent *e);

public:

  explicit MyRectangle(QWidget *parent=0) ;

  ~MyRectangle(void) ;

} ;

#endif
