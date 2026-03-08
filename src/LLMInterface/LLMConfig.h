#pragma once
/*
 * Copyright 2010-2024 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <map>

namespace OpenXcom
{

/**
 * Configuration constants and mappings for LLM interface system.
 * Provides facility code mappings and other configuration data.
 */
class LLMConfig
{
public:
	/**
	 * Get the 2-character abbreviation for a facility type.
	 * @param facilityType The facility type string (e.g., "STR_ACCESS_LIFT")
	 * @return The abbreviation (e.g., "AL") or "??" if unknown
	 */
	static std::string getFacilityCode(const std::string& facilityType);

	/**
	 * Get the full legend of facility codes and their meanings.
	 * @return A map of facility types to their abbreviations
	 */
	static const std::map<std::string, std::string>& getFacilityCodeMap();

private:
	// Facility type to abbreviation mappings
	static std::map<std::string, std::string> _facilityCodeMap;

	// Initialize the facility code map
	static void initFacilityCodeMap();
};

}
