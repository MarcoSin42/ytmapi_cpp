#include <cstdio>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;
int main() {
    // Note: in order to run this example an oauth json must be within the same directory
    YTMusic test;

    // https://music.youtube.com/watch?v=lYBUbBu4W08 | Rick Astley's Never going to give you up
    test.unlikeSong("lYBUbBu4W08");
}