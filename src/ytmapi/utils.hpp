#ifndef YTMAPI_UTILS_HPP
#define YTMAPI_UTILS_HPP


#include <iostream>

#include <boost/regex.hpp>

//#include "utils.hpp"

using std::string;

void inline ReplaceStringInPlace(std::string &subject, const std::string &search,
                          const std::string &replace) {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}


string replacementCallback(const boost::smatch& match) {
    if (match.str() == R"(\x7b)") {
        return "{";
    } else if (match.str() == R"(\x7d)") {
        return "}";
    } else if (match.str() == R"(\x22)") {
        return R"(")";
    } else if (match.str() == R"(\x5b)") {
        return "[";
    } else if (match.str() == R"(\x5d)") {
        return "]";
    } else if (match.str() == R"(\)") {
        return "";
    }

    return "";
}

namespace ytmapi_utils {
/** 
// Determines whether or not a key exists within a json file
bool inline keyExists(json a_json, string key) {
    try {
        a_json.at(key);
        return true;
    } catch (std::exception const&) {
        return false;
    }
}*/

// Utility function used exclusively for the 
string inline trimTopicSuffix(string s) {
    int startPosRm = s.find(" - Topic");
    if (startPosRm < 0)
        return s;

    return s.erase(startPosRm, strlen(" - Topic"));
}

// Utility function to extract 
string inline extractJSONstr(string s) {
    //WTF?  For some reason this is valid for boost regex but not valid for the standard library regex?!
    // WTF WTF WTF
    // I'VE WASTED TOO MUCH TIME ON STL'S REGEX LIBRARY, NEVER AGAIN.

    // I wish boost supported variable length look behind
    // This is a workaround
    boost::regex re1(R"~((\/browse)',(.*)(\\x7d))~");
    boost::regex re2(R"~((data: ')(.+)(\\x7d))~");
    boost::smatch m1, m2;

    if (!boost::regex_search(s, m1, re1)) {
        std::cerr << "Warning: YouTube Music may have changed their API\n";
        throw std::runtime_error("No Regex matches occured when looking for JSON string.");
    }
    string s2 = m1[0];
    if (!boost::regex_search(s2, m2, re2)) {
        std::cerr << "Warning: YouTube Music may have changed their API\n";
        throw std::runtime_error("No Regex matches occured when looking for JSON string.");
    }
    // m2[0]
    string input = m2[0].str(); 
    boost::regex re(R"~((\\x7b)|(\\x7d)|(\\x22)|(\\x5b)|(\\x5d)|\\)~");
    string out = boost::regex_replace(input, re, replacementCallback, boost::regex_constants::match_default);

    return out.substr(7);
}
};
#endif