/*
 *
 * main.cpp --
 *
 * Authors: Nicolas Roussel, Géry Casiez
 *
 * Copyright © Inria
 *
 */

#include <QApplication>
#include <QTextCodec>

#include "LagMeterQt.h"

#include <stdexcept>
#include <iostream>

int
main(int argc, char *argv[]) {
  QApplication app(argc, argv) ;

  try {

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8")) ;
#endif

    LagMeterQt meter(&app, 0, QCoreApplication::arguments()) ;
    meter.raise() ;

    return app.exec() ;

  } catch (std::runtime_error &e) {
    std::cerr << "Runtime error: " << e.what() << std::endl ;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl ;
  }

  return 0 ;
}
