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

size_t write_data(char* data, size_t itemsize, size_t nitems, void* ignore) {
	size_t bytes = itemsize * nitems;
	file << data;
	return bytes;
}



int main() {
	CURL* curl;
	websitedata = NULL;
	curl = curl_easy_init();
	std::string url;
	std::cout << "Input URL to scrape .zips from";
	std::cin >> url;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
	curl_easy_perform(curl);
	curl_easy_reset(curl);
	//parse data for .zip file links
	memptr pt = websitedata;
	while (pt != NULL) {
		for (size_t i = 0; i < pt->size; i++) {
			//probably more efficient way of doing this
			if (pt->mem[i] == '.') {
				if (pt->mem[i + 1] == 'z') {
					if (pt->mem[i + 2] == 'i') {
						if (pt->mem[i + 3] == 'p') {
							i += 3;
							std::string str;
							for (size_t n = i; pt->mem[n] != '\"' && n >= 0; n--) {
								str.push_back(pt->mem[n]);
							}
							//have to reverse string before pushing it back
							ziplinks.push_back(str);
						}
					}
				}
			}
		}
		pt = pt->next;
	}
	for (auto& i : ziplinks) {
		std::cout << i << std::endl;
	}
	std::cout << std::endl;
	//put zip download loop here
	system("pause");
	curl_easy_cleanup(curl);
	return 0;
}