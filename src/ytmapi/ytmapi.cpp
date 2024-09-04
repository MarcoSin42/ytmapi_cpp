#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <cpr/cpr.h>
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include "cpr/parameters.h"
#include "cpr/response.h"
#include "cpr/status_codes.h"
#include "simdjson.h"


#include "ytmapi/ytmapi.hpp"
#include "ytmapi/utils.hpp"

using std::string, std::format;

// Yes, I know this is bad practises and it should open up a file on the user's computer.  However, I don't really care if you use my free 10,000 tokens.
// This isn't even linked to my Personal Google account :P
std::string ytmCLIENT_ID = "1030788119864-ve66t4k7qsfmdu93c3u1ku09mie1vre7.apps.googleusercontent.com";
std::string ytmCLIENT_SECRET = "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";

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

string inline getCToken(simdjson::ondemand::object obj) {
    try {
        //std::string_view view = obj.find_field_unordered("continuations").at(0)["nextContinuationData"]["continuation"];
        std::string_view view = obj.at_path(".continuations[0].nextContinuationData.continuation");
        return string(view.begin(), view.end());
    } catch (std::exception const&) {
        return "";
    }
}

void inline appendTracks(ytmapi::Tracks &output, simdjson::ondemand::array trackItems) {
    using namespace ytmapi;
    std::string_view view;

    // used to index into the raw json
    constexpr int titleIdx  = 0;
    constexpr int artistIdx = 1;
    constexpr int albumIdx  = 2;
    string title, artist, album, videoId, duration_str;

    for (simdjson::ondemand::value musicRespLstItemRenderer : trackItems) {
        simdjson::ondemand::object listItemRenderer = musicRespLstItemRenderer["musicResponsiveListItemRenderer"];
        
        //! TODO: convert unicode characters
        simdjson::ondemand::array flexColumns = listItemRenderer["flexColumns"];
        title  = getmRLIFRText(flexColumns.at(titleIdx));
        flexColumns.reset();
        artist = getmRLIFRText(flexColumns.at(artistIdx));
        flexColumns.reset();
        album  = getmRLIFRText(flexColumns.at(albumIdx)); 
        
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
}


}


namespace ytmapi {

inline bool YTMusic::validOauth() {
    using namespace std::chrono;
    auto currentEpoch_ms = system_clock::now();
    if (std::chrono::duration_cast<seconds>(currentEpoch_ms.time_since_epoch()) < m_expires_at)
        return false;

    return true;
}

cpr::AsyncResponse YTMusic::contPlaylist(const string &ctoken) {
    return cpr::PostAsync(
        cpr::Url{"https://music.youtube.com/youtubei/v1/browse"},
        cpr::Bearer{m_oauthToken},
        cpr::Header{
            {"accept", "*/*"},
            {"content-type", "application/json"},
            {"priority","u=1, i"},
            {"user-agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36"}
        },
        cpr::Parameters{
            {"continuation", ctoken},
            {"prettyPrint","true"},
        },
        cpr::ReserveSize{1024 * 1024 * 4},
        cpr::Body{R"~({"context":{"client":{"hl":"en","gl":"CA","userAgent":"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36,gzip(gfe)","clientName":"WEB_REMIX","clientVersion":"1.20240819.01.00"}}})~"}
    );
}

// Mostly used for debug purposes
string extractJSONstr(string s) {
    return ytmapi_utils::extractJSONstr(s);
}


YTMusic::YTMusic(string oauth_path, string lang) {
    std::ifstream oauth_file;

    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string;
    simdjson::ondemand::document doc;

    std::string_view view;

    try {
        oauth_file = std::ifstream(oauth_path);
    } catch (std::ifstream::failure const&) {
        throw std::runtime_error("Unable to open the oauth json file");
    }

    try {
        std::stringstream buffer;
        buffer << oauth_file.rdbuf();
        simdjson::padded_string pad_string = simdjson::padded_string(buffer.str());
        doc = parser.iterate(pad_string);

        view = doc.find_field_unordered("access_token");
        m_oauthToken = string(view.begin(), view.end());

        view = doc.find_field_unordered("refresh_token");
        m_refreshToken = string(view.begin(), view.end());

        uint64_t dur_raw = doc.find_field_unordered("expires_in").get_uint64();
        m_expires_at = std::chrono::seconds(dur_raw);
    } catch (std::exception const&) {
        throw std::runtime_error("The OAUTH JSON file is incorrectly formatted");
    }

    m_language = lang;
}

YTMusic::YTMusic() {
    requestOAuth();
}


Playlists YTMusic::getPlaylists() {
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
    if (r.status_code != cpr::status::HTTP_OK)
        throw std::runtime_error(format("getPlaylists() HTTP request returned status code: {}", r.status_code));

    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string = simdjson::padded_string(r.text);
    simdjson::ondemand::document doc = parser.iterate(pad_string);

    simdjson::ondemand::array playlistEntries = doc.at_path(".items[1].guideSectionRenderer.items").get_array();
    std::string_view view;
    string playlistID, title;
    for (simdjson::ondemand::object entry: playlistEntries) {
        simdjson::ondemand::object guideEntry = entry["guideEntryRenderer"];

        view = guideEntry.at_path(".formattedTitle.runs[0].text");
        title = string(view.begin(), view.end());

        view = guideEntry["entryData"]["guideEntryData"]["guideEntryId"];
        playlistID = string(view.begin(), view.end());

        output.push_back(Playlist{
            title,
            playlistID
        });
    }

    return output;
}

Tracks YTMusic::getPlaylistTracks(string playlistID) {
    Tracks output;

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
    if (r.status_code != cpr::status::HTTP_OK)
        throw std::runtime_error(format("Attempted to get the following playlistID: {}.  Got an error code.  Is the PlaylistID valid?", playlistID));

    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string = simdjson::padded_string(ytmapi_utils::extractJSONstr(r.text));
    simdjson::ondemand::document doc = parser.iterate(pad_string);
    
    simdjson::ondemand::object sectionListRenderer = 
        doc.at_path(".contents.twoColumnBrowseResultsRenderer.secondaryContents.sectionListRenderer");
    simdjson::ondemand::value playlistShelfRenderer = 
        sectionListRenderer.at_path(".contents[0].musicPlaylistShelfRenderer");

    int itemCount = playlistShelfRenderer["collapsedItemCount"].get_int64();
    output.reserve(itemCount);
    simdjson::ondemand::array trackItems = playlistShelfRenderer["contents"].get_array();

    string contToken = getCToken(playlistShelfRenderer);
    cpr::AsyncResponse ar = contPlaylist(contToken);
    
    trackItems.reset();
    appendTracks(output, trackItems);

    // Asynchronous stuff
    //! TODO: There is a potential optimization here by removing the reset() call and reordering 
    while (contToken != "" && (r = ar.get()).status_code != cpr::status::HTTP_OK) {
        pad_string = simdjson::padded_string(r.text);
        doc = parser.iterate(pad_string);

        simdjson::ondemand::object contContents = doc["continuationContents"]["musicPlaylistShelfContinuation"];
        trackItems = contContents["contents"];
        contToken = getCToken(contContents);

        ar = contPlaylist(contToken);
        
        trackItems.reset();
        appendTracks(output, trackItems);
    }


    return output;
}

// Refreshes the OAUTH token, returns true if successful, false otherwise
bool YTMusic::refreshOAuth() {
    cpr::Response r = cpr::Post(
        cpr::Url{"https://oauth2.googleapis.com/token"},
        cpr::Parameters{
            {"client_id", ytmCLIENT_ID},
            {"client_secret", ytmCLIENT_SECRET}, 
            {"refresh_token", m_refreshToken},
            {"grant_type", "refresh_token"}
        }
    );
    if (r.status_code != cpr::status::HTTP_OK) // Unsure how to deal with this semantic-wise, allow the caller to handle this and return false or to throw an exception
        return false;
    
    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string = simdjson::padded_string(r.text);
    simdjson::ondemand::document doc = parser.iterate(pad_string);
    
    std::string_view view;
    
    view = doc.find_field_unordered("access_token");
    m_oauthToken = string(view.begin(), view.end());

    using namespace std::chrono;
    auto now = system_clock::now();
    uint64_t expires_in = doc.find_field_unordered("expires_in").get_uint64();
    m_expires_at =  duration_cast<seconds>(now.time_since_epoch()) + seconds(expires_in);

    return true;
}

void YTMusic::requestOAuth() {
    cpr::Response r = cpr::Post(
        cpr::Url{"https://oauth2.googleapis.com/device/code"},
        cpr::Parameters{
            {"client_id", ytmCLIENT_ID},
            {"scope", R"~(https://www.googleapis.com/auth/youtube)~"},
        }
    );
    switch (r.status_code) {
        case 403:
            throw std::runtime_error("Error code 403: Too many requests for authentication");
        case 400:
            throw std::runtime_error("Error code 400: We have an error, but we don't know what");
    };

    std::cout << r.text << std::endl;

    simdjson::ondemand::parser parser;
    simdjson::padded_string pad_string = simdjson::padded_string(r.text);
    simdjson::ondemand::document doc = parser.iterate(pad_string);
    std::string_view view;

    view = doc.at_path(".device_code");
    string device_code = string(view.begin(), view.end());

    view = doc.at_path(".user_code");
    string user_code = string(view.begin(), view.end());

    view = doc.at_path(".verification_url");
    string verification_url = string(view.begin(), view.end());

    string verification_link = verification_url + "?user_code=" + user_code; 
    #ifdef  _WIN64
    ShellExecute(0, 0, verification_link, 0, 0 , SW_SHOW ); // Untested on windows
    #elif __linux__
    system(format("xdg-open {}", verification_link).c_str());
    #endif

    do 
    {
    std::cout << '\n' << "Press enter once you have authorized an OAUTH request";
    } while (std::cin.get() != '\n');

    r = cpr::Post(
      cpr::Url{"https://oauth2.googleapis.com/token"},
      cpr::Parameters{
        {"client_id", ytmCLIENT_ID},
        {"client_secret", ytmCLIENT_SECRET},
        {"code", device_code},
        {"grant_type", R"(http://oauth.net/grant_type/device/1.0)"},
      }  
    );
    if (r.status_code != cpr::status::HTTP_OK)
        throw std::runtime_error("Error: Did you accept the OAUTH request?");

    std::ofstream oauth_file("oauth.json");
    oauth_file << r.text;
    oauth_file.close();

    try {
        simdjson::padded_string pad_string = simdjson::padded_string(r.text);
        doc = parser.iterate(pad_string);

        view = doc.find_field_unordered("access_token");
        m_oauthToken = string(view.begin(), view.end());

        view = doc.find_field_unordered("refresh_token");
        m_refreshToken = string(view.begin(), view.end());

        uint64_t dur_raw = doc.find_field_unordered("expires_in").get_uint64();
        m_expires_at = std::chrono::seconds(dur_raw);
    } catch (std::exception const&) {
        // Should not happen, unless the API has changed
        throw std::runtime_error("The OAuth JSON response was unable to be parsed.  Did YouTube change their API?");
    }

}


}; // namespace ytmapi
