

import slskd_api, time

client = slskd_api.SlskdClient(
	host="http://localhost:5030",
	api_key="E+qXeEVNuSOB1kHL6qmxWdOjdszZ5Po26BsO+8sE1s0=",
	url_base="/"
)

def download_song(song_name):
	search = client.searches.search_text(song_name)
	print(search)
	print("and there you have it")
	search_uuid = search["uuid"]

	while True:
		if client.searches.state(search_uuid).status == "completed":
			break
		time.sleep(1)

	responses = client.searches.search_responses(search_uuid)
	if not responses.items:
		return "No results"

	result = responses.items[0]
	download = client.transfers.enqueue(result.user, result.path)
	return f"Download started (ID = {download.id})"