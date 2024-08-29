#include <cstring>
#include <format>
#include <string>

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include "include/utils.hpp"
#include "include/ytmapi.hpp"


using std::string, std::format;
using json = nlohmann::json;

namespace ytmapi {

YTMusicBase::YTMusicBase(string oauth_path, string lang) {
    std::ifstream oauth_file;
    try {
        oauth_file = std::ifstream(oauth_path);
    } catch (std::ifstream::failure const&) {
        throw std::runtime_error("Unable to open the oauth json file");
    }

    try {
        json oath_json = json::parse(oauth_file);

        m_oauthToken = oath_json.at("access_token");
        m_refreshToken = oath_json.at("refresh_token");
    } catch (std::exception const&) {
        throw std::runtime_error("The OAUTH JSON file is incorrectly formatted");
    }

    m_language = lang;
}


Playlists YTMusicBase::getPlaylists() {
    Playlists output;
    cpr::Response r = cpr::Post(
        cpr::Url{"https://music.youtube.com/youtubei/v1/guide?prettyPrint=true"},
        cpr::Bearer{m_oauthToken},
        cpr::Header{
            {"accept", "*/*"},
            {"accept-language", "en-US"}, // TODO: Allow for different languages later
            {"Authorization", format("Bearer {}", m_oauthToken)}, // TODO: Allow for other types of authentication
            {"content-type", "application/json"},
            {"priority", "u=1, i"}
        },
        cpr::Body{R"~({"context":{"client":{"hl":"en","gl":"CA","userAgent":"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36,gzip(gfe)","clientName":"WEB_REMIX","clientVersion":"1.20240819.01.00"}}})~"}
    );

    json r_json = json::parse(r.text)["items"][1]["guideSectionRenderer"]["items"];

    for (json &entry : r_json) {
        output.push_back(Playlist{
            entry["guideEntryRenderer"]["formattedTitle"]["runs"][0]["text"],
            entry["guideEntryRenderer"]["entryData"]["guideEntryData"]["guideEntryId"]
        });
    }

    return output;
}



// This uses the publicly available YouTube Data API
Tracks YTMusicBase::getPlaylistTracks(string playlistID) {
    Tracks output;
    string contToken = "";
    json r_json;
    output.reserve(50);

    cpr::Response r = cpr::Get(
        cpr::Url{"https://youtube.googleapis.com/youtube/v3/playlistItems"},
        cpr::Bearer{m_oauthToken},
        cpr::Header{{"Accept", "application/json"}},
        cpr::Parameters{
            {"part", "contentDetails"},
            {"part", "snippet"},
            {"maxResults", "50"},
            {"playlistId", playlistID}
        }
    );
    //std::cout << r.text << "\n"; 
    r_json = json::parse(r.text);



    for (json &entry : r_json["items"]) {
        output.push_back(
            Track{
                entry["contentDetails"]["videoId"],
                entry["snippet"]["title"],
                ytmapi_utils::trimTopicSuffix(entry["snippet"]["videoOwnerChannelTitle"]),
                "placeholer",
                0,
                0
            }
        );
    }
    
    while (ytmapi_utils::keyExists(r_json, "nextPageToken")) {
        contToken = r_json["nextPageToken"];

        cpr::Response r = cpr::Get(
            cpr::Url{"https://youtube.googleapis.com/youtube/v3/playlistItems"},
            cpr::Bearer{m_oauthToken},
            cpr::Header{{"Accept", "application/json"}},
            cpr::Parameters{
                {"part", "contentDetails"},
                {"part", "snippet"},
                {"maxResults", "50"},
                {"playlistId", playlistID},
                {"pageToken", contToken},
            }
        );
        r_json = json::parse(r.text);
        
        for (json &entry : r_json["items"]) {
            output.push_back(
                Track{
                    entry["contentDetails"]["videoId"],
                    entry["snippet"]["title"],
                    ytmapi_utils::trimTopicSuffix(entry["snippet"]["videoOwnerChannelTitle"]),
                    "placeholer",
                    0,
                    0
                }
            );
        }
    }

    return output;
}

}; // namespace ytmapi
