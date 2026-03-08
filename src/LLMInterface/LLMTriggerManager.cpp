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

	// Build context-specific output based on trigger type
	std::string output;

	if (triggerName == "RESEARCH_COMPLETE")
	{
		// Research completion: only show research + manufacturing (may have unlocked new items)
		std::ostringstream ss;
		ss << "========== RESEARCH COMPLETE ==========\n";
		ss << LLMSerializer::serializeResearchState(save, mod, lang);
		ss << "\n";
		ss << LLMSerializer::serializeManufacturing(save, mod, lang);
		output = ss.str();
	}
	else if (triggerName == "PRODUCTION_COMPLETE")
	{
		// Production completion: show manufacturing + inventory
		std::ostringstream ss;
		ss << "========== PRODUCTION COMPLETE ==========\n";
		ss << LLMSerializer::serializeManufacturing(save, mod, lang);
		ss << "\n";
		ss << LLMSerializer::serializeInventory(save, mod, lang);
		output = ss.str();
	}
	else if (triggerName == "TRANSFER_COMPLETE")
	{
		// Transfer arrival: show soldiers + inventory + crafts
		std::ostringstream ss;
		ss << "========== TRANSFER COMPLETE ==========\n";
		ss << LLMSerializer::serializeSoldierRoster(save, mod, lang);
		ss << "\n";
		ss << LLMSerializer::serializeInventory(save, mod, lang);
		output = ss.str();
	}
	else
	{
		// All other triggers: full geoscape state
		output = LLMSerializer::serializeGeoscapeState(save, mod, lang, triggerName);
	}

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
		const int MAX_EXPORTS = Options::llmMaxExports;  // Configurable max exports to keep
		std::string existingContent;

		// Read existing file content
		std::ifstream inFile(Options::llmOutputPath);
		if (inFile.is_open())
		{
			std::stringstream buffer;
			buffer << inFile.rdbuf();
			existingContent = buffer.str();
			inFile.close();
		}

		// Count existing exports by looking for the separator
		const std::string separator = "========================================";
		size_t exportCount = 0;
		size_t pos = 0;
		while ((pos = existingContent.find(separator, pos)) != std::string::npos)
		{
			exportCount++;
			pos += separator.length();
		}
		// Each export has 2 separators (start and end), so divide by 2
		exportCount = exportCount / 2;

		// If we have too many exports, trim the oldest ones
		if (exportCount >= MAX_EXPORTS)
		{
			// Find the position of the (exportCount - MAX_EXPORTS + 1)th export start
			size_t exportsToRemove = exportCount - MAX_EXPORTS + 1;
			pos = 0;
			for (size_t i = 0; i < exportsToRemove * 2; ++i)
			{
				pos = existingContent.find(separator, pos);
				if (pos == std::string::npos) break;
				pos += separator.length();
			}

			// Keep only content after this position
			if (pos != std::string::npos)
			{
				// Find the newline before the next separator to get a clean cut
				size_t cutPos = existingContent.find('\n', pos);
				if (cutPos != std::string::npos)
				{
					existingContent = existingContent.substr(cutPos + 1);
				}
			}
		}

		// Now write the trimmed content + new export
		std::ofstream outFile(Options::llmOutputPath, std::ios::trunc);
		if (outFile.is_open())
		{
			// Write existing content first (if any)
			if (!existingContent.empty())
			{
				outFile << existingContent;
			}

			// Get current timestamp
			std::time_t now = std::time(nullptr);
			char timestamp[64];
			std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

			// Write delimiter with timestamp and trigger
			outFile << "\n";
			outFile << "========================================\n";
			outFile << "TIMESTAMP: " << timestamp << "\n";
			outFile << "TRIGGER: " << triggerName << "\n";
			outFile << "========================================\n";
			outFile << content << "\n";
			outFile << "\n";

			outFile.close();

			Log(LOG_INFO) << "[LLM-INTERFACE] Exported to file (keeping last " << MAX_EXPORTS << " exports)";
		}
		else
		{
			Log(LOG_WARNING) << "[LLM-INTERFACE] Failed to open output file: " << Options::llmOutputPath;
		}
	}
}

}
