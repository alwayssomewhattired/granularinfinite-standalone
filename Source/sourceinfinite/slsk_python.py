
import slskd_api, time

# currently grabs first choice from searches.
# downloads the file onto the host machine
# 'incomplete' directory of slskd_volume
# is where the audio get stored


client = slskd_api.SlskdClient(
	host="http://localhost:5030",
	api_key="E+qXeEVNuSOB1kHL6qmxWdOjdszZ5Po26BsO+8sE1s0=",
	url_base="/"
)

def download_song(song_name):
	print("tally-ho knobknocker!")
	search = client.searches.search_text(song_name)
	print(search)
	search_id = search["id"]

	while True:
		state = client.searches.state(search_id)
		if state.get("isComplete") or state.get("state") == "Complete":
			break
		time.sleep(1)

	responses = client.searches.search_responses(search_id)
	response = responses[0]
	print(response)
	if len(responses) == 0:
		return "No results"
	response_username = response["username"]
	print(response_username)

	client.transfers.enqueue(response_username, response["files"])

	while True:
		download_info = client.transfers.get_downloads(response_username)
		print("download info: " + str(download_info))
		download_state = str(download_info['directories'][0]['files'][0]['state'])
		print(download_state)
		if download_state == "Completed, Succeeded":
			return "We have finished"
			
		elif download_state == "Completed, Errored":
			print("Error occurred. Retrying...")
			download_song(song_name)
		time.sleep(1)


	return "BOOM MUTHAFUCKAHhHhHh!"