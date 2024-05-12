#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Character.h"
#include "Quest.h"
#include "StringConvert.h"
#include "Log.h"

namespace PrintTool
{
	
void PrintCharacterData(const std::filesystem::path& path, std::ofstream& out)
{
    Character characterData;
    if (!characterData.ReadFromFile(path))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data from \"%\".", path.string()));
	
	out << "// " << path.string() << std::endl;
	
	out << "Header\n{\n";
	out << "\tName = \"" << WideToChar(characterData._headerBlock._charName) << "\"\n";
	out << "\tSex = " << ((characterData._headerBlock._charSex == 1) ? "Male" : "Female") << "\n";
	out << "\tClass = " << Character::GetCharacterClassName(characterData._headerBlock._charClass) << "\n";
	out << "\tLevel = " << characterData._headerBlock._charLevel << "\n";
	out << "\tHardcore = " << ((characterData._headerBlock._charIsHardcore == 1) ? "True" : "False") << "\n";
	out << "\tExpansions = " << (uint32_t)characterData._headerBlock._charExpansions << "\n";
	out << "}\n";
	
	out << "Attributes\n{\n";
	out << "\tExperience = \"" << characterData._attributesBlock._charExperience << "\"\n";
	out << "\tAttributePoints = \"" << characterData._attributesBlock._charAttributePoints << "\"\n";
	out << "\tSkillPoints = \"" << characterData._attributesBlock._charSkillPoints << "\"\n";
	out << "\tDevotionPoints = \"" << characterData._attributesBlock._charDevotionPoints << "\"\n";
	out << "\tTotalDevotionPoints = \"" << characterData._attributesBlock._charTotalDevotionPoints << "\"\n";
	out << "\tPhysique = \"" << characterData._attributesBlock._charPhysique << "\"\n";
	out << "\tCunning = \"" << characterData._attributesBlock._charCunning << "\"\n";
	out << "\tSpirit = \"" << characterData._attributesBlock._charSpirit << "\"\n";
	out << "\tHealth = \"" << characterData._attributesBlock._charHealth << "\"\n";
	out << "\tEnergy = \"" << characterData._attributesBlock._charEnergy << "\"\n";
	out << "}\n";
	
	out << "Stats\n{\n";
	out << "\tPlayedTime = \"" << characterData._statsBlock._charPlayTime << "\"\n";
	out << "\tDeaths = \"" << characterData._statsBlock._charDeaths << "\"\n";
	out << "\tKills = \"" << characterData._statsBlock._charKills << "\"\n";
	out << "\tChampionKills = \"" << characterData._statsBlock._charChampionKills << "\"\n";
	out << "\tHeroKills = \"" << characterData._statsBlock._charHeroKills << "\"\n";
	out << "\tExperienceFromKills = \"" << characterData._statsBlock._charExpFromKills << "\"\n";
	out << "\tHitsReceived = \"" << characterData._statsBlock._charHitsReceived << "\"\n";
	out << "\tHitsInflicted = \"" << characterData._statsBlock._charHitsInflicted << "\"\n";
	out << "\tCritsReceived = \"" << characterData._statsBlock._charHitsReceived << "\"\n";
	out << "\tCritsInflicted = \"" << characterData._statsBlock._charCritsInflicted << "\"\n";
	out << "\tGreatestDamageReceived = \"" << characterData._statsBlock._charGreatestDamageReceived << "\"\n";
	out << "\tGreatestDamageInflicted = \"" << characterData._statsBlock._charGreatestDamageInflicted << "\"\n";
	out << "}\n\n\n";
	
	std::cout << path.string() << std::endl;
}

void PrintQuestData(const std::filesystem::path& path, std::ofstream& out)
{
	Quest questData;
	if (!questData.ReadFromFile(path))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load quest data from \"%\".", path.string()));
	
	out << "// " << path.string() << std::endl;
	
	out << "Tokens\n{\n";
	for (std::string token : questData._tokensBlock._questTokens)
	{
		out << "\t" << token << std::endl;
	}
	out << "}\n\n\n";
	
	std::cout << path.string() << std::endl;
}

void PrintFile(const std::filesystem::path& path, std::ofstream& out, bool silent)
{
	std::filesystem::path filename = path.filename();
	if (filename == "player.gdc")
	{
		PrintCharacterData(path, out);
	}
	else if (filename == "quests.gdd")
	{
		PrintQuestData(path, out);
	}
	else if (!silent)
	{
		throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file \"%\" is not currently supported.", filename));
	}
}

void Run(const char* argv)
{
    Logger::SetLogFilename("PrintTool.log");
	try
	{
		std::filesystem::path path = argv;
		std::string filename = "output_";
		filename += path.stem().string();
		filename += ".txt";
		
		std::ofstream out(filename, std::ofstream::out);
		if (out.is_open())
		{
			if (std::filesystem::is_directory(path))
			{
				for (const auto& it : std::filesystem::recursive_directory_iterator(path))
				{
					const std::filesystem::path& filePath = it.path();
					if (std::filesystem::is_regular_file(filePath))
					{
						PrintFile(filePath, out, true);
					}
				}
			}
			else if (std::filesystem::is_regular_file(path))
			{
				PrintFile(path, out, false);
			}
			else
			{
				throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified path \"%\" is not valid.", path));
			}
			
			std::cout << "File data successfully printed to " << filename << std::endl;
			out.close();
		}
		else
		{
			throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open file \"%\" for writing.", filename));
		}
	}
	catch (std::exception& ex)
	{
		std::cout << "[ERROR] " << ex.what();
	}
}

}