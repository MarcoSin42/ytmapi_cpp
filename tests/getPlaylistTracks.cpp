#include <cstdio>
#include <iostream>
#include "ytmapi.hpp"


int main() {
    // Note: in order to run this example an oauth json must be within the same directory
    YTMusicBase test("oauth.json");
    Tracks tmp = test.getPlaylistTracks("LM"); // Retrieves liked music

    for (Track &t : tmp) {
        printf("Title: %60s | Artist: %25s \n", t.title.c_str(), t.artist.c_str());
    }
    std::cout << "0. size: " << tmp.size() << '\n';

}