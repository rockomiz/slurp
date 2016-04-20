/*
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

#ifndef SLURP_INTERACTER_H
#define SLURP_INTERACTER_H

#include <QMainWindow>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>
#include <QUrl>
#include <QKeyEvent>
#include <QIcon>

#include "about.h"

namespace slurp {

    class Interacter: public QMainWindow {
        Q_OBJECT 
           
           QWidget * centralwidget;
           QProgressBar *progressBar;
           QLCDNumber *bitrateNumber;
           QLCDNumber *queuedNumber;
           QLCDNumber *crawledNumber;
           QTextBrowser *urlBrowser;
           QSplitter *splitter;
           QLineEdit *urlEntry;
           QPushButton *crawlButton;
           QPushButton *aboutButton;
           QIcon *icon;
           About *aboutBox;

        public:

           Interacter();

        public slots:

           void updateStats(int queued, int crawled, double avgBytesPerSecond);
           void updateProgress(int n);
           void newUrl(QUrl url);
           void stopComplete();

        signals: 
        
           void crawlClicked(QUrl seedUrl);
           void crawlStarted();
           void crawlAborted();

        private slots: 
           
           void handleCrawl();
           void handleAboutClicked();
           void handleReturnPressed();
           void handleUrlChange(const QString & newUrl);
    };
}                               /* namespace slurp */
#endif                          /* SLURP_INTERACTER_H */
