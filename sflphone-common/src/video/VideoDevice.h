/*
 *  Copyright (C) 2010 Savoir-Faire Linux inc.
 *  Author: Pierre-Luc Bacon <pierre-luc.bacon@savoirfairelinux.com>
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

#ifndef __SFL_VIDEO_DEVICE_H__
#define __SFL_VIDEO_DEVICE_H__

#include "FrameFormat.h"

#include <vector>
#include <string>
#include <tr1/memory>

namespace sfl {
/**
 * Video source types that might be supported.
 */
enum VideoSourceType {
	V4L, V4L2, DV1394, XIMAGE, IMAGE, TEST, NONE
};

/**
 * This exception is thrown when the user attempts to build an invalid video device.
 */
class InvalidVideoDeviceException: public std::runtime_error {
public:
	InvalidVideoDeviceException(const std::string& msg) :
		std::runtime_error(msg) {
	}
};

/**
 * Representation of a given gstreamer video device.
 * Mutable class.
 */
class VideoDevice {
public:
	/**
	 * @param type One of the available source type (eg. V4L2)
	 * @param formats The video formats that this device supports.
	 * @param device The device identifier (eg: /dev/video0).
	 * @param name The representative, and unique name of this device.
	 * @throws InvalidVideoDeviceException if no format is specified.
	 */
	VideoDevice(VideoSourceType type, std::vector<FrameFormat> formats,
			const std::string& device, const std::string& name) throw(InvalidVideoDeviceException);

	/**
	 * Copy constructor.
	 * @param other The object to copy from.
	 */
	VideoDevice(const VideoDevice& other);

	/**
	 * @return The video source type (eg. V4L2)
	 */
	VideoSourceType getType() const;

	/**
	 *@return The device identifier (eg: /dev/video0)
	 */
	std::string getDevice() const;

	/**
	 * @return The representative, and unique name of this device.
	 */
	std::string getName() const;

	/**
	 * @return the supported frame formats for this device.
	 */
	std::vector<FrameFormat> getSupportedFormats() const;

	/**
	 * @param format The format to use on this device.
	 */
	void setPreferredFormat(const FrameFormat& format);

	/**
	 * @return The preferred frame format for this device.
	 */
	FrameFormat getPreferredFormat() const;

	/**
	 * @return The preferred width.
	 */
	int getPreferredWidth();

	/**
	 * @return The preferred width.
	 */
	int getPreferredHeight();

	/**
	 * @return The preferred width.
	 */
	int getPreferredFrameRateNumerator();

	/**
	 * @return The preferred width.
	 */
	int getPreferredFrameRateDenominator();

	/**
	 * The string representation for this device.
	 */
	std::string toString() const {
		return name;
	}

protected:
	VideoSourceType type;
	std::vector<FrameFormat> formats;
	FrameFormat preferredFormat;
	std::string device;
	std::string name;
};

/**
 * Video devices are returned as std::shared_ptr in std::vector.
 */
typedef std::shared_ptr<VideoDevice> VideoDevicePtr;

}

#endif