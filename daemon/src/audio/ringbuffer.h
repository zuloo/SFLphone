/*
 *  Copyright (C) 2007 Savoir-Faire Linux inc.
 *  Author: Yan Morin <yan.morin@savoirfairelinux.com>
 *  Author: Laurielle Lea <laurielle.lea@savoirfairelinux.com>
 *  Author: Alexandre Savard <alexandre.savard@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __RING_BUFFER__
#define __RING_BUFFER__

#include "../call.h"

#include <fstream>

typedef std::map<std::string, int> ReadPointer;

static std::string default_id = "audiolayer_id";

class RingBuffer
{
    public:
        /**
         * Constructor
         * @param size  Size of the buffer to create
         */
        RingBuffer (int size, std::string call_id = default_id);

        /**
         * Destructor
         */
        ~RingBuffer();

        std::string getBufferId() {
            return buffer_id;
        }

        /**
         * Reset the counters to 0 for this read pointer
         */
        void flush (std::string call_id = default_id);

        void flushAll();

        /**
         * Get read pointer coresponding to this call
         */
        int getReadPointer (std::string call_id = default_id);

        /**
         * Get the whole readpointer list for this ringbuffer
         */
        ReadPointer* getReadPointerList() {
            return &_readpointer;
        }

        /**
         * Return the smalest readpointer. Usefull to evaluate if ringbuffer is full
         */
        int getSmallestReadPointer();

        /**
         * Move readpointer forward by pointer_value
         */
        void storeReadPointer (int pointer_value, std::string call_id = default_id);

        /**
         * Add a new readpointer for this ringbuffer
         */
        void createReadPointer (std::string call_id = default_id);

        /**
         * Remove a readpointer for this ringbuffer
         */
        void removeReadPointer (std::string call_id = default_id);

        /**
         * Test if readpointer coresponding to this call is still active
         */
        bool hasThisReadPointer (std::string call_id);

        int getNbReadPointer();

        /**
         * Write data in the ring buffer
         * @param buffer Data to copied
         * @param toCopy Number of bytes to copy
         */
        void Put (void* buffer, int toCopy);

        /**
         * To get how much space is available in the buffer to read in
         * @return int The available size
         */
        int AvailForGet (std::string call_id = default_id);

        /**
         * Get data in the ring buffer
         * @param buffer Data to copied
         * @param toCopy Number of bytes to copy
         * @return int Number of bytes copied
         */
        int Get (void* buffer, int toCopy, std::string call_id = default_id);

        /**
         * Discard data from the buffer
         * @param toDiscard Number of bytes to discard
         * @return int Number of bytes discarded
         */
        int Discard (int toDiscard, std::string call_id = default_id);

        /**
         * Total length of the ring buffer
         * @return int
         */
        int putLen();

        int getLen (std::string call_id = default_id);

        /**
         * Debug function print mEnd, mStart, mBufferSize
         */
        void debug();

    private:
        // Copy Constructor
        RingBuffer (const RingBuffer& rh);

        // Assignment operator
        RingBuffer& operator= (const RingBuffer& rh);

        /** Pointer on the first data */
        // int           mStart;
        /** Pointer on the last data */
        int           mEnd;
        /** Buffer size */
        int           mBufferSize;
        /** Data */
        unsigned char *mBuffer;

        ReadPointer   _readpointer;

        std::string buffer_id;

    public:

        friend class MainBufferTest;

        std::fstream *buffer_input_rec;
        std::fstream *buffer_output_rec;

        static int count_rb;

};


#endif /*  __RING_BUFFER__ */
