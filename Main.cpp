#define CURL_STATICLIB
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <curl/curl.h>
std::ofstream file;

struct Memory {
	char* mem;
	size_t size;
	Memory* next;
}; typedef Memory* memptr;
memptr websitedata;
std::vector<std::string> ziplinks;
FILE* fp;
//use this for inital download of website
size_t save_data(char* data, size_t itemsize, size_t nitems, void* ignore) {
	size_t bytes = itemsize * nitems;
	Memory* me = new Memory;
	me->mem = (char*)std::malloc(bytes);
	if (me->mem != 0) {
		std::memcpy(me->mem, data, bytes);
	}
	me->size = bytes;
	me->next = websitedata;
	websitedata = me;
	return bytes;
}
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}
//MSVCRT
int main() {
	CURL* curl;
	websitedata = NULL;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	std::string url;
	std::cout << "Input URL to scrape .zips from";
	std::cin >> url;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_perform(curl);
	curl_easy_reset(curl);
	//parse data for .zip file links
	memptr pt = websitedata;
	while (pt != NULL) {
		for (size_t i = 0; i < pt->size; i++) {
			bool found = false;
			//finding links
			if (pt->mem[i] == 'a') {
				std::string it = " href=\"";
				int n = 0;
				i++;
				bool quit = false;
				for (size_t j = i; j < pt->size && !quit; j++) {
					if (pt->mem[j] == it[n]) {
						n++;
					}
					else {
						n = 0;
						quit = true;
					}
					if (n == 6) {
						i = j;
						found = true;
						break;
					}
				}
			}
			if (found) {
				for (i; pt->mem[i] != '\"'; i++);
				i++;
				std::string str;
				//getting actual link 
				for (size_t n = i; pt->mem[n] != '\"' && n < pt->size; n++) {
					str.push_back(pt->mem[n]);
				}
				//checking it contains .zip file type
				std::string zi = ".zip";
				int n = 0;
				bool iszip = false;
				for (size_t j = 0; j < str.size(); j++) {
					if (str[j] == zi[n]) {
						n++;
					}
					else {
						n = 0;
					}
					if (n == 3) {
						iszip = true;
						break;
					}
				}
				if (iszip) {
					ziplinks.push_back(str);
				}
			}
		}
		pt = pt->next;
	}
	pt = websitedata;
	while (pt != NULL) {
		free(pt->mem);
		pt = pt->next;
	}
	curl_slist* header = curl_slist_append(NULL, "Connection: keep-alive");
	header = curl_slist_append(header, "sec-ch-ua:\"Chromium\";v=\"2021\", \"; Not A Brand\";v=\"99\"");
	header = curl_slist_append(header, "Sec-Fetch-Dest: document");
	header = curl_slist_append(header, "Sec-Fetch-Mode: navigate");
	header = curl_slist_append(header, "Accept-Language: en-US,en;q=0.9");
	header = curl_slist_append(header, "DNT: 1");
	for (auto& i : ziplinks) {
		std::cout << i << std::endl;
		std::string cururl = url + i;
		std::string filepath = "zips/" + i;
		fopen_s(&fp, filepath.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, cururl.c_str());
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1000L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_perform(curl);
		curl_easy_reset(curl);
		fclose(fp);
	}
	std::cout << std::endl;
	system("pause");
	curl_easy_cleanup(curl);
	return 0;
}