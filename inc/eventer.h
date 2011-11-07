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

#ifndef EVENTER_H
#define EVENTER_H

#include <QString>
#include <QSet>
#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include <QMutex>
#include <QQueue>

#include <event2/event.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include <curl/curl.h>

#include <cstdlib>

#include "retriever.h"

namespace slurp {
  class Eventer {
     QQueue<QString> pendingURIs;
     QSet<QString> processedURIs;
     QThreadPool threadPool;
     QMutex uriQueueMutex;
     CURLM *multi;
     int quota;
     int flags;
     struct event_base* eventPtr;

     void newConnection();

     public:

     Eventer( 
         const QQueue<QString>& initialURIs, 
	 int quota, 
	 int maxThreads, 
	 int flags );
     ~Eventer();

     static void curlVerify(const char *where, CURLMcode code);
     static void eventCallback(int fd, short kind, void *userp);
     static void timerCallback(int fd, short kind, void *userp);
     static int socketCallback(
        CURL *e, 
	curl_socket_t s, 
	int what, 
	void *cbp, 
	void *sockp);
     static size_t writeCallback(
        void *ptr, 
	size_t size, 
	size_t nmemb, 
	void *data);
     static int progressCallback(
	void *p, 
	double dltotal, 
	double dlnow, 
	double ult,
        double uln);
     int run();
     void queueURI( const QString& uri );
  };
}


#endif 



