#pragma once

#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include "curl/curl.h"
#include <string>
#include <format>
#include <thread>
#include <memory>
#include "Globals.h"

#define BACKEND_URL     "http://20.51.153.182:5555"
#define SERVER_IP       "20.51.153.182"
#define SERVER_PORT     7777
#define SERVER_REGION   "NAE"
#define ADMIN_API_KEY   "c-09d9e8f-1234-5678-90ab-cdef12345678"
#define SERVER_SEASON   13

enum EReqType
{
	EReqType_POST,
	EReqType_GET,
	EReqType_DELETE
};

static size_t Write_Callback(char* contents, size_t size, size_t nmemb, void* RES)
{
	((std::string*)RES)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

class API
{
protected:
	curl_slist* headers = nullptr;
public:
	API()
	{
		curl_global_init(CURL_GLOBAL_ALL);
		headers = curl_slist_append(nullptr, "content-Type: application/json");
	}

	~API()
	{
		if (headers)
			curl_slist_free_all(headers);
		curl_global_cleanup();
	}

	FORCEINLINE bool Request(EReqType RequestType, const std::string& Endpoint, const std::string& Body, std::string* OutResponse = nullptr)
	{
		CURL* curl = curl_easy_init();
		if (!curl)
		{
			return false;
		}

		curl_easy_setopt(curl, CURLOPT_URL, Endpoint.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1500L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

		if (RequestType == EReqType_DELETE)
		{
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Body.c_str());
		}
		else if (RequestType == EReqType_POST)
		{
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Body.c_str());
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		}

		std::string callback;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Write_Callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);

		CURLcode Res = curl_easy_perform(curl);
		long statusCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
		curl_easy_cleanup(curl);

		if (Res != CURLE_OK)
		{
			return false;
		}

		if (statusCode < 200 || statusCode >= 300)
		{
			if (OutResponse)
				*OutResponse = callback;
			return false;
		}

		if (OutResponse)
			*OutResponse = callback;

		return true;
	}
};

namespace Backend
{
	static API* api = nullptr;
	static bool bRegistered = false;
	static int  gAlivePlayerCount = 0;
	static bool bGameStarted = false;

	void Setup()
	{
		if (!api)
			api = new API();
	}

	void RegisterServer()
	{
		if (!api) return;

		auto body = std::format(
			"{{\"ip\":\"{}\",\"port\":{},\"region\":\"{}\",\"playlist\":\"{}\",\"season\":{},\"maxPlayers\":{},\"apiKey\":\"{}\"}}",
			SERVER_IP, SERVER_PORT, SERVER_REGION, Globals::PlaylistShortName, SERVER_SEASON, 100, ADMIN_API_KEY
		);

		std::string response;
		bool ok = api->Request(EReqType_POST, BACKEND_URL "/api/v1/server/register", body, &response);
		if (ok)
		{
			bRegistered = true;
			Utils::Log("Server registered successfully!");
		}
		else
		{
			Utils::Log("Server registration failed! Response: ", response.c_str());
		}
	}

	void GameStarted()
	{
		if (!api || !bRegistered) return;

		auto body = std::format("{{\"ip\":\"{}\",\"port\":{}}}", SERVER_IP, SERVER_PORT);
		api->Request(EReqType_POST, BACKEND_URL "/api/v1/server/gamestarted", body);
	}

	void GameStopped()
	{
		if (!api || !bRegistered) return;
		bRegistered = false;

		auto body = std::format("{{\"ip\":\"{}\",\"port\":{}}}", SERVER_IP, SERVER_PORT);
		api->Request(EReqType_POST, BACKEND_URL "/api/v1/server/gamestopped", body);
	}

	void NotifyPlayerLeft()
	{
		if (!api) return;
		auto body = std::format("{{\"ip\":\"{}\",\"port\":{}}}", SERVER_IP, SERVER_PORT);
		api->Request(EReqType_POST, BACKEND_URL "/api/v1/server/playerleft", body);
	}

	void NotifyPlayerLeftAsync()
	{
		std::thread([]() { NotifyPlayerLeft(); }).detach();
	}

	void RewardVBucks(const std::string& username, int amount, const std::string& reason)
	{
		if (!api || username.empty()) return;
		auto body = std::format(
			"{{\"username\":\"{}\",\"amount\":{},\"apiKey\":\"{}\",\"reason\":\"{}\"}}",
			username, amount, ADMIN_API_KEY, reason
		);
		api->Request(EReqType_POST, BACKEND_URL "/api/v1/player/reward", body);
	}

	void RewardVBucksAsync(const std::string& username, int amount, const std::string& reason)
	{
		std::thread([username, amount, reason]() { RewardVBucks(username, amount, reason); }).detach();
	}

	static int GetPlacementHype(int placement)
	{
		if (placement == 1) return 100;
		if (placement <= 3) return 50;
		if (placement <= 5) return 25;
		return 0;
	}

	void AddArenaHype(const std::string& username, int amount)
	{
		if (!api || username.empty() || amount == 0) return;
		auto body = std::format(
			"{{\"username\":\"{}\",\"amount\":{},\"season\":{},\"apiKey\":\"{}\"}}",
			username, amount, SERVER_SEASON, ADMIN_API_KEY
		);
		api->Request(EReqType_POST, BACKEND_URL "/api/v1/arena/hype/add", body);
	}

	void AddArenaHypeAsync(const std::string& username, int amount)
	{
		std::thread([username, amount]() { AddArenaHype(username, amount); }).detach();
	}
}
