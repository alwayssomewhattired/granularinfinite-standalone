
#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_cryptography/juce_cryptography.h>

class SpotifyAuthenticator : juce::Thread
{
public:
	SpotifyAuthenticator()
		: juce::Thread("SpotifyAuthThread")
	{
	}

	~SpotifyAuthenticator() override
	{
		signalThreadShouldExit();
		stopThread(2000);
	}

	juce::String waitAndGetToken() {
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [this]() { return done; });
		return accessToken;
	}

	juce::String getToken()
	{
		return accessToken;
	}

	void init(const juce::String& clientID, const juce::String& clientSecreT, const juce::String& redirectUrI)
	{
		clientId = clientID;
		clientSecret = clientSecreT;
		redirectUri = redirectUrI;
	}

	void startAuthentication()
	{
		if (!isThreadRunning())
			startThread();
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

		juce::String code;
		while (!threadShouldExit() && code.isEmpty())
		{
			std::unique_ptr<juce::StreamingSocket> client(server.waitForNextConnection());
			if (client == nullptr) {
				continue;
			}

			juce::MemoryBlock buffer;
			buffer.setSize(8192);
			int totalBytes = 0;

			double startTime = juce::Time::getMillisecondCounterHiRes();
			const double timeoutMs = 5000.0;
			while (true)
			{
				int bytesRead = client->read(buffer.getData(), (int)buffer.getSize(), false);
				if (bytesRead > 0)
				{
					totalBytes += bytesRead;

					// checks if full HTTP request received
					juce::String temp((char*)buffer.getData(), totalBytes);
					if (temp.contains("\r\n\r\n"))
						break;
				}

				if (juce::Time::getMillisecondCounterHiRes() - startTime > timeoutMs)
				{
					std::cout << "Timed out waiting for HTTP request\n";
					break;
				}

				juce::Thread::sleep(5);
			}

			if (totalBytes > 0)
			{
				// sends a response back to the browser so doesn't hang
				juce::String request = juce::String::fromUTF8((const char*)buffer.getData(), totalBytes);


				int start = request.indexOf("GET /callback?code=");
				// conditional that grabs data from api response and posts a confirmation
				if (start >= 0)
				{
					int codeStart = start + juce::String("GET /callback?code=").length();

					// Looks for end of code (space or &)
					int codeEnd = request.indexOfChar(codeStart, ' ');
					int ampPos = request.indexOfChar(codeStart, '&');

					if (ampPos > 0 && ampPos < codeEnd)
						codeEnd = ampPos;

					if (codeEnd < 0)
						codeEnd = request.length();

					code = request.substring(codeStart, codeEnd).trim();
				
					juce::String response =
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Connection: close\r\n\r\n"
						"<html><body><h2>You can close this window.</h2></body></html>";

					client->write(response.toRawUTF8(), (int)response.getNumBytesAsUTF8());
				}
			}
			client->close();
		}
		if (!code.isEmpty())
		{
			juce::URL tokenUrl("https://accounts.spotify.com/api/token");

			juce::String body =
				"grant_type=authorization_code"
				"&code=" + code
				+ "&redirect_uri=" + redirectUri   // raw, exactly as registered
				+ "&client_id=" + clientId
				+ "&client_secret=" + clientSecret;


			juce::String authString = clientId + ":" + clientSecret;
			juce::String base64Auth = juce::Base64::toBase64(authString).trim();

			juce::String headers = "Content-Type: application/x-www-form-urlencoded\r\n";

			tokenUrl = tokenUrl.withPOSTData(body);


			juce::URL::InputStreamOptions options(juce::URL::ParameterHandling::inPostData);
			options.withConnectionTimeoutMs(10000);
			options.withExtraHeaders(headers);


			// this sends post request
			std::unique_ptr<juce::InputStream> stream(tokenUrl.createInputStream(options));

			if (stream != nullptr)
			{
				juce::String response = stream->readEntireStreamAsString();
				juce::var json = juce::JSON::parse(response);
				if (json.isObject() && json.hasProperty("access_token"))
				{
					std::lock_guard<std::mutex> lock(mutex);
					accessToken = json["access_token"].toString();
					done = true;
					cv.notify_one();
				}
			}
			else
			{
				std::cerr << "Failed to create input stream. \n";
			}
		}
		else {
			std::cout << "authentication completely failed\n";
		}
	}

	std::mutex mutex;
	std::condition_variable cv;
	bool done = false;

};