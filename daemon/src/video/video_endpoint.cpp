/*
 *  Copyright (C) 2004, 2005, 2006, 2009, 2008, 2009, 2010, 2011 Savoir-Faire Linux Inc.
 *  Author: Tristan Matthews <tristan.matthews@savoirfairelinux.com>
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
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include "video_endpoint.h"

#include <iostream>
#include <tr1/memory>
#include <sstream>
#include <map>
#include "libav_utils.h"

namespace sfl_video {

/* anonymous namespace */
namespace {
int FAKE_BITRATE()
{
    return 1000;
}

int getBitRate(const std::string &codec)
{
    return FAKE_BITRATE();
}
} // end anonymous namespace

std::vector<std::string> getVideoCodecList()
{
	return libav_utils::getVideoCodecList();
}

std::vector<std::string> getCodecSpecifications(const std::string &codec)
{
    std::vector<std::string> v(1, codec);
    std::stringstream ss;

    // Add the bit rate
    ss << getBitRate(codec);
    v.push_back(ss.str());
    ss.str("");

    return v;
}

} // end namespace sfl_video