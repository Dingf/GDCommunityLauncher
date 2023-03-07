#ifndef INC_GDCL_DLL_ITEM_LINKER_H
#define INC_GDCL_DLL_ITEM_LINKER_H

#include <stdint.h>

class ItemLinker
{
    public:
        static bool Initialize();
        static void Cleanup();

        static void* GetLinkedItem();

    private:
        void*   _item;          // The last hovered item to be linked when calling GetLinkedItem()
        int64_t _timestamp;     // The timestamp of the link event (to prevent issues with multiple tooltips, e.g. item comparison)

        void LinkItem(void* item);

        static void HandleItemLink(void* _this, void* character, void* lines);
        static void HandleItemLinkArtifact(void* _this, void* character, void* lines);
        static void HandleItemLinkFormula(void* _this, void* character, void* lines);
        static void HandleItemLinkEnchantment(void* _this, void* character, void* lines);
        static void HandleItemLinkNote(void* _this, void* character, void* lines);
        static void HandleItemLinkRelic(void* _this, void* character, void* lines);

        static ItemLinker& GetInstance();
};

#endif//INC_GDCL_DLL_ITEM_LINKER_H