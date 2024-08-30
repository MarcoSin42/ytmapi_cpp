#include "ytmapi.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>
#include <boost/regex.hpp>


using json = nlohmann::json;
using std::string;


int main() {

    std::ifstream html_file("pout.html");
    std::stringstream buf;
    buf << html_file.rdbuf();
    std::string pout_text = buf.str();

    std::string raw = ytmapi::extractJSONstr(pout_text);

    // std::string raw = R"~(\x7b "test": "abc" \x7d)~";
    

    json a = json::parse(raw);

    std::cout << a.dump();
}


