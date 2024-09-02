This is C++ YouTube Music api based loosely off of the Python [YTM API](https://github.com/sigma67/ytmusicapi/tree/621584be3ca28d19667fc7c6353fab9ff09b7fa6).

I made this so I could use it in my terminal-user interface YouTube Music player.  

Rather than using YouTube's public data API, this library emulates browser requests because YouTube does not yet provide a publicly available API for YouTube Music.  

# Why use this over the Python Library?

The Python library is quite slow.  This is an order of magnitude faster (at least on my machine).  I attribute this speedup due to the way responses are parsed.  As I plan on using this in my UI, I prioritized latency.  This library achieves its speedup due to it being faster at parsing responses, the JSON files can be quite large (>50k lines).  I used SIMDjson, intelligent use of REGEX as well as asynchronous requests to improve latency between the time of calling the function and the time the function is returned.  

You can easily adapt this for Python by creating a binder.  For example, I did the following here except in the opposite direction in [my project here.](https://github.com/MarcoSin42/yay-tui/blob/7eba5eb558fd7451e7caf5912be83510d7612d86/src/py_wraps/python_wrappers.cpp)

# Usage

First, you need to obtain an oauth.  Currently, I have not implemented a feature to prompt the user.  You can obtain an oauth for your desired account using the [YTMusic API](https://github.com/sigma67/ytmusicapi/) Python library.  After installing YTMusic API, you simply run `ytmusic oauth` which should prompt you to open your browser.

Then, you want to initialize a `YTMusicBase` by passing it the path to your `oauth.json` which you just created.


Read the header file.  Currently this only supports retrieval of playlists and retrieval of track information.  In the future, I plan on reaching feature parity with the Python library.  



## Feature list and todo

- [x] Getting user playlists - getPlaylists
- [x] Getting track information of a songs in a playlist - getPlaylistTrack
- [ ] A way to prompt the user to get an OAUTH token
- [ ] Getting artist information
- [ ] Retrieving song meta data

### Exploring music
- [ ] Get latest charts globe and country
- [ ] Retrieval of moods and genre playlists

### Library management
- [ ] Get library contents: playlists, songs, artists, albums and subscriptions
- [ ] add/remove library content: rate songs, albums and playlists, subscribe/unsubscribe artists
- [ ] get and modify play history

### Playlists
- [ ] Create and delete playlists
- [ ] Modify playlists: edit metadata, add/move/remove tracks
- [x] Get playlist contents
- [ ] Get playlist suggestions

### Localization features
- [x] Only English is supported
- [ ] Localization support
