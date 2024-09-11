#include <cstdio>
#include <format>
#include <iostream>
#include <ostream>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;
int main() {
    // Note: in order to run this example an oauth json must be within the same directory
    YTMusic test("oauth.json");
    Tracks tmp = test.getPlaylistTracks("LM"); // Retrieves liked music

    for (Track &t : tmp) {
        std::cout << std::format("Title: {:25.25s} | Artist: {:15.15s} | Album: {:15.15s} | Duration: {:02}:{:02} | ID: {}",
        t.title,
        t.artist,
        t.album,
        t.mins,
        t.secs,
        t.videoId
        ) << "\n";
    }
    std::cout << "0. size: " << tmp.size() << '\n';

}