

import slskd_api, time

client = slskd_api.SlskdClient(
	host="http://localhost:5030",
	api_key="E+qXeEVNuSOB1kHL6qmxWdOjdszZ5Po26BsO+8sE1s0=",
	url_base="/"
)
# IMPORTANT
# RUN THIS SCRIPT IN VS CODE
# IN ORDER TO FIND WHAT THE RESULTS (DICTS) CONTAIN 
# CANNOT PRINT TO CONSOLE
# TOO DIFFICULT. 
# JUST USE VSCODE
# OR ACTUALLY....
# YEAH NO JUST USE VSCODE

def download_song(song_name):
	print("tally-ho knobknocker!")
	search = client.searches.search_text(song_name)
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

	while True:
		state = client.transfers.state(download["id"])
		if state["status"] == "completed":
			break
		time.sleep(1)

	return f"Downloaded to: {state["local_path"]})"