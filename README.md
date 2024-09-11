This is C++ YouTube Music api based loosely off of the Python [YTM API](https://github.com/sigma67/ytmusicapi/tree/621584be3ca28d19667fc7c6353fab9ff09b7fa6).

I made this so I could use it in my terminal-user interface YouTube Music player.  

Rather than using YouTube's public data API, this library emulates browser requests because YouTube does not yet provide a publicly available API for YouTube Music.  

# Why use this over the Python Library?

The Python library is quite slow.  The Python library in some cases can take up to 20s to respond, this library has request latency parity with the browser (not rigorously profiled).

Retrieval of my liked music playlist (consisting of 225 songs) which takes 3 separate requests (up to 100 songs per message is permitted).  The roundtrip time for each request is ~0.7s, giving total network travel time of 2.1s.  Giving roughly 0.4s of processing time.  I believe this is sufficient for use in a UI as the even the YouTube music website using Firefox takes about that much time (even pre-cached).   

This library is an order of magnitude faster (at least on my machine) than it's Python bretheren.  Because I plan on using this in my YouTube music Terminal UI, I prioritized latency.  This library achieves its speedup due to it being faster at parsing responses.  JSON files are more than 50k lines long and in some cases must be extracted from large HTML files.  To deal with this issue, I used SIMDjson and Regex in order to minimize function call latency.  

You can easily adapt this for Python by creating a binder.  For example, I did exactly that in the other direction in my WIP [YouTube terminal user interface player here.](https://github.com/MarcoSin42/yay-tui/blob/7eba5eb558fd7451e7caf5912be83510d7612d86/src/py_wraps/python_wrappers.cpp)

# Usage

Using an empty constructor, the player should automatically prompt you for an OAUTH request and should open your browser and print to the terminal your device code.  If you already have an `oauth.json`, you can pass a the path to your `oauth.json` to the YTMusic base constructor.

For a comprehensive list of features, you should consult the header file.

## Feature list and todo

- [x] Getting user playlists - getPlaylists
- [x] Getting track information of a songs in a playlist - getPlaylistTrack
- [x] A way to prompt the user to get an OAUTH token
- [ ] Getting artist information
- [ ] Retrieving song meta data

### Exploring music
- [ ] Get latest charts globe and country
- [ ] Retrieval of moods and genre playlists

### Library management
- [x] Get library contents: playlists, songs
- [ ] artists, albums and subscriptions
- [ ] add/remove library content: rate songs, albums and playlists, subscribe/unsubscribe artists
- [ ] get and modify play history

### Playlists
- [x] Create playlists
- [x] Delete playlists
- [x] Modify playlists: add/move/remove tracks
- [ ] edit Playlist metadata
- [x] Get playlist contents
- [ ] Get playlist suggestions
and subscriptions
- [ ] Localization support
