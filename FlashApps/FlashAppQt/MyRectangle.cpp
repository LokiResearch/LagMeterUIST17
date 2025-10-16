/*
 *
 * PatternViewer.cpp --
 *
 * Authors: Géry Casiez, Nicolas Roussel
 *
 * Copyright © Inria
 *
 */

#include "MyRectangle.h"

#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QStandardPaths>
#include <QDateTime>

#include <iostream>

#include <QDebug>

MyRectangle::MyRectangle(QWidget *parent) : QWidget(parent) {
    setGeometry(0,0,800,600);
    setAttribute(Qt::WA_AcceptTouchEvents, true);

#ifdef ANDROID
    QDateTime today = QDateTime::currentDateTime();
    QString timestamp = "/storage/emulated/0/HIDLogger/touchLogger-qt-";
    timestamp += today.toString("yyyy-MM-dd-hh-mm-ss");
    timestamp += ".log";
    logfile = new QFile(timestamp);
    logfile->open(QIODevice::WriteOnly);
    logstream = new QTextStream(logfile);
    qDebug() << "Log filepath: " << logfile->fileName();
#endif

    debug = "";
}

void
MyRectangle::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this) ;

  static int windowWidth = painter.window().width();
  static int windowHeight = painter.window().height();

  QRect r = QRect(0, 0, windowWidth, windowHeight);
  painter.fillRect(r,QBrush((white ? Qt::white : Qt::black))) ;
  painter.setPen(Qt::cyan);
#ifdef ANDROID
  painter.drawText(QRect(5, 5, 2000, 100), logfile->fileName());
#endif
}


bool MyRectangle::event(QEvent *e) {
    QEvent::Type et = e->type() ;

    qint64 time = QDateTime::currentMSecsSinceEpoch();
    time *= 1000000; // To ns
    QTouchEvent *t = static_cast<QTouchEvent*>(e);

    switch (et) {
        case QEvent::MouseButtonPress:
            debug = "Mouse";
            white = true;
            #ifdef DEBUG
            std::cout << "Mouse pressed" << std::endl;
            #endif
            repaint();
            break;

        case QEvent::MouseButtonRelease:
            white = false;
            #ifdef DEBUG
            std::cout << "Mouse released" << std::endl;
            #endif
            repaint();
            break;

        case QEvent::TouchBegin:
            debug = "Touch";
            white = true;
#ifdef ANDROID
            *logstream << time << " toolkit ";
            *logstream << t->touchPoints().at(0).pos().x() << ",";
            *logstream << t->touchPoints().at(0).pos().y() << "\n";
            logstream->flush();
#endif
            #ifdef DEBUG
            std::cout << "Touch down" << std::endl;
            #endif
  #ifdef ANDROID
            *logstream << QString::number(QDateTime::currentMSecsSinceEpoch()*1000000) << " repaint\n";
  #endif
            repaint();
            break;

        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            white = false;
            #ifdef DEBUG
            std::cout << "Touch up" << std::endl;
            #endif
            repaint();
            break;

        default:
            return QWidget::event(e);
            break;
    }
    return true;
}

MyRectangle::~MyRectangle() {
#ifdef ANDROID
    qDebug() << "Flushing...";
    logstream->flush();
    logfile->close();
#endif
}
