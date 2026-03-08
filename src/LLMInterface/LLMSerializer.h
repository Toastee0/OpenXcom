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
class Base;
class Language;

/**
 * Pure static class for serializing game state to text format for LLM interface.
 * All methods are pure functions with no side effects.
 */
class LLMSerializer
{
public:
	/**
	 * Main entry point - serialize complete geoscape state.
	 * @param save The saved game state
	 * @param mod The current mod/ruleset
	 * @param lang The current language
	 * @param triggerName The trigger that caused this serialization
	 * @return Complete text representation of geoscape state
	 */
	static std::string serializeGeoscapeState(SavedGame* save, Mod* mod, Language* lang, const std::string& triggerName);

	/**
	 * Serialize base summary (personnel, capacity metrics).
	 * Spec section 3.2.1
	 * @param base The base to serialize
	 * @param save The saved game (for context)
	 * @param mod The current mod
	 * @return Text representation of base state
	 */
	static std::string serializeBaseState(Base* base, SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize craft roster for a base.
	 * Spec section 3.2.2
	 * @param base The base whose crafts to serialize
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of craft roster
	 */
	static std::string serializeCraftRoster(Base* base, Mod* mod, Language* lang);

	/**
	 * Serialize base layout (6x6 facility grid).
	 * Spec section 3.2.3
	 * @param base The base whose layout to serialize
	 * @param save The saved game (for research ACL)
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of base layout
	 */
	static std::string serializeBaseLayout(Base* base, SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize research state (current and available research).
	 * Spec section 3.3
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of research state
	 */
	static std::string serializeResearchState(SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize manufacturing/production queue.
	 * Spec section 3.4
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of manufacturing state
	 */
	static std::string serializeManufacturing(SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize finances (funds, regional satisfaction).
	 * Spec section 3.5
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of financial state
	 */
	static std::string serializeFinances(SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize soldier roster.
	 * Spec section 3.6
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of all soldiers
	 */
	static std::string serializeSoldierRoster(SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize inventory (storage items, alien containment).
	 * Spec section 3.7
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of inventory
	 */
	static std::string serializeInventory(SavedGame* save, Mod* mod, Language* lang);

	/**
	 * Serialize globe state (UFO activity, craft missions).
	 * Spec section 3.8
	 * @param save The saved game
	 * @param mod The current mod
	 * @param lang The current language
	 * @return Text representation of globe state
	 */
	static std::string serializeGlobeState(SavedGame* save, Mod* mod, Language* lang);

private:
	// Helper methods
	static std::string formatMoney(int64_t amount);
	static std::string formatPercentage(int value);
	static std::string getRegionName(double lat, double lon, SavedGame* save, Language* lang);
};

}
