#include <iostream>
#include <regex>
#include <string>
int main() {
    // en-dash U+2013 in UTF-8: 0xE2 0x80 0x93
    std::string endash = "\xe2\x80\x93";
    std::string text = "1000" + endash + "925 BCE";
    // Test: character class (broken for multibyte) vs alternation (works)
    std::regex r_class(R"((\d+)\s*[-)" + endash + R"(]\s*(\d+)\s*BCE)", std::regex::icase);
    std::regex r_alt(  "(" R"(\d+)" ")" R"(\s*(?:-|)" + endash + R"()\s*(\d+)\s*BCE)", std::regex::icase);
    std::smatch m;
    std::cout << "text len=" << text.size() << "\n";
    std::cout << "char class match: " << std::regex_search(text, m, r_class) << "\n";
    std::cout << "alternation match: " << std::regex_search(text, m, r_alt) << "\n";
    if (std::regex_search(text, m, r_alt))
        std::cout << "  y1=" << m[1] << " y2=" << m[2] << "\n";
    return 0;
}
