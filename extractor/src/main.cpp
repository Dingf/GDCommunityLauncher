#include "ItemDBR.h"
#include "ARZExtractor.h"
#include "Log.h"

int main(int argc, char** argv)
{
    ARZExtractor::Extract("C:\\temp\\database.arz", "C:\\temp\\test2");
}