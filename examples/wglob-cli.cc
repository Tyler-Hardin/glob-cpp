// wglob-cli.cc
#include <iostream>
#include <string>
#include <stdexcept>

#include "glob-cpp/glob.h"

// Helper to convert UTF-8 std::string_view to std::wstring.
// Hand-rolled decoder that avoids std::wstring_convert / <codecvt>
// (deprecated in C++17, removed in C++26).
std::wstring utf8_to_wstring(const std::string_view& utf8)
{
  std::wstring result;
  result.reserve(utf8.size());

  for (std::size_t i = 0; i < utf8.size(); ) {
    const auto byte = static_cast<unsigned char>(utf8[i]);

    // Determine the code point and how many bytes it spans.
    unsigned int cp;
    std::size_t extra;

    if (byte <= 0x7F) {
      cp = byte;
      extra = 0;
    } else if ((byte & 0xE0) == 0xC0) {
      cp = byte & 0x1F;
      extra = 1;
    } else if ((byte & 0xF0) == 0xE0) {
      cp = byte & 0x0F;
      extra = 2;
    } else if ((byte & 0xF8) == 0xF0) {
      cp = byte & 0x07;
      extra = 3;
    } else {
      throw std::runtime_error("invalid UTF-8 lead byte");
    }

    if (i + 1 + extra > utf8.size())
      throw std::runtime_error("truncated UTF-8 sequence");

    for (std::size_t j = 0; j < extra; ++j) {
      const auto c = static_cast<unsigned char>(utf8[i + 1 + j]);
      if ((c & 0xC0) != 0x80)
        throw std::runtime_error("invalid UTF-8 continuation byte");
      cp = (cp << 6) | (c & 0x3F);
    }

    result.push_back(static_cast<wchar_t>(cp));
    i += 1 + extra;
  }

  return result;
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <pattern> <string>\n";
    std::cerr << "Example: " << argv[0] << " \"[a-z]*\" \"test.txt\"\n";
    std::cerr << "Both arguments are interpreted as UTF-8.\n";
    return 1;
  }

  std::string_view utf8_pattern = argv[1];
  std::string_view utf8_input   = argv[2];

  std::wstring pattern;
  std::wstring input_string;

  try {
    pattern    = utf8_to_wstring(utf8_pattern);
    input_string = utf8_to_wstring(utf8_input);
  } catch (const std::exception& e) {
    std::cerr << "Error converting UTF-8 to wchar_t: " << e.what() << "\n";
    return 1;
  }

  glob::wglob glob_pattern(pattern);
  bool matches = glob::glob_match(input_string, glob_pattern);

  std::cout << "Pattern: \"" << utf8_pattern << "\"\n";
  std::cout << "String:  \"" << utf8_input << "\"\n";
  std::cout << "Result:  " << (matches ? "MATCH" : "NO MATCH") << "\n";

  return matches ? 0 : 1;
}
