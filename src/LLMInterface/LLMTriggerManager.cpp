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
#include "LLMTriggerManager.h"
#include "LLMSerializer.h"
#include "../Engine/Options.h"
#include "../Engine/Logger.h"
#include "../Engine/Language.h"
#include "../Savegame/SavedGame.h"
#include "../Mod/Mod.h"
#include <fstream>
#include <sstream>
#include <ctime>

namespace OpenXcom
{

/**
 * Check if the LLM interface is enabled globally.
 */
bool LLMTriggerManager::isEnabled()
{
	return Options::llmInterfaceEnabled;
}

/**
 * Check if Geoscape serialization is enabled.
 */
bool LLMTriggerManager::isGeoscapeEnabled()
{
	return Options::llmGeoscapeEnabled;
}

/**
 * Main trigger handler - called when game events occur.
 */
void LLMTriggerManager::onGeoscapeTrigger(const std::string& triggerName, SavedGame* save, Mod* mod, Language* lang)
{
	// Check if interface is enabled
	if (!isEnabled() || !isGeoscapeEnabled())
	{
		return;
	}

	// Validate inputs
	if (!save || !mod || !lang)
	{
		Log(LOG_WARNING) << "[LLM-INTERFACE] Trigger '" << triggerName << "' called with null save, mod, or lang";
		return;
	}

	// Serialize the current game state
	std::string output = LLMSerializer::serializeGeoscapeState(save, mod, lang, triggerName);

	// Write to configured destinations
	writeOutput(output, triggerName);
}

/**
 * Write output to log and/or file based on configuration.
 */
void LLMTriggerManager::writeOutput(const std::string& content, const std::string& triggerName)
{
	// Output to log if enabled
	if (Options::llmOutputToLog)
	{
		// Split content into lines and log each one
		std::istringstream stream(content);
		std::string line;
		Log(LOG_INFO) << "[LLM-INTERFACE] ========== TRIGGER: " << triggerName << " ==========";
		while (std::getline(stream, line))
		{
			Log(LOG_INFO) << "[LLM-INTERFACE] " << line;
		}
		Log(LOG_INFO) << "[LLM-INTERFACE] ========================================";
	}

	// Output to file if enabled
	if (Options::llmOutputToFile)
	{
		std::ofstream file;
		file.open(Options::llmOutputPath, std::ios::app);

		if (file.is_open())
		{
			// Get current timestamp
			std::time_t now = std::time(nullptr);
			char timestamp[64];
			std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

			// Write delimiter with timestamp and trigger
			file << "\n";
			file << "========================================\n";
			file << "TIMESTAMP: " << timestamp << "\n";
			file << "TRIGGER: " << triggerName << "\n";
			file << "========================================\n";
			file << content << "\n";
			file << "\n";

			file.close();
		}
		else
		{
			Log(LOG_WARNING) << "[LLM-INTERFACE] Failed to open output file: " << Options::llmOutputPath;
		}
	}
}

}
