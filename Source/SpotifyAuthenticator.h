
#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_cryptography/juce_cryptography.h>

class SpotifyAuthenticator : juce::Thread
{
public:
	SpotifyAuthenticator(const juce::String& clientID, const juce::String& clientSecret, const juce::String& redirectUri)
		: juce::Thread("SpotifyAuthThread"),
		clientId(clientID),
		clientSecret(clientSecret),
		redirectUri(redirectUri)
	{ 
	}

	~SpotifyAuthenticator() override
	{
		signalThreadShouldExit();
		stopThread(2000);
	}

	void startAuthentication()
	{
		if (!isThreadRunning())
			startThread();
	}

	juce::String getAccessToken() const
	{
		return accessToken;
	}

private:
	juce::String clientId;
	juce::String clientSecret;
	juce::String redirectUri;
	juce::String accessToken;

	void run() override
	{
		juce::String url =
			"https://accounts.spotify.com/authorize"
			"?client_id=" + clientId +
			"&response_type=code"
			"&redirect_uri=" + juce::URL::encode(redirectUri) +
			"&scope=" + juce::URL::encode("user-top-read");

		juce::URL::launchInDefaultBrowser(url);

		juce::StreamingSocket server;
		server.createListener(8888, "127.0.0.1");

		juce::String code;

		while (!threadShouldExit() && code.isEmpty())
		{
			if (server.waitUntilReady(true, 1000))
			{
				juce::StreamingSocket client;
				server.accept(client);

				juce::MemoryBlock buffer;
				buffer.setSize(8192);
				int bytesRead = client.read(buffer.getData(), (int)buffer.getSize(), true);
				juce::String request = juce::String::fromUTF8((const char*)buffer.getData(), bytesRead);

				auto match = request.indexOf(" ", match);
				if (match >= 0)
				{
					auto end = request.indexOf(" ", match);
					code = juce::URL::decode(request.substring(match + 5, end));

					juce::String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>You can close this window</h2></body></html>";
					client.write(response.toRawUTF8(), (int)response.getNumBytesAsUTF8());
				}
			}
		}
		if (code.isEmpty())
		{
			juce::URL tokenUrl("https://accounts.spotify.com/api/token");

			juce::String body = 
				"grant_type=authorization_code"
				"&code=" + code +
				"&redirect_uri=" + juce::URL::encode(redirectUri) +
				"&client_id=" + clientId +
				"&client_secret=" + clientSecret;

			juce::String headers = "Content-Type: application/x-www-form-urlencoded";

			auto stream = tokenUrl.createInputStream(
				false, nullptr, nullptr, headers, 10000, body.toRawUTF8(), (int)body.getNumBytesAsUTF8()
			);

			if (stream != nullptr)
			{
				juce::String response = stream->readEntireStreamAsString();
				juce::var json = juce::JSON::parse(response);
				if (json.isObject() && json.hasProperty("access_token"))
				{
					accessToken = json["access_token"].toString();
					std::cout << "Access token: " << accessToken << "\n";
				}
			}
		}
	}
};