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

namespace OpenXcom
{

class SavedGame;
class Mod;
class Language;

/**
 * Manages LLM interface trigger events and output routing.
 * Orchestrates when game state is serialized and where output goes.
 */
class LLMTriggerManager
{
public:
	/**
	 * Main hook called from game code when a trigger event occurs.
	 * @param triggerName Name of the trigger (e.g., "UFO_DETECTED", "BASE_OPENED")
	 * @param save The current saved game state
	 * @param mod The current mod/ruleset
	 * @param lang The current language for string translation
	 */
	static void onGeoscapeTrigger(const std::string& triggerName, SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Check if the LLM interface is enabled.
	 * @return true if enabled in options
	 */
	static bool isEnabled();

	/**
	 * Check if Geoscape serialization is enabled.
	 * @return true if enabled in options
	 */
	static bool isGeoscapeEnabled();

private:
	/**
	 * Write output to configured destinations (log and/or file).
	 * @param content The text content to output
	 * @param triggerName The trigger that generated this content
	 */
	static void writeOutput(const std::string& content, const std::string& triggerName);
};

}
