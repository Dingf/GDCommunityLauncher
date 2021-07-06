#include "ItemDBR.h"
#include "ARZExtractor.h"
#include "ARCExtractor.h"
#include "Log.h"

int main(int argc, char** argv)
{
    ARZExtractor::Extract("C:\\temp\\database.arz", "C:\\temp\\test2");
    ARCExtractor::Extract("C:\\temp\\Items.arc", "C:\\temp\\test2");
    ARCExtractor::Extract("C:\\temp\\Text_EN.arc", "C:\\temp\\test2");
}