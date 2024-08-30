#include <cstdio>
#include "ytmapi/ytmapi.hpp"

using namespace ytmapi;

int main() {
    // Note: in order to run this example an oauth json must be within the same directory
    YTMusicBase test("oauth.json");

    Playlists tmp = test.getPlaylists();


    for (auto &playlist : tmp) {
        printf("Title: %s \n", playlist.title.c_str());
        printf("ID: %s \n", playlist.id.c_str());
    }


}