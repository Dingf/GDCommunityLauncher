#ifndef INC_GDCL_DLL_GAME_API_SKILL_H
#define INC_GDCL_DLL_GAME_API_SKILL_H

#include <string>

namespace GameAPI
{

#if _WIN64
constexpr char GAPI_NAME_GET_SKILL_MANAGER[] = "?GetSkillManager@Character@GAME@@QEAAAEAVSkillManager@2@XZ";
constexpr char GAPI_NAME_GET_SKILL_NAME_TAG[] = "?GetDisplayNameTag@Skill@GAME@@QEBAAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
#else
constexpr char GAPI_NAME_GET_SKILL_MANAGER[] = "?GetSkillManager@Character@GAME@@QAEAAVSkillManager@2@XZ";
constexpr char GAPI_NAME_GET_SKILL_NAME_TAG[] = "?GetDisplayNameTag@Skill@GAME@@QBEABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ";
#endif

void* GetSkillManager(void* character);
std::string GetSkillNameTag(void* skill);

}

#endif//INC_GDCL_DLL_GAME_API_SKILL_H