#include "../NewGame/PreviewModes.hpp"
#include <cassert>
#include <iostream>
int main()
{
    using namespace frontline::preview;
    for (const auto& mode : Modes) assert(FindMode(mode.name) == mode.id);
    assert(FindMode("") == 1 && FindMode("unknown") == 1);
    assert(FindMode("China-atlas") == 1 && FindMode("china-atlas ") == 1);
    assert(IsCompact(68) && !IsCompact(67));
    std::cout << "Preview selectors and compact sizes passed\n";
}
