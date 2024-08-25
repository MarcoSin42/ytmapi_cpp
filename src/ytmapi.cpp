#include <format>
#include <string>
#include <fstream>

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

#include "ytmapi.hpp"

#include "cpr/body.h"
#include "cpr/cprtypes.h"
#include "cpr/response.h"

using std::string, std::format;
using json = nlohmann::json;

YTMusicBase::YTMusicBase(string oauth_path, string lang) {
    std::ifstream oauth_file(oauth_path);


    json oath_json = json::parse(oauth_file);
    
    m_oauthToken = oath_json["access_token"];
    m_refreshToken = oath_json["refresh_token"];

    m_language = lang;
}


Playlists YTMusicBase::getPlaylists() {
    Playlists output;
    cpr::Response r = cpr::Post(cpr::Url{"https://music.youtube.com/youtubei/v1/guide?prettyPrint=true"},
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
