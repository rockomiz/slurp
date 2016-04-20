﻿/*
 * Slurp - a web crawler
 * Copyright (C) 2011 Joseph Max DeLiso

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <QDebug>
#include <QThread>
#include <QApplication>
#include <QFile>
#include <QUrl>
#include <QSharedPointer>

#include "globals.h"
#include "eventer.h"
#include "interacter.h"

using namespace slurp;

int main(int argc, char **argv)
{
    Eventer::logFile.setFileName("slurp.log");
    Eventer::logFile.open(QIODevice::ReadWrite);

//    qInstallMessageHandler(Eventer::debugHandler);

    qDebug() << "slurp started up";

    Eventer ev(argc, argv);

    Interacter inter;

    QObject::connect(&inter, SIGNAL(crawlClicked(QUrl)),
                     &ev, SLOT(addUrl(QUrl)), Qt::QueuedConnection);

    QObject::connect(&inter, SIGNAL(crawlStarted()),
                     &ev, SLOT(startCrawling()), Qt::QueuedConnection);

    QObject::connect(&inter, SIGNAL(crawlAborted()),
                     &ev, SLOT(stopCrawling()), Qt::QueuedConnection);

    QObject::connect(&ev, SIGNAL(newUrl(QUrl)),
                     &inter, SLOT(newUrl(QUrl)), Qt::QueuedConnection);

    QObject::connect(&ev, SIGNAL(statsChanged(int, int, double)),
                     &inter, SLOT(updateStats(int, int, double)),
                     Qt::QueuedConnection);

    QObject::connect(&ev, SIGNAL(progressChanged(int)),
                     &inter, SLOT(updateProgress(int)), Qt::QueuedConnection);

    QObject::connect(&ev, SIGNAL(lastParserFinished()),
                     &inter, SLOT(stopComplete()), Qt::QueuedConnection);

    inter.show();
    return ev.exec();
}
