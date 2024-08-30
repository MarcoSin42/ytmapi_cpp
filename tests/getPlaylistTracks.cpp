#include <cstdio>
#include <format>
#include <iostream>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;
int main() {
    // Note: in order to run this example an oauth json must be within the same directory
    YTMusicBase test("oauth.json");
    Tracks tmp = test.getPlaylistTracksPAPI("PLFk-2zvjcx0ATnreTXEe6gKYxxisfAey0"); // Retrieves liked music

    for (Track &t : tmp) {
        std::cout << std::format("Title: {:25} | Artist: {:15} | Album: {:15} | Duration: {:2}:{:2} | ID: {}",
        t.title,
        t.artist,
        t.album,
        t.mins,
        t.secs,
        t.id
        ) << std::endl;
    }
    std::cout << "0. size: " << tmp.size() << '\n';

}