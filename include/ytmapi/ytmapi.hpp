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
        YTMusic(const string& oauth_path, const string& lang = "en");
        YTMusic();
        Tracks getPlaylistTracks(const string& playlistID);
        Playlists getPlaylists();

        void requestOAuth();
        bool refreshOAuth();

        bool createPlaylist(const string& title);
        bool delSongFromPlaylist(const string& playlistID, const string& videoId, const string& setVideoId);
        bool addSongToPlaylist(const string& playlistID, const string& videoId);
        bool delPlaylist(const string& playlistID);

        bool likeSong(const string& videoId);
        bool unlikeSong(const string& videoId);
        bool dislikeSong(const string& videoId);
        bool undislikeSong(const string& videoId);
    private:
        cpr::AsyncResponse contPlaylist(const string & ctoken);

        inline bool isValidOauth();
};
};
#endif