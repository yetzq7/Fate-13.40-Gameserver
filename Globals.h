#pragma once
#include <string>

class Globals
{
public:
	inline static const std::string DefaultPlaylistName = "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo"; 
	inline static const std::string DefaultPlaylistShortName = "Playlist_DefaultSolo"; //"Playlist_ShowdownAlt_Solo";

	inline static bool bCustomMap = false; // lunar skiddy paks needed
	inline static std::string PlaylistName = DefaultPlaylistName;
	inline static std::string PlaylistShortName = DefaultPlaylistShortName;
	// internally this is controlled in lategame.h's LateGameInternal::operator bool(), you can add some logic to only activate lategame after some amount of players 
	inline static bool bLateGame = false;
	inline static bool PregameMats = false;

	inline static bool bDisableLogs = true;

	//boss stuff
	inline static bool bBossesEnabled = true;
	inline static bool bBotsEnabled = true;
};
