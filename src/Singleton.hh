/* Original version Copyright (C) Scott Bilas, 2000.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Scott Bilas, 2000"
 */
#ifndef GABBER_SINGLETON_H
#define GABBER_SINGLETON_H

#include <assert.h>

namespace Gabber {

/** 
 * Template class for creating single-instance global classes.
 */
template <typename T> class Singleton
{
protected:

    static T* ms_Singleton;

public:
    Singleton( void )
    {
        assert( !ms_Singleton );
        long offset = (long)(T*)1 - (long)(Singleton <T>*)(T*)1;
        ms_Singleton = (T*)((long)this + offset);
    }
    ~Singleton( void )
    {  assert( ms_Singleton );  ms_Singleton = 0;  }
    /**
     * Access the singleton
     * This is how all access to the singleton should be made
     */
    static T& getSingleton( void )
    {  assert( ms_Singleton );  return ( *ms_Singleton );  }
}; // class Singleton

template <typename T> T* Singleton <T>::ms_Singleton = 0;

}; // namespace Gabber

#endif // GABBER_SINGLETON_H
