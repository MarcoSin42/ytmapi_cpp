#include <cstring>
#include <format>
#include <iostream>
#include <string>

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include "ytmapi/ytmapi.hpp"
#include "ytmapi/utils.hpp"


using std::string, std::format;
using json = nlohmann::json;

namespace ytmapi {

// Mostly used for debug purposes
string extractJSONstr(string s) {
    return ytmapi_utils::extractJSONstr(s);
}


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
/**
This has the following limitations:
    - This does not provide the album data
 */
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


Tracks YTMusicBase::getPlaylistTracksPAPI(string playlistID) {
    Tracks output;
    json r_json;

    // used to index into the raw json
    constexpr int titleIdx = 0;
    constexpr int artistIdx = 1;
    constexpr int albumIdx = 2;

    cpr::Response r = cpr::Get(
        cpr::Url{"https://music.youtube.com/playlist"},
        cpr::Bearer{m_oauthToken},
        cpr::Header{
            {"accept-language", "en-GB,en-US;q=0.9,en;q=0.8,fr;q=0.7"},
            {"accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"},
            {"priority","u=0, i"},
            {"user-agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36"}
        },
        cpr::Parameters{
            {"list", playlistID}
        }
    );
    r_json = json::parse(ytmapi_utils::extractJSONstr(r.text));

    //std::cout << r_json.dump() << std::endl;
    
    int itemCount  = r_json["contents"]["twoColumnBrowseResultsRenderer"]["secondaryContents"]["sectionListRenderer"]["contents"][0]["musicPlaylistShelfRenderer"]["collapsedItemCount"];
    auto trackItems = r_json["contents"]["twoColumnBrowseResultsRenderer"]["secondaryContents"]["sectionListRenderer"]["contents"][0]["musicPlaylistShelfRenderer"]["contents"];
    output.reserve(itemCount);

    std::cout << "Track is: " << trackItems.is_array() << "\n";
    std::cout << "Track size: " << trackItems.size() << "\n";

    // Responsive List item Flex Colum (RLIFR) 
    // Indexes into a flex column entry to retrieve the text content;
    auto getmRLIFRText = [&](json colEntry) -> string {
        return colEntry["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"][0]["text"];
    };

    for (json &musicRespLstItemRenderer : trackItems) {
        // Yes, this is quite ugly, this API is not for public use. I'm sorry!
        string duration_str = 
            musicRespLstItemRenderer["musicResponsiveListItemRenderer"]["fixedColumns"][0]["musicResponsiveListItemFixedColumnRenderer"]["text"]["runs"][0]["text"];

        //! TODO: convert unicode characters
        string title  = getmRLIFRText(musicRespLstItemRenderer["musicResponsiveListItemRenderer"]["flexColumns"][titleIdx]);
        string artist = getmRLIFRText(musicRespLstItemRenderer["musicResponsiveListItemRenderer"]["flexColumns"][artistIdx]);
        string album  = getmRLIFRText(musicRespLstItemRenderer["musicResponsiveListItemRenderer"]["flexColumns"][albumIdx]); 

        string videoId = musicRespLstItemRenderer["musicResponsiveListItemRenderer"]["playlistItemData"]["videoId"];

        int mins = std::stoi(duration_str.substr(0, duration_str.find(":")));
        int secs = std::stoi(duration_str.substr(duration_str.find(":") + 1));

        output.push_back(Track{
            videoId,
            title,
            artist,
            album,
            mins,
            secs
        });
    }
    return output;
}



}; // namespace ytmapi
