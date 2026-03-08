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
#include "LLMSerializer.h"
#include "LLMConfig.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/Craft.h"
#include "../Savegame/CraftWeapon.h"
#include "../Savegame/Vehicle.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Country.h"
#include "../Savegame/Region.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/ResearchProject.h"
#include "../Savegame/Production.h"
#include "../Savegame/Transfer.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleCraftWeapon.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleUfo.h"
#include "../Mod/Armor.h"
#include "../Engine/Language.h"
#include <sstream>
#include <iomanip>
#include <set>

namespace OpenXcom
{

/**
 * Main entry point for serializing geoscape state.
 */
std::string LLMSerializer::serializeGeoscapeState(SavedGame* save, Mod* mod, Language* lang, const std::string& triggerName)
{
	std::ostringstream ss;

	// Header with trigger information
	ss << "========== GEOSCAPE STATE ==========\n";
	ss << "TRIGGER: " << triggerName << "\n";

	// Format date as "Month Day, Year"
	GameTime* time = save->getTime();
	int day = time->getDay();
	std::string daySuffix;
	if (day == 1 || day == 21 || day == 31) daySuffix = "st";
	else if (day == 2 || day == 22) daySuffix = "nd";
	else if (day == 3 || day == 23) daySuffix = "rd";
	else daySuffix = "th";

	ss << "DATE: " << lang->getString(time->getMonthString()) << " " << day << daySuffix << ", " << time->getYear() << "\n";
	ss << "\n";

	// Finances overview
	ss << serializeFinances(save, mod, lang);
	ss << "\n";

	// All bases
	std::vector<Base*>* bases = save->getBases();
	for (size_t i = 0; i < bases->size(); ++i)
	{
		Base* base = (*bases)[i];
		ss << serializeBaseState(base, save, mod, lang);
		ss << "\n";
		ss << serializeBaseLayout(base, save, mod, lang);
		ss << "\n";
		ss << serializeCraftRoster(base, mod, lang);
		ss << "\n";
	}

	// Research state
	ss << serializeResearchState(save, mod, lang);
	ss << "\n";

	// Manufacturing state
	ss << serializeManufacturing(save, mod, lang);
	ss << "\n";

	// Soldier roster
	ss << serializeSoldierRoster(save, mod, lang);
	ss << "\n";

	// Inventory
	ss << serializeInventory(save, mod, lang);
	ss << "\n";

	// Globe state
	ss << serializeGlobeState(save, mod, lang);

	return ss.str();
}

/**
 * Serialize base summary (spec 3.2.1).
 */
std::string LLMSerializer::serializeBaseState(Base* base, SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;

	ss << "BASE: " << base->getName() << "\n";
	ss << "LOCATION: " << std::fixed << std::setprecision(2)
	   << base->getLatitude() << ", " << base->getLongitude();

	// Try to get region name
	std::string region = getRegionName(base->getLatitude(), base->getLongitude(), save, lang);
	if (!region.empty())
	{
		ss << " (" << region << ")";
	}
	ss << "\n";

	// Personnel
	ss << "\nPERSONNEL:\n";

	int totalSoldiers = base->getTotalSoldiers();
	int availableSoldiers = base->getAvailableSoldiers();
	int assignedSoldiers = totalSoldiers - availableSoldiers;
	ss << "  Soldiers: " << totalSoldiers << "/" << base->getAvailableQuarters()
	   << " (" << assignedSoldiers << " on crafts, " << availableSoldiers << " available for base defense)\n";

	int totalScientists = base->getScientists();
	int allocatedScientists = base->getAllocatedScientists();
	ss << "  Scientists: " << totalScientists << " (" << allocatedScientists << " allocated)\n";

	int totalEngineers = base->getEngineers();
	int allocatedEngineers = base->getAllocatedEngineers();
	ss << "  Engineers: " << totalEngineers << " (" << allocatedEngineers << " allocated)\n";

	// Capacity
	ss << "\nCAPACITY:\n";
	ss << "  Living Quarters: " << base->getUsedQuarters() << "/" << base->getAvailableQuarters() << "\n";
	ss << "  Storage: " << std::fixed << std::setprecision(1)
	   << base->getUsedStores() << "/" << base->getAvailableStores() << " units\n";
	ss << "  Laboratory Space: " << base->getUsedLaboratories() << "/" << base->getAvailableLaboratories() << "\n";
	ss << "  Workshop Space: " << base->getUsedWorkshops() << "/" << base->getAvailableWorkshops() << "\n";
	ss << "  Hangars: " << base->getUsedHangars() << "/" << base->getAvailableHangars() << "\n";

	// Defense
	ss << "\nDEFENSE:\n";
	ss << "  Defense Value: " << base->getDefenseValue() << "\n";
	ss << "  Short Range Detection: " << base->getShortRangeDetection() << "\n";
	ss << "  Long Range Detection: " << base->getLongRangeDetection() << "\n";

	return ss.str();
}

/**
 * Serialize finances (spec 3.5).
 */
std::string LLMSerializer::serializeFinances(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;

	ss << "FINANCES:\n";
	ss << "Funds: " << formatMoney(save->getFunds()) << "\n";

	// Calculate monthly income/expenses
	int64_t monthlyIncome = 0;
	int64_t monthlyExpenses = 0;

	// Country funding
	std::vector<Country*>* countries = save->getCountries();
	for (size_t i = 0; i < countries->size(); ++i)
	{
		Country* country = (*countries)[i];
		monthlyIncome += country->getFunding().back();
	}

	// Base maintenance
	std::vector<Base*>* bases = save->getBases();
	for (size_t i = 0; i < bases->size(); ++i)
	{
		monthlyExpenses += (*bases)[i]->getMonthlyMaintenace();
	}

	ss << "Monthly Income: " << formatMoney(monthlyIncome) << "\n";
	ss << "Monthly Expenses: " << formatMoney(monthlyExpenses) << "\n";
	ss << "Monthly Balance: " << formatMoney(monthlyIncome - monthlyExpenses) << "\n";

	// Regional satisfaction
	ss << "\nREGIONAL FUNDING:\n";
	for (size_t i = 0; i < countries->size(); ++i)
	{
		Country* country = (*countries)[i];
		std::string name = lang->getString(country->getRules()->getType());
		int satisfaction = country->getSatisfaction();
		int64_t funding = country->getFunding().back();

		// Map satisfaction level to label (0=Alien Pact, 1=Unhappy, 2=Satisfied, 3=Happy)
		std::string satisfactionLabel;
		switch (satisfaction)
		{
			case 0: satisfactionLabel = "Alien Pact"; break;
			case 1: satisfactionLabel = "Unhappy"; break;
			case 2: satisfactionLabel = "Satisfied"; break;
			case 3: satisfactionLabel = "Happy"; break;
			default: satisfactionLabel = "Unknown"; break;
		}

		ss << "  " << name << ": " << formatMoney(funding)
		   << " (Satisfaction: " << satisfactionLabel << ")\n";
	}

	return ss.str();
}

/**
 * Format money amount.
 */
std::string LLMSerializer::formatMoney(int64_t amount)
{
	std::ostringstream ss;
	ss << "$" << std::setprecision(0) << std::fixed << amount;
	return ss.str();
}

/**
 * Format percentage.
 */
std::string LLMSerializer::formatPercentage(int value)
{
	std::ostringstream ss;
	ss << value << "%";
	return ss.str();
}

/**
 * Get region name for coordinates.
 */
std::string LLMSerializer::getRegionName(double lat, double lon, SavedGame* save, Language* lang)
{
	std::vector<Region*>* regions = save->getRegions();
	for (size_t i = 0; i < regions->size(); ++i)
	{
		Region* region = (*regions)[i];
		if (region->getRules()->insideRegion(lon, lat))
		{
			return lang->getString(region->getRules()->getType());
		}
	}
	return "";
}

/**
 * Placeholder implementations for remaining serializers.
 * These will be implemented in subsequent steps.
 */

std::string LLMSerializer::serializeCraftRoster(Base* base, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<Craft*>* crafts = base->getCrafts();

	if (crafts->empty())
	{
		ss << "CRAFT ROSTER: None\n";
		return ss.str();
	}

	ss << "CRAFT ROSTER:\n";
	for (size_t i = 0; i < crafts->size(); ++i)
	{
		Craft* craft = (*crafts)[i];
		RuleCraft* rules = craft->getRules();

		ss << "  " << craft->getName(lang) << " (" << lang->getString(rules->getType()) << ")\n";
		ss << "    Status: " << lang->getString(craft->getStatus()) << "\n";
		ss << "    Fuel: " << craft->getFuelPercentage() << "%\n";
		ss << "    Damage: " << craft->getDamagePercentage() << "%\n";
		ss << "    Speed: " << rules->getMaxSpeed() << "\n";

		// Weapons
		std::vector<CraftWeapon*>* weapons = craft->getWeapons();
		ss << "    Weapons: ";
		bool hasWeapons = false;
		for (size_t w = 0; w < weapons->size(); ++w)
		{
			CraftWeapon* weapon = (*weapons)[w];
			if (weapon != 0)
			{
				if (hasWeapons) ss << ", ";
				ss << lang->getString(weapon->getRules()->getType());
				ss << " (" << weapon->getAmmo() << "/" << weapon->getRules()->getAmmoMax() << ")";
				hasWeapons = true;
			}
		}
		if (!hasWeapons) ss << "None";
		ss << "\n";

		// Crew
		ss << "    Crew: " << craft->getNumSoldiers() << "/" << rules->getSoldiers() << " soldiers";
		if (rules->getVehicles() > 0)
		{
			ss << ", " << craft->getNumVehicles() << "/" << rules->getVehicles() << " vehicles";
		}
		ss << "\n";

		// Equipment loaded on craft
		ItemContainer* items = craft->getItems();
		std::map<std::string, int>* contents = items->getContents();
		if (!contents->empty())
		{
			ss << "    Equipment: ";
			bool first = true;
			for (std::map<std::string, int>::const_iterator it = contents->begin(); it != contents->end(); ++it)
			{
				if (!first) ss << ", ";
				ss << it->second << "x " << lang->getString(it->first);
				first = false;
			}
			ss << "\n";
		}

		// Vehicles (tanks/HWPs) on craft
		std::vector<Vehicle*>* vehicles = craft->getVehicles();
		if (!vehicles->empty())
		{
			ss << "    Vehicles: ";
			for (size_t v = 0; v < vehicles->size(); ++v)
			{
				if (v > 0) ss << ", ";
				Vehicle* vehicle = (*vehicles)[v];
				ss << lang->getString(vehicle->getRules()->getType());
			}
			ss << "\n";
		}
	}

	return ss.str();
}

std::string LLMSerializer::serializeBaseLayout(Base* base, SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<BaseFacility*>* facilities = base->getFacilities();

	// Create 6x6 grid
	std::string grid[6][6];
	for (int y = 0; y < 6; ++y)
	{
		for (int x = 0; x < 6; ++x)
		{
			grid[y][x] = "00";  // Empty
		}
	}

	// Fill grid with facilities
	int hangarCount = 0;
	for (size_t i = 0; i < facilities->size(); ++i)
	{
		BaseFacility* facility = (*facilities)[i];
		int x = facility->getX();
		int y = facility->getY();
		std::string facilityType = facility->getRules()->getType();
		std::string code = LLMConfig::getFacilityCode(facilityType);

		// Assign unique hangar IDs (H1, H2, H3, H4)
		if (code == "H1" || code == "H2" || code == "H3" || code == "H4")
		{
			hangarCount++;
			code = "H" + std::to_string(hangarCount);
		}

		// Handle 2x2 facilities (hangars)
		RuleBaseFacility* rules = facility->getRules();
		int size = rules->getSize();

		if (size == 1)
		{
			grid[y][x] = code;
		}
		else
		{
			// 2x2 facility
			for (int dy = 0; dy < 2; ++dy)
			{
				for (int dx = 0; dx < 2; ++dx)
				{
					if (x + dx < 6 && y + dy < 6)
					{
						grid[y + dy][x + dx] = code;
					}
				}
			}
		}
	}

	// Output grid
	ss << "BASE LAYOUT:\n";
	for (int y = 0; y < 6; ++y)
	{
		for (int x = 0; x < 6; ++x)
		{
			if (x > 0) ss << ",";
			ss << grid[y][x];
		}
		ss << "\n";
	}

	// Output legend - only for facilities present in this base
	ss << "\nLEGEND:\n";
	ss << "00 = Empty\n";

	// Build set of facility codes actually present in grid
	std::set<std::string> presentCodes;
	for (int y = 0; y < 6; ++y)
	{
		for (int x = 0; x < 6; ++x)
		{
			if (grid[y][x] != "00")
			{
				presentCodes.insert(grid[y][x]);
			}
		}
	}

	// Get discovered research for ACL check
	const std::vector<const RuleResearch*>& discoveredResearch = save->getDiscoveredResearch();
	std::set<std::string> researchSet;
	for (size_t i = 0; i < discoveredResearch.size(); ++i)
	{
		researchSet.insert(discoveredResearch[i]->getName());
	}

	// Build reverse map (code -> facility type) for facilities in base OR unlocked via research
	std::map<std::string, std::string> codeToName;

	// First, add facilities actually in the base
	for (size_t i = 0; i < facilities->size(); ++i)
	{
		BaseFacility* facility = (*facilities)[i];
		std::string facilityType = facility->getRules()->getType();
		std::string code = LLMConfig::getFacilityCode(facilityType);

		// For hangars, we assigned H1, H2, H3, H4 dynamically, so map them all to STR_HANGAR
		if (code.length() == 2 && code[0] == 'H' && code[1] >= '1' && code[1] <= '4')
		{
			// Use the generic hangar name for all hangar codes
			for (std::set<std::string>::const_iterator it = presentCodes.begin(); it != presentCodes.end(); ++it)
			{
				if (it->length() == 2 && (*it)[0] == 'H' && (*it)[1] >= '1' && (*it)[1] <= '4')
				{
					if (codeToName.find(*it) == codeToName.end())
					{
						std::ostringstream hangarName;
						hangarName << lang->getString("STR_HANGAR") << " " << (*it)[1];
						codeToName[*it] = hangarName.str();
					}
				}
			}
		}
		else
		{
			codeToName[code] = lang->getString(facilityType);
		}
	}

	// Second, add facilities unlocked via research (per spec: ACL includes researched facilities)
	const std::vector<std::string>& allFacilityTypes = mod->getBaseFacilitiesList();
	for (size_t i = 0; i < allFacilityTypes.size(); ++i)
	{
		RuleBaseFacility* facilityRule = mod->getBaseFacility(allFacilityTypes[i]);
		if (!facilityRule) continue;

		std::string code = LLMConfig::getFacilityCode(allFacilityTypes[i]);

		// Skip if already in legend from built facilities
		if (codeToName.find(code) != codeToName.end()) continue;

		// Check if requirements are met
		const std::vector<std::string>& requirements = facilityRule->getRequirements();
		bool unlocked = true;

		for (size_t j = 0; j < requirements.size(); ++j)
		{
			if (researchSet.find(requirements[j]) == researchSet.end())
			{
				unlocked = false;
				break;
			}
		}

		// If unlocked, add to legend
		if (unlocked)
		{
			codeToName[code] = lang->getString(allFacilityTypes[i]);
		}
	}

	// Output legend entries for present codes only
	for (std::set<std::string>::const_iterator it = presentCodes.begin(); it != presentCodes.end(); ++it)
	{
		if (codeToName.find(*it) != codeToName.end())
		{
			ss << *it << " = " << codeToName[*it] << "\n";
		}
	}

	return ss.str();
}

std::string LLMSerializer::serializeResearchState(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<Base*>* bases = save->getBases();

	ss << "RESEARCH:\n";

	// Current research projects
	bool hasResearch = false;
	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<ResearchProject*> projects = base->getResearch();

		if (projects.empty())
			continue;

		if (!hasResearch)
		{
			ss << "\nCurrent Projects:\n";
			hasResearch = true;
		}

		ss << "  Base: " << base->getName() << "\n";
		for (size_t i = 0; i < projects.size(); ++i)
		{
			ResearchProject* project = projects[i];
			const RuleResearch* rules = project->getRules();

			ss << "    " << lang->getString(rules->getName()) << "\n";
			ss << "      Scientists: " << project->getAssigned() << "\n";
			ss << "      Cost: " << rules->getCost() << " per day\n";
			ss << "      Progress: " << project->getSpent() << "/" << project->getCost() << "\n";
		}
	}

	if (!hasResearch)
	{
		ss << "  No active research projects\n";
	}

	// Completed research count
	ss << "\nCompleted Research: " << save->getDiscoveredResearch().size() << " topics\n";

	// Available research topics (across all bases)
	ss << "\nAvailable Research Topics:\n";
	bool hasAvailable = false;
	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<RuleResearch*> availableProjects;
		save->getAvailableResearchProjects(availableProjects, mod, base);

		if (!availableProjects.empty() && !hasAvailable)
		{
			hasAvailable = true;
		}

		// Show first 15 available projects (to avoid spam)
		for (size_t i = 0; i < availableProjects.size() && i < 15; ++i)
		{
			RuleResearch* research = availableProjects[i];
			ss << "  " << lang->getString(research->getName()) << " (cost: " << research->getCost() << ")\n";
		}

		// Only show from first base with available research
		if (!availableProjects.empty())
		{
			if (availableProjects.size() > 15)
			{
				ss << "  ... and " << (availableProjects.size() - 15) << " more topics\n";
			}
			break;
		}
	}

	if (!hasAvailable)
	{
		ss << "  None available\n";
	}

	return ss.str();
}

std::string LLMSerializer::serializeManufacturing(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<Base*>* bases = save->getBases();

	ss << "MANUFACTURING:\n";

	bool hasProduction = false;
	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<Production*> productions = base->getProductions();

		if (productions.empty())
			continue;

		if (!hasProduction)
		{
			hasProduction = true;
		}

		ss << "\nBase: " << base->getName() << "\n";
		for (size_t i = 0; i < productions.size(); ++i)
		{
			Production* production = productions[i];
			const RuleManufacture* rules = production->getRules();

			ss << "  " << lang->getString(rules->getName()) << "\n";
			ss << "    Engineers: " << production->getAssignedEngineers() << "\n";
			ss << "    Amount: " << production->getAmountProduced() << "/" << production->getAmountTotal() << "\n";
			ss << "    Time per unit: " << rules->getManufactureTime() << " hours\n";
			ss << "    Cost per unit: " << formatMoney(rules->getManufactureCost()) << "\n";

			// Show required materials
			const std::map<std::string, int>& required = rules->getRequiredItems();
			if (!required.empty())
			{
				ss << "    Materials: ";
				bool first = true;
				for (std::map<std::string, int>::const_iterator it = required.begin(); it != required.end(); ++it)
				{
					if (!first) ss << ", ";
					ss << it->second << "x " << lang->getString(it->first);
					first = false;
				}
				ss << "\n";
			}
		}
	}

	if (!hasProduction)
	{
		ss << "  No active production\n";
	}

	// Available manufacturing items (across all bases)
	ss << "\nAvailable Manufacturing:\n";
	bool hasAvailableManufacture = false;
	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<RuleManufacture*> availableManufacture;
		save->getAvailableProductions(availableManufacture, mod, base);

		if (!availableManufacture.empty() && !hasAvailableManufacture)
		{
			hasAvailableManufacture = true;
		}

		// Show first 15 available items (to avoid spam)
		for (size_t i = 0; i < availableManufacture.size() && i < 15; ++i)
		{
			RuleManufacture* item = availableManufacture[i];
			ss << "  " << lang->getString(item->getName())
			   << " (" << item->getManufactureTime() << " hrs, "
			   << formatMoney(item->getManufactureCost()) << ")\n";
		}

		// Only show from first base with available manufacturing
		if (!availableManufacture.empty())
		{
			if (availableManufacture.size() > 15)
			{
				ss << "  ... and " << (availableManufacture.size() - 15) << " more items\n";
			}
			break;
		}
	}

	if (!hasAvailableManufacture)
	{
		ss << "  None available\n";
	}

	return ss.str();
}

std::string LLMSerializer::serializeSoldierRoster(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<Base*>* bases = save->getBases();

	ss << "SOLDIER ROSTER:\n";

	int totalSoldiers = 0;
	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<Soldier*>* soldiers = base->getSoldiers();

		if (soldiers->empty())
			continue;

		ss << "\nBase: " << base->getName() << " (Total soldiers in base: " << soldiers->size() << ")\n";

		for (size_t i = 0; i < soldiers->size(); ++i)
		{
			Soldier* soldier = (*soldiers)[i];
			UnitStats* stats = soldier->getCurrentStats();

			ss << "  " << soldier->getName() << " (" << lang->getString(soldier->getRankString()) << ")\n";
			ss << "    TUs: " << stats->tu << "  Stamina: " << stats->stamina
			   << "  Health: " << stats->health << "  Bravery: " << stats->bravery << "\n";
			ss << "    Reactions: " << stats->reactions << "  Firing: " << stats->firing
			   << "  Throwing: " << stats->throwing << "  Strength: " << stats->strength << "\n";
			ss << "    Psi Strength: " << stats->psiStrength << "  Psi Skill: " << stats->psiSkill << "\n";
			ss << "    Missions: " << soldier->getMissions() << "  Kills: " << soldier->getKills() << "\n";

			// Status
			if (soldier->getCraft() != 0)
			{
				ss << "    Status: Assigned to " << soldier->getCraft()->getName(lang) << "\n";
			}
			else if (soldier->getWoundRecovery() > 0)
			{
				ss << "    Status: Wounded (" << soldier->getWoundRecovery() << " days recovery)\n";
			}
			else
			{
				ss << "    Status: Ready\n";
			}

			// Armor
			if (soldier->getArmor() != 0)
			{
				ss << "    Armour: " << lang->getString(soldier->getArmor()->getType()) << "\n";
			}

			totalSoldiers++;
		}

		// Show soldiers in transfer to this base
		std::vector<Transfer*>* transfers = base->getTransfers();
		int transferCount = 0;
		for (size_t t = 0; t < transfers->size(); ++t)
		{
			Transfer* transfer = (*transfers)[t];
			if (transfer->getType() == TRANSFER_SOLDIER)
			{
				if (transferCount == 0)
				{
					ss << "\n  Soldiers In Transfer:\n";
				}
				Soldier* soldier = transfer->getSoldier();
				if (soldier != 0)
				{
					int hoursRemaining = transfer->getHours();
					int daysRemaining = (hoursRemaining + 23) / 24;  // Round up to days
					ss << "    " << soldier->getName() << " (" << lang->getString(soldier->getRankString())
					   << ") - Arrives in " << daysRemaining << " day" << (daysRemaining != 1 ? "s" : "") << "\n";
					transferCount++;
				}
			}
		}
	}

	if (totalSoldiers == 0)
	{
		ss << "  None\n";
	}

	return ss.str();
}

std::string LLMSerializer::serializeInventory(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;
	std::vector<Base*>* bases = save->getBases();

	ss << "INVENTORY:\n";

	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		ss << "\nBase: " << base->getName() << "\n";

		// Storage items
		ItemContainer* items = base->getStorageItems();
		std::map<std::string, int>* contents = items->getContents();

		if (contents->empty())
		{
			ss << "  Storage: Empty\n";
		}
		else
		{
			ss << "  Storage:\n";
			int itemCount = 0;
			for (std::map<std::string, int>::const_iterator it = contents->begin(); it != contents->end(); ++it)
			{
				if (itemCount >= 20)
				{
					ss << "    ... and " << (contents->size() - 20) << " more items\n";
					break;
				}

				std::string itemName = lang->getString(it->first);
				ss << "    " << it->second << "x " << itemName << "\n";
				itemCount++;
			}
		}

		// Alien containment
		int aliens = base->getUsedContainment();
		int capacity = base->getAvailableContainment();
		if (capacity > 0)
		{
			ss << "  Alien Containment: " << aliens << "/" << capacity << "\n";
		}
	}

	return ss.str();
}

std::string LLMSerializer::serializeGlobeState(SavedGame* save, Mod* mod, Language* lang)
{
	std::ostringstream ss;

	// UFO activity
	std::vector<Ufo*>* ufos = save->getUfos();
	ss << "UFO ACTIVITY:\n";

	if (ufos->empty())
	{
		ss << "  No UFOs detected\n";
	}
	else
	{
		for (size_t i = 0; i < ufos->size(); ++i)
		{
			Ufo* ufo = (*ufos)[i];
			if (!ufo->getDetected())
				continue;

			ss << "  UFO #" << ufo->getId() << " (" << lang->getString(ufo->getRules()->getType()) << ")\n";
			ss << "    Location: " << std::fixed << std::setprecision(2)
			   << ufo->getLatitude() << ", " << ufo->getLongitude() << "\n";
			ss << "    Altitude: " << ufo->getAltitude() << "\n";

			// Status
			std::string status;
			switch (ufo->getStatus())
			{
				case Ufo::FLYING: status = "Flying"; break;
				case Ufo::LANDED: status = "Landed"; break;
				case Ufo::CRASHED: status = "Crashed"; break;
				case Ufo::DESTROYED: status = "Destroyed"; break;
				default: status = "Unknown"; break;
			}
			ss << "    Status: " << status << "\n";
		}
	}

	// Craft missions
	ss << "\nCRAFT MISSIONS:\n";
	std::vector<Base*>* bases = save->getBases();
	bool hasMissions = false;

	for (size_t b = 0; b < bases->size(); ++b)
	{
		Base* base = (*bases)[b];
		std::vector<Craft*>* crafts = base->getCrafts();

		for (size_t c = 0; c < crafts->size(); ++c)
		{
			Craft* craft = (*crafts)[c];
			std::string status = craft->getStatus();

			// Only show crafts on missions (not at base)
			if (status != "STR_READY")
			{
				hasMissions = true;
				ss << "  " << craft->getName(lang) << " (Base: " << base->getName() << ")\n";
				ss << "    Status: " << lang->getString(status) << "\n";
				ss << "    Location: " << std::fixed << std::setprecision(2)
				   << craft->getLatitude() << ", " << craft->getLongitude() << "\n";
				ss << "    Fuel: " << craft->getFuelPercentage() << "%\n";
			}
		}
	}

	if (!hasMissions)
	{
		ss << "  No active missions\n";
	}

	return ss.str();
}

}
