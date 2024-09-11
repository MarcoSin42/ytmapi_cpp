#ifndef YTM_API_H
#define YTM_API_H
#include "cpr/api.h"
#include <chrono>
#include <string>
#include <sys/types.h>
#include <vector>

using std::string;

namespace ytmapi {

struct Playlist {
    string title;
    string id;

};

struct Track {
    string videoId;
    string setVideoId; // This is distinct from videoId as it makes it distinct from copies within the same playlist
    string title;
    string artist;
    string album;
    int mins;
    int secs;
};


string extractJSONstr(string s);

using Playlists = std::vector<Playlist>;
using Tracks = std::vector<Track>;


class YTMusic {
    private:
        string m_oauthToken;
        string m_refreshToken;
        string m_language;
        string m_location;
        
        std::chrono::seconds m_expires_at; // Unix epoch time
    public:
        YTMusic(string oauth_path, string lang = "en");
        YTMusic();
        Tracks getPlaylistTracks(string playlistID);
        Playlists getPlaylists();

        void requestOAuth();
        bool refreshOAuth();

        bool createPlaylist(string title);
        bool delSongFromPlaylist(string playlistID, string videoId, string setVideoId);
        bool addSongToPlaylist(string playlistID, string videoId);
        bool delPlaylist(string playlistID);

        bool likeSong(string videoId);
        bool unlikeSong(string videoId);
        bool dislikeSong(string videoId);
        bool undislikeSong(string videoId);
    private:
        cpr::AsyncResponse contPlaylist(const string & ctoken);

        inline bool isValidOauth();
};
};
#endif