### ai_cli
[WIP]
- actually need to add a call to an AI api using a network library
- think about should we allow follow up questions or just a one time quick question
- if it's a quick question, add a system prompt telling the AI to be concise and not yap

cli tool written in C++ to quickly open a terminal or script and ask an ai something.
config file uses json; $XDG_CONFIG_HOME/ai_cli/config.json


uses nlohmann/json for json
uses premake as the build system
