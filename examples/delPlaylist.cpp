#include <iostream>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;

int main() {
    // Note: in order to run this example an oauth json must be within the same directory as the executable
    YTMusic test("oauth.json");

    bool result = test.delPlaylist("PLFk-2zvjcx0CwfVrn1VYP9dc8nhSQ8sns");
    
    std::cout << "Successful deletion: " << result << std::endl;

}