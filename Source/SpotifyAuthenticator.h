
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
			"&response_type=code" +
			"&redirect_uri=" + juce::URL::addEscapeChars(redirectUri, true) +
			"&scope=" + juce::URL::addEscapeChars("user-top-read", true);

		juce::URL(url).launchInDefaultBrowser();

		juce::StreamingSocket server;
		if (!server.createListener(8888, "0.0.0.0"))
		{
			std::cout << "Could not bind to 127.0.0.1:8888 \n";
			return;
		}
		else
		{
			std::cout << "Listening on 127.0.0.1:8888...\n";
		}

		juce::String code;
		while (!threadShouldExit() && code.isEmpty())
		{
			std::cout << "in first condition \n";
			std::unique_ptr<juce::StreamingSocket> client(server.waitForNextConnection());
			if (client == nullptr)
				continue;

				juce::MemoryBlock buffer;
				buffer.setSize(8192);
				int bytesRead = client->read(buffer.getData(), (int)buffer.getSize(), true);

				if (bytesRead > 0)
				{
					juce::String request = juce::String::fromUTF8((const char*)buffer.getData(), bytesRead);

					int start = request.indexOf("GET /?code=");
					if (start >= 0)
					{
						int end = request.indexOfChar('&', start);
						juce::String codeParam = request.substring(start + 10, end);
						code = codeParam.upToFirstOccurrenceOf("&", false, false);

						juce::String response = 
							"HTTP/1.1 200 OK\r\n"
							"Content-Type: text/html\r\n\r\n"
							"<html><body><h2>You can close this window</h2></body></html>";

						client->write(response.toRawUTF8(), (int)response.getNumBytesAsUTF8());
						}
					}
				client->close();
				}
		if (!code.isEmpty())
		{
			std::cout << "in second condition \n";

			juce::URL tokenUrl("https://accounts.spotify.com/api/token");

			juce::String body = 
				"grant_type=authorization_code"
				"&code=" + code +
				"&redirect_uri=" + juce::URL::addEscapeChars(redirectUri, true) +
				"&client_id=" + clientId +
				"&client_secret=" + clientSecret;

			tokenUrl = tokenUrl.withPOSTData(body);

			juce::String headers = "Content-Type: application/x-www-form-urlencoded";

			juce::URL::InputStreamOptions options(juce::URL::ParameterHandling::inPostData);
			options.withConnectionTimeoutMs(10000);
			options.withExtraHeaders(headers);

			std::unique_ptr<juce::InputStream> stream(tokenUrl.createInputStream(options));

			if (stream != nullptr)
			{
				juce::String response = stream->readEntireStreamAsString();
				juce::var json = juce::JSON::parse(response);
				if (json.isObject() && json.hasProperty("access_token"))
				{
					accessToken = json["access_token"].toString();
					std::cout << "Access token: " << accessToken << "\n";
				}
				else 
				{
					std::cout << "Token response: " << response << "\n";
				}
			}
			else 
			{
				std::cerr << "Failed to create input stream. \n";
			}
		}
	}
};