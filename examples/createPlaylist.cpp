#include <iostream>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;

int main() {
    // Note: in order to run this example an oauth json must be within the same directory as the executable
    YTMusic test("oauth.json");

    bool result = test.createPlaylist("this is a test");

    std::cout << "Successful creation: " << result << std::endl;

}