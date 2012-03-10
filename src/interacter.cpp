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

#include <QApplication>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QIcon>

#include <QDebug>

#include "globals.h"
#include "interacter.h"
#include "parser.h"
#include "about.h"

namespace slurp {

    Interacter::Interacter() {
        icon = new QIcon("res/slurp.png");

        setWindowIcon(*icon);
        setFixedSize(512, 361);

        aboutBox = new About(this);

        centralwidget = new QWidget(this);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setGeometry(QRect(20, 69, 481, 31));
        progressBar->setValue(0);

        bitrateNumber = new QLCDNumber(centralwidget);
        bitrateNumber->setGeometry(QRect(20, 207, 64, 23));

        queuedNumber = new QLCDNumber(centralwidget);
        queuedNumber->setGeometry(QRect(20, 168, 64, 23));

        crawledNumber = new QLCDNumber(centralwidget);
        crawledNumber->setGeometry(QRect(20, 129, 64, 23));

        urlBrowser = new QTextBrowser(centralwidget);
        urlBrowser->setGeometry(QRect(110, 129, 381, 202));
        urlBrowser->setOpenExternalLinks(true);

        splitter = new QSplitter(centralwidget);
        splitter->setGeometry(QRect(20, 29, 471, 25));
        splitter->setOrientation(Qt::Horizontal);

        urlEntry = new QLineEdit("http://", splitter);
        splitter->addWidget(urlEntry);

        crawlButton = new QPushButton("Crawl", splitter);
        crawlButton->setEnabled(false);
        splitter->addWidget(crawlButton);

        aboutButton = new QPushButton("About", splitter);
        splitter->addWidget(aboutButton);

        setCentralWidget(centralwidget);

        QObject::connect(crawlButton, SIGNAL(clicked()),
                         this, SLOT(handleCrawl()));

        QObject::connect(aboutButton, SIGNAL(clicked()),
                         this, SLOT(handleAboutClicked()));

        QObject::connect(urlEntry, SIGNAL(textChanged(const QString &)),
                         this, SLOT(handleUrlChange(const QString &)));

         QObject::connect(urlEntry, SIGNAL(returnPressed()),
                          this, SLOT(handleReturnPressed()));

        emit urlEntry->setFocus();
    } void Interacter::updateStats(int queued, int crawled,
                                   double avgBytesPerSecond) {
        emit queuedNumber->display(queued);
        emit crawledNumber->display(crawled);
        emit bitrateNumber->display(avgBytesPerSecond / 1024);
    }

    void Interacter::updateProgress(int n) {
        emit progressBar->setValue(n);
    }

    void Interacter::newUrl(QUrl url) {
        urlBrowser->append("<a href='" +
                           url.toString() + "'>" + url.toString() + "</a> ");
    }

    void Interacter::stopComplete() {
        qDebug() << "interacter: received stopComplete signal ";

        crawlButton->setText("Crawl");
    }

    void Interacter::handleCrawl() {
        if (crawlButton->text() == "Crawl") {
            crawlButton->setText("Stop");

            QUrl seedUrl = QUrl(urlEntry->text());

            emit crawlClicked(seedUrl);
            emit crawlStarted();
        } else if (crawlButton->text() == "Stop") {
            qDebug() << "interface: user aborted crawl";

            crawlButton->setText("Crawl");
            emit crawlAborted();
        }
    }

    void Interacter::handleAboutClicked() {
        aboutBox->show();
    }

    void Interacter::handleReturnPressed() {
        if (crawlButton->text() == "Crawl" &&
            Parser::validateUrl(urlEntry->text())) {
            emit handleCrawl();
        }
    }

    void Interacter::handleUrlChange(const QString & newUrl) {
        if (Parser::validateUrl(newUrl)) {
            crawlButton->setEnabled(true);
        } else {
            crawlButton->setEnabled(false);
        }
    }

}                               /* namespace slurp */
