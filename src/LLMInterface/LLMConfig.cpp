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
#include "LLMConfig.h"

namespace OpenXcom
{

std::map<std::string, std::string> LLMConfig::_facilityCodeMap;

/**
 * Initialize the facility code map with standard facility abbreviations.
 */
void LLMConfig::initFacilityCodeMap()
{
	if (!_facilityCodeMap.empty())
		return;

	// Standard X-COM facility types to abbreviations
	_facilityCodeMap["STR_ACCESS_LIFT"] = "AL";
	_facilityCodeMap["STR_LIVING_QUARTERS"] = "LQ";
	_facilityCodeMap["STR_LABORATORY"] = "LB";
	_facilityCodeMap["STR_WORKSHOP"] = "WS";
	_facilityCodeMap["STR_SMALL_RADAR_SYSTEM"] = "SR";
	_facilityCodeMap["STR_LARGE_RADAR_SYSTEM"] = "LR";
	_facilityCodeMap["STR_MISSILE_DEFENSES"] = "MD";
	_facilityCodeMap["STR_GENERAL_STORES"] = "GS";
	_facilityCodeMap["STR_ALIEN_CONTAINMENT"] = "AC";
	_facilityCodeMap["STR_LASER_DEFENSES"] = "LD";
	_facilityCodeMap["STR_PLASMA_DEFENSES"] = "PD";
	_facilityCodeMap["STR_FUSION_BALL_DEFENSES"] = "FD";
	_facilityCodeMap["STR_GRAV_SHIELD"] = "GV";
	_facilityCodeMap["STR_MIND_SHIELD"] = "MS";
	_facilityCodeMap["STR_PSIONIC_LABORATORY"] = "PL";
	_facilityCodeMap["STR_HYPER_WAVE_DECODER"] = "HW";

	// Hangars (2x2 facilities)
	_facilityCodeMap["STR_HANGAR"] = "H1";
	_facilityCodeMap["STR_HANGAR_2"] = "H2";
	_facilityCodeMap["STR_HANGAR_3"] = "H3";
	_facilityCodeMap["STR_HANGAR_4"] = "H4";
}

/**
 * Get the facility code for a facility type.
 */
std::string LLMConfig::getFacilityCode(const std::string& facilityType)
{
	initFacilityCodeMap();

	auto it = _facilityCodeMap.find(facilityType);
	if (it != _facilityCodeMap.end())
	{
		return it->second;
	}
	return "??";  // Unknown facility type
}

/**
 * Get the full facility code map.
 */
const std::map<std::string, std::string>& LLMConfig::getFacilityCodeMap()
{
	initFacilityCodeMap();
	return _facilityCodeMap;
}

}
