========================================
OpenXcom LLM Interface - Quick Start
========================================

LOCATION: C:\OpenXcomSource\OpenXcom\bin\

HOW TO RUN:
-----------
1. Double-click: START_OPENXCOM_LLM.bat
   OR
2. Run openxcom.exe directly

WHERE TO FIND OUTPUT:
---------------------
The game will automatically export text when events happen.
Look in these files:

  llm_output.txt - Clean formatted output with timestamps
  openxcom.log   - Full game log (search for [LLM-INTERFACE])

WHAT GETS EXPORTED:
-------------------
Game state is exported when these events occur:

  ✓ BASE_OPENED - When you open a base screen
  ✓ UFO_DETECTED - When radar detects a UFO
  ✓ MISSION_DETECTED - When a UFO lands/crashes
  ✓ RESEARCH_COMPLETE - When research finishes
  ✓ PRODUCTION_COMPLETE - When manufacturing finishes
  ✓ TERROR_SITE - When a terror site appears
  ✓ MONTH_END - At the end of each month

Each export includes:
  - All bases (soldiers, crafts, facilities, storage)
  - Research progress and completed topics
  - Manufacturing queue
  - Financial status and regional satisfaction
  - UFO activity and locations
  - Soldier stats and assignments

USING WITH CLAUDE:
------------------
Method 1: Using the Viewer (Recommended)
  1. Double-click: OPEN_LLM_VIEWER.bat
  2. In the browser, select llm_output.txt
  3. Click any box to copy that section to clipboard
  4. Paste into your chat with Claude
  5. Enable "Auto-Refresh" to watch for game updates
  6. Ask for advice, analysis, or strategy help!

Method 2: Direct Copy-Paste
  1. Play the game - exports happen automatically
  2. Open llm_output.txt
  3. Copy the latest export (between the ======== lines)
  4. Paste into your chat with Claude
  5. Ask for advice, analysis, or strategy help!

LLM VIEWER FEATURES:
--------------------
The viewer (llm_viewer.html) provides:

  ✓ Click-to-copy boxes for each game section
  ✓ Auto-refresh mode - watches file for updates every 2 seconds
  ✓ Organized sections: Research, Finances, Skyranger, Barracks, Base Map
  ✓ Geoscape and Battlescape tabs (Battlescape coming in Phase 2)
  ✓ Retro green terminal styling
  ✓ Keyboard shortcuts: Ctrl+R to reload

Viewer Boxes (Geoscape Tab):
  - Research State - Current and available research
  - Financial State - Funds, income, regional satisfaction
  - Skyranger Contents - Crew and equipment loadout
  - Barracks Contents - All soldiers with stats and status
  - Base Map - Facility layout and legend
  - Base State - Personnel, capacity, defense
  - Manufacturing - Production queue
  - Inventory - Storage and alien containment

CONFIGURATION:
--------------
Edit options.cfg to change settings:

  llmInterfaceEnabled: true/false  - Master on/off switch
  llmOutputToLog: true/false       - Write to openxcom.log
  llmOutputToFile: true/false      - Write to text file
  llmMaxExports: 20                - Keep last N exports (circular buffer)
  llmOutputPath: "filename.txt"    - Output filename

The circular buffer prevents llm_output.txt from growing indefinitely.
When the file reaches llmMaxExports entries, the oldest export is
automatically removed. Default is 20 exports (plenty for recent history).
Set higher if you want more history, lower to save disk space.

TROUBLESHOOTING:
----------------
No output appearing?
  → Check options.cfg has llmInterfaceEnabled: true
  → Trigger an event (open a base, end a month, etc.)
  → Look in openxcom.log for [LLM-INTERFACE] messages

Game won't start?
  → Make sure all data folders are present (UFO, common, standard)
  → Check for missing DLL files
  → Try running as administrator

========================================
Built with Phase 1: Text Serialization
Ready for copy-paste to web chat!
========================================
