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

#include <iostream>
#include "eventer.h"

using namespace slurp;

Eventer::Eventer( 
    const QQueue<QString>& initialURIs, 
    int quota, 
    int maxThreads ) {
      pendingURIs = initialURIs;
      this->quota = quota;
      threadPool.setMaxThreadCount( maxThreads );
      multi = curl_multi_init();
}

Eventer::~Eventer() {
  threadPool.waitForDone();	
  if( multi )
  {
    curl_multi_cleanup(multi);
  }
}

void Eventer::curlVerify(const char *where, CURLMcode code)
{
  if ( CURLM_OK != code ) {
    const char *s;
    switch (code) {
      case     CURLM_CALL_MULTI_PERFORM: s="CURLM_CALL_MULTI_PERFORM"; break;
      case     CURLM_BAD_HANDLE:         s="CURLM_BAD_HANDLE";         break;
      case     CURLM_BAD_EASY_HANDLE:    s="CURLM_BAD_EASY_HANDLE";    break;
      case     CURLM_OUT_OF_MEMORY:      s="CURLM_OUT_OF_MEMORY";      break;
      case     CURLM_INTERNAL_ERROR:     s="CURLM_INTERNAL_ERROR";     break;
      case     CURLM_UNKNOWN_OPTION:     s="CURLM_UNKNOWN_OPTION";     break;
      case     CURLM_LAST:               s="CURLM_LAST";               break;
      default: s="CURLM_unknown";
        break;
    case     CURLM_BAD_SOCKET:         s="CURLM_BAD_SOCKET";
       std::cerr << "ERROR: " << where << "returns" << s << std::endl;
      return;
    }
    std::cerr << "ERROR: " << where << "returns" << s << std::endl;
    exit(code);
  }
}

int Eventer::run() {

  //threadPool.start( new QRunnable() );

  threadPool.waitForDone();
  return 0;
}

void Eventer::queueURI( const QString& uri ) {
  pendingURIs.enqueue( uri );
}


