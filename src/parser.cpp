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

#include <QDebug>
#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QWebSettings>
#include <QPainter>
#include <QWidget>
#include <QUrl>
#include <QSharedPointer>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "globals.h"
#include "parser.h"
#include "eventer.h"

namespace slurp {

    Parser::Parser(QUrl url) {
        this->url = url;
        qDebug() << "constructing parser on thread " << thread();
    } 
    
    Parser::~Parser() {
        qDebug()
            << "destroying parser for "
            << url
            << " with network manager " << networkManager
            << " and page instance " << page.data()
            << " on thread " << thread();

        if (networkManager) {
            networkManager->deleteLater();
        }

        if (page) {
            page.clear();
        }
    }

    /* A public static function which is used to screen crawled URLs 
     * for potential queuing 
     */
    bool Parser::validateUrl(QUrl url) {
        if (!url.isValid()) {
            qDebug() << "discarding invalid " << url;
            return false;
        } else if (url.scheme() == "https") {
            qDebug() << "discarding https " << url;
            return false;
        } else if (url.host() == "") {
            qDebug() << "discarding url with no host" << url;
            return false;
        }

        return true;
    }

    /* This function constructs a page instance and initiates network I/O.
     * Note that this function must always run on the GUI thread ( the eventer
     * thread. Even though QWebPage doesn't inherit from QWidget it will still
     * make window manager calls which all must come from the same thread.
     * Attempting to put this call into a separate thread WILL cause heap
     * corruption and eventually a segfault. It will fail with an assertion
     * at run-time on windows. */
    void Parser::requestPage() {
        qDebug() << "parser: constructing page instance on thread " << thread();

        page = QSharedPointer < QWebPage > (new QWebPage(this));
        networkManager =
            QPointer < QNetworkAccessManager >
            (new QNetworkAccessManager(this));

        page->setNetworkAccessManager(networkManager);
        page->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
        page->settings()->setAttribute(QWebSettings::AutoLoadImages, false);
        page->settings()->setAttribute(QWebSettings::PluginsEnabled, false);

        QObject::connect(page.data(), SIGNAL(loadProgress(int)),
                         this, SLOT(loadProgress(int)), Qt::QueuedConnection);

        QObject::connect(page.data(), SIGNAL(loadFinished(bool)),
                         this, SLOT(pageLoadFinished(bool)),
                         Qt::QueuedConnection);

        qDebug() << "parser: initiating page load";
        page->mainFrame()->load(url);
    }

    /* The parse function is invoked after the loadFinished signal is received from 
     * the webkit code upon successfully parsing a page. From here, the entire DOM
     * can be manipulated and traversed really conveniently. The code right now
     * just traverses the DOM looking for link tags, and queues them if they are 
     * valid, but this function is where all of the interesting development will
     * take place in the future. 
     */
    void Parser::parse() {
        QUrl currentUrl;
        QWebElementCollection linkTags =
            page->mainFrame()->findAllElements("a");

        foreach(QWebElement current, linkTags) {
            currentUrl = QUrl(current.attribute("href"));
            /* This discards the fragment. It is useless in this context and 
             * will complicate our visited hashtable.
             */
            currentUrl.setEncodedFragment(QByteArray());

            if (currentUrl.isEmpty()) {
                continue;
            }

            /* Prepend the parent URL if we have a relative link in an attempt to 
             * validate it for retrieval.
             */
            if (currentUrl.isRelative() &&
                currentUrl.host() == "" && currentUrl.path() != "") {
                qDebug() << currentUrl << " is relative path. prepending host";
                currentUrl.setHost(url.host());
                currentUrl.setScheme(url.scheme());
                qDebug() << "with host fix: " << currentUrl;
            }

            /* Finally, check to make sure it's valid before queueing it */
            if (validateUrl(currentUrl)) {
                parsedUrls.push_back(currentUrl);
            }
        }

        emit finished(url);
    }

    /* This function simply updates the GUI to tell it how far along
     * this page is in being loaded. 
     */
    void Parser::loadProgress(int n) {
        qDebug() << "parser: " << url << " load progress " << n;

        QCoreApplication::flush();
        emit progress(n);
    }

    /* This function is connected to the webkit code and receives the load
     * results. On successful load, it schedules a parse. Otherwise, it 
     * reports failure. 
     */
    void Parser::pageLoadFinished(bool ok) {
        if (ok) {
            qDebug() << "parser: " << url << " page load finished ok.";

            emit parse();
        } else {
            qDebug() << "parser: failed to parse page " << url;
            emit parseFailed(url);
        }
    }
}                               /* namespace slurp */
