#ifndef YTM_API_H
#define YTM_API_H
#include <string>
#include <vector>

using std::string;

struct Playlist {
    string title;
    string id;

};

struct Track {
    string id; 
    string title;
    string artist;
    string album;
    int mins;
    int secs;
};


using Playlists = std::vector<Playlist>;
using Tracks = std::vector<Track>;


class YTMusicBase {
    private:
        string m_oauthToken;
        string m_refreshToken;
        string m_language;
        string m_location;
    
    public:
        YTMusicBase(string oauth_path, string lang = "en");
        Tracks getPlaylistTracks(string playlistID, int limit, bool related, int suggestions_limit);
        Playlists getPlaylists();

        string getOauth();
    
    
};

#endif