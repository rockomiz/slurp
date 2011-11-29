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

/*
 * note: the example hiperfifo.c by Jeff Pohlmeyer was referenced extensively
 * while implementing this class.
 */

#include <iostream>

#include "globals.h" 

using namespace slurp;

Retriever::Retriever( Eventer* eventer, QString uri, int flags ) {
  this -> uri = uri;
  this -> flags = flags;
  owner = eventer;

  errorBuffer[0] = '\0';
  evset = false;
  setAutoDelete(false);

  conn = curl_easy_init();

  qDebug() << "debug: constructed retriever with owner @" 
	      << owner << "\n";
}

Retriever::~Retriever() {
  qDebug() << "debug: deleting retriever instance\n";
  
  if( conn ) {
     curl_easy_cleanup( conn );
  }

  if( evset ) {
     event_del( & ev );
  }
}

void Retriever::setSocketData( curl_socket_t sockfd, int action, int kind, CURL* curlHandle ) {
  qDebug() << "debug: setting socket data with sock " << sockfd 
           << " action " << action
           << " kind " << kind 
           << " and handle@ " << curlHandle << "\n";
  qDebug() << "debug: the handle of this retriever is at " << conn << "\n";

  
  //f->sockfd = s;
  //f->action = act;
  //f->easy = e; should already have this

  //if (f->evset)
  //  event_del(&f->ev);

  //event_set(&f->ev, f->sockfd, kind, event_cb, g);
  //f->evset=1;
  //event_add(&f->ev, NULL);
}


void Retriever::run() {
    CURLMcode rc;

    if( conn ) {  
      curl_easy_setopt(
          conn, 
          CURLOPT_URL, 
	  uri.toAscii().data());
      curl_easy_setopt(
          conn, 
          CURLOPT_WRITEFUNCTION, 
	  writeCallback);
      curl_easy_setopt(
          conn,
          CURLOPT_WRITEDATA,
	  this);
      curl_easy_setopt(
          conn, 
          CURLOPT_VERBOSE, 
	  flags & FLAGS_VERBOSE ); 
      curl_easy_setopt(
          conn, 
          CURLOPT_NOPROGRESS, 
	  ~(flags & FLAGS_VERBOSE) );
      curl_easy_setopt( 
          conn, 
          CURLOPT_PROGRESSFUNCTION, 
          progressCallback);
      curl_easy_setopt( 
          conn, 
          CURLOPT_PROGRESSDATA, 
          this);
      curl_easy_setopt(
          conn, 
          CURLOPT_ERRORBUFFER, 
	  errorBuffer);
      curl_easy_setopt( 
          conn, 
          CURLOPT_PRIVATE, 
          this);

      rc = curl_multi_add_handle(owner -> getMultiHandle(), conn);
      curlVerify("curl_multi_add_handle from Retriever()", rc);

      qDebug() << "debug: added retriever with easy @"
	        << conn << " to multi handle owned by eventer @"
	        << owner << " with multi @" << owner->getMultiHandle() 
		<< "and target of: " << uri.toAscii().data() << "\n";
  } else {
      qDebug() << "error: could not initialize retriever curl handle\n";
  }

}

bool Retriever::isValid() const {
  return (uri.length() > 0) && ( conn != NULL );
}
