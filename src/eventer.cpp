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
#include <QCoreApplication>
#include <QMutex>
#include <QDebug>
#include <QUrl>
#include <QQueue>
#include <QVector>
#include <QMetaType>
#include <QSet>
#include <QFile>
#include <QSharedPointer>
#include <QTime>

#include "globals.h"
#include "eventer.h"
#include "parser.h"

namespace slurp {

    QFile Eventer::logFile; 

    Eventer::Eventer(
        int& argc, 
        char** argv) : QApplication( argc, argv, true ) {
            QUrl currentUrl;

            setOrganizationName("Megafrock Laboratories");
            setApplicationName("Slurp");

            pagesCrawled = 0;
            totalBytes = 0;

            active = false;
    }

    void Eventer::die(const char *errmsg, int errcode) {
        qFatal(errmsg);
        exit(errcode);
    }

    void Eventer::debugHandler(QtMsgType type, const char* msg ) {
        (void)type;

        logFile.write( msg, qstrlen( msg ) );
        logFile.write( "\n", 1 );
    }

    void Eventer::addUrl( QUrl url ) {
        if( retryMap.contains( url ) && retryMap[ url ] >= 3 ) {
            qDebug() << "discarding url because we've failed to parse it thrice" 
                     << url;
            emit dispatchParsers();
            return;
        } 
        
        if( queuedUrls.contains( url ) && !retryMap.contains( url) ) {
            qDebug() << "discarding duplicate not in retry map" << url;
            return;
        }
         
		queuedUrls.insert( url );
        queuedParsers.enqueue( QSharedPointer< Parser > ( new Parser( url ) ) );

        emit dispatchParsers();
    }

    void Eventer::parserFinished( QUrl seed ) {
        if( !runningParserMap.contains( seed ) ) {
            qDebug() << "warning: got parse completion for a Url not in runningParserMap";
            return ;
        }

        QSharedPointer< Parser > thisParser = runningParserMap.take( seed );

        ++pagesCrawled;
        totalBytes += thisParser -> getTotalBytes();

        foreach( QUrl currentUrl, thisParser -> getResults() ) {
            emit addUrl( currentUrl );
            emit newUrl( currentUrl );
        }
    
        emit dispatchParsers(); 
        emit statsChanged( 
            queuedParsers.count(), 
            pagesCrawled, 
            (double)totalBytes/ ( crawlTime.elapsed()/1000 ));

        qDebug() << "eventer: " << totalBytes << " bytes processed";
    } 

    void Eventer::stopCrawling() {
        qDebug() << "user aborted crawl";
        active = false;
    }

    void Eventer::startCrawling() {
        active = true;

        crawlTime.start();
        retryMap.clear();

        emit dispatchParsers();
    }

    void Eventer::dispatchParsers() {
        if( !active ) {
            return;
        }

        while( !queuedParsers.isEmpty() && runningParserMap.count() < 8 ) {
            qDebug() << "starting a new parser";
        
            QSharedPointer< Parser > queuedParser = queuedParsers.dequeue();
      
            emit queuedParser -> requestPage();

            QObject::connect(queuedParser.data(), 
                SIGNAL(finished(QUrl)), 
                this, 
                SLOT(parserFinished(QUrl)), 
                Qt::QueuedConnection);
                
            QObject::connect(queuedParser.data(), 
                SIGNAL(progress(int)),
                this, 
                SLOT(parserProgress(int)), 
                Qt::QueuedConnection);

            QObject::connect(queuedParser.data(), 
                SIGNAL(parseFailed(QUrl)),
                this, 
                SLOT(handleParseFailure(QUrl)), 
                Qt::QueuedConnection);

            runningParserMap.insert( queuedParser -> getUrl() , queuedParser );
			
			qDebug() << "queued: " << queuedParsers.count();
        }

        if( active && 
            runningParserMap.count() == 0 && 
            queuedParsers.count()  == 0 ) {
                qDebug() << "eventer: in dispatch parsers with"
                         << "nothing running and nothing queued";
                active = false;
                emit lastParserFinished();
        }
    }
    
    void Eventer::parserProgress( int n ) {
        emit progressChanged( n );
    }

    void Eventer::handleParseFailure( QUrl url ) {
        if( !runningParserMap.contains( url ) ) {
            qDebug() << "warning: got parse failure for a Url not in runningParserMap";
            return;
        }

        if( !retryMap.contains( url ) ) {
            retryMap[ url ] = 1;
        } else {
            ++retryMap[ url ];
        }

        qDebug() << url << " has failed to parse " 
                 << retryMap[url] << " times";

        runningParserMap.remove( url );

        emit addUrl( url );
        emit dispatchParsers();
    }

    void Eventer::forceStop() {
        qDebug() << "forcing stop due to timeout...\n";
        /* TODO: stub */
    }
}    /* namespace slurp */
