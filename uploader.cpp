#include <iostream>
#include <string>
#include "curl/curl.h"

struct UploadResult {
	bool error;
	std::string resMessage;
};

// Callback function to receive the response data
size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* response)
{
	size_t total_size = size * nmemb;
	response->append(ptr, total_size);
	return total_size;
}

// Uploads a file to pixeldrain and returns the upload's result
UploadResult upload(const std::string &file_name)
{
	UploadResult result;
	result.error = true;

	CURLcode init_res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (init_res != CURLE_OK) {
		result.resMessage = "Failed to initialize libcurl";
		return result;
	}

	CURL* curl = curl_easy_init();
	if (!curl) {
		result.resMessage = "Failed to initialize libcurl handle";
		return result;
	}

	curl_easy_setopt(curl, CURLOPT_URL, "https://pixeldrain.com/api/file");

	// Disable SSL verification
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

	// Set the file to be uploaded as the request body
	curl_mime* mime = curl_mime_init(curl);
	curl_mimepart* part = curl_mime_addpart(mime);
	curl_mime_name(part, "file");
	curl_mime_filedata(part, file_name.c_str());

	// Set the callback function to receive the response data
	std::string response;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	// Set the multipart form data
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

	CURLcode res = curl_easy_perform(curl);
	if (res == CURLE_OK) {
		result.error = false;
		result.resMessage = response;
	} else {
		result.resMessage = curl_easy_strerror(res);
	}

	curl_mime_free(mime);
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return result;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <file_name>" << std::endl;
		return 1;
	}

	std::string file_name = argv[1];
	UploadResult server_response = upload(file_name);
	if (!server_response.error) {
		std::cout << "Server response: " << server_response.resMessage << std::endl;
	} else {
		std::cerr << "Upload Error: " << server_response.resMessage << std::endl;
		return 1;
	}

	return 0;
}
