/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 *  Gabber ZeroConf Responder
 *  Copyright (c) 2002 Julian Missig
 */

#include "mDNSClientAPI.h"
#include "mDNSPlatformEnvironment.h"

static mDNS gMDNSStorage;       // mDNS core uses this to store its globals

#define RR_CACHE_SIZE 500
static ResourceRecord gRRCache[RR_CACHE_SIZE];
                                        // mDNS core uses this to cache resource records

static mDNS_PlatformSupport gPlatform;  // Stores this platform's globals

    mStatus status;
    int     result;

    gPlatform.pClientOnly  = mDNSfalse;
    gPlatform.pAvoidPort53 = gAvoidPort53;
    gPlatform.pRichTextHostName = gRichTextHostName;
    gPlatform.pProgramName = gProgramName;

    status = mDNS_Init(&gMDNSStorage, &gPlatform, gRRCache, RR_CACHE_SIZE, NULL, NULL);

    if (status == mStatus_NoError) {
        do {
            // From here on in a SIGHUP will trigger mDNSPlatformPosixRun 
            // to return.
            
            gPlatform.pStop = mDNSfalse;
            
            status = RegisterOurServices();
            if (status == mStatus_NoError) {
                status = mDNSPlatformPosixRun(&gMDNSStorage);
            }
            if (status == mStatus_NoError) {
                // The only reason mDNSPlatformPosixRun will return with 
                // no error is if we get a signal that called 
                // mDNSPlatformPosixStop , in which case we deregister our 
                // existing services, tear down and reestablish all of our 
                // interfaces, and then, if the signal wasn't SIGINT, loop 
                // to register services again.
                
                DeregisterOurServices();
                
                status = mDNSPlatformPosixRefreshInterfaceList(&gMDNSStorage);
            }
        } while (status == mStatus_NoError && ! gReceivedSigInt);
    
        DeregisterOurServices();
        mDNS_Close(&gMDNSStorage);
    }
