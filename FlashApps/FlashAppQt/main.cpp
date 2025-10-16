/* -*- mode: c++ -*-
 *
 *
 * Authors: Géry Casiez
 *
 * Copyright © Inria
 *
 */

#include <QApplication>

#include "MyRectangle.h"

int main(int argc, char **argv)
{
    QApplication app (argc, argv);

    MyRectangle myRectangle;
    //myRectangle.show();
    myRectangle.showFullScreen();

    return app.exec();
}
