#include <cstdlib>
#include <cstring>
#include <exception>
#include <format>
#include <iostream>
#include <string>

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
#include <string_view>
#include "simdjson.h"

#include "ytmapi/ytmapi.hpp"
#include "ytmapi/utils.hpp"


using std::string, std::format;
using json = nlohmann::json;

namespace {
// Responsive List item Flex Colum (RLIFR) 
// Indexes into a flex column entry to retrieve the text content;
string inline getmRLIFRText(simdjson::ondemand::object colEntry) {
    try {
        //std::string_view view = colEntry["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"][0]["text"];
        std::string_view view = colEntry.at_path(".musicResponsiveListItemFlexColumnRenderer.text.runs[0].text");
        string result(view.begin(), view.end());
        return result;
    } catch (std::exception const&) { 
        // Occurs if it's not a YouTube music native, in which case some columns are empty!
        return "N/A";
    }
};

}


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

    cpr::Response r = cpr::Get(
        cpr::Url{"https://music.youtube.com/playlist"},
        cpr::Bearer{m_oauthToken},
        cpr::Header{
            {"accept-language", "en-GB,en-US;q=0.9,en;q=0.8,fr;q=0.7"},
            {"accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"},
            {"priority","u=0, i"},
            {"user-agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36"}
        },
        cpr::ReserveSize{1024 * 1024 * 8},
        cpr::Parameters{
            {"list", playlistID}
        }
    );
    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string = simdjson::padded_string(ytmapi_utils::extractJSONstr(r.text));
    simdjson::ondemand::document doc = parser.iterate(pad_string);
    
    simdjson::ondemand::value playlistShelfRenderer = 
        doc.at_path(".contents.twoColumnBrowseResultsRenderer.secondaryContents.sectionListRenderer.contents[0].musicPlaylistShelfRenderer");
    
    int itemCount = playlistShelfRenderer["collapsedItemCount"].get_int64();
    output.reserve(itemCount);
    simdjson::ondemand::array trackItems = playlistShelfRenderer["contents"].get_array();

    // used to index into the raw json
    constexpr int titleIdx  = 0;
    constexpr int artistIdx = 1;
    constexpr int albumIdx  = 2;
    std::string_view view;
    string title, artist, album, videoId, duration_str;
    
    for (simdjson::ondemand::value musicRespLstItemRenderer : trackItems) {
        // Yes, this is quite ugly, this API is not for public use. I'm sorry!
        simdjson::ondemand::object listItemRenderer = musicRespLstItemRenderer["musicResponsiveListItemRenderer"];
        
        //! TODO: convert unicode characters
        simdjson::ondemand::array flexColumns = listItemRenderer["flexColumns"];
        title  = getmRLIFRText(flexColumns.at(titleIdx));
        flexColumns.reset();
        artist = getmRLIFRText(flexColumns.at(artistIdx));
        flexColumns.reset();
        album  = getmRLIFRText(flexColumns.at(albumIdx)); 
        
        //view = listItemRenderer["fixedColumns"][0]["musicResponsiveListItemFixedColumnRenderer"]["text"]["runs"][0]["text"];
        view = listItemRenderer.at_path(".fixedColumns[0].musicResponsiveListItemFixedColumnRenderer.text.runs[0].text");
        duration_str = string(view.begin(), view.end()); 
        int mins = std::stoi(duration_str.substr(0, duration_str.find(":")));
        int secs = std::stoi(duration_str.substr(duration_str.find(":") + 1));

        view = listItemRenderer["playlistItemData"]["videoId"];
        videoId = string(view.begin(), view.end());

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
