#include <iostream>
#include <fstream>
#include <string>
#include <regex>

#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

#include <curl/curl.h>

#define APOD_URL "http://apod.nasa.gov/apod"

using namespace std;

class MemoryPage
{
        public:
                MemoryPage() {
                        memory = (char*) malloc(1);
                        size = 0;
                }
                ~MemoryPage() {
                        cerr << "Dealloc" << endl;
                        free(memory);
                        size = 0;
                }
                char* memory;
                size_t size;

};


static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
                                  void* userp) {
        size_t realsize = size * nmemb;
        MemoryPage* mem = (MemoryPage*)userp;

        mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
        if (mem->memory == NULL) {
                std::cout << "Not enough memory (realloc returned NULL)"
                          << std::endl;
                return 0;
        }

        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
}

MemoryPage* get_page(string url) {
        CURL* curl;
        CURLcode res;

        MemoryPage* page = new MemoryPage;

        curl = curl_easy_init();
        if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                                 WriteMemoryCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)page);
                curl_easy_setopt(curl, CURLOPT_USERAGENT,
                                 "apod_downloader/0.1 github.com/supr");

                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                        std::cerr << "curl_easy_perform() failed: "
                                  << curl_easy_strerror(res) << std::endl;
                        return nullptr;
                }

                std::cerr << "Downloaded: " << url << "\n" << (long)page->size
                          << " bytes retrieved" << std::endl;
                return page;
        }
        return nullptr;
}

string get_apod_image_url(void) {
        MemoryPage* apod_page = get_page(APOD_URL);
        if(apod_page->size) {
                regex reg("<a href=\"(image.*)\">");
                smatch m;
                if(regex_search(string(apod_page->memory), m, reg)) {
                        delete apod_page;
                        string image_url(string(APOD_URL) + "/" + m[1].str());
                        cerr << "Image URL: " << image_url << endl;
                        return image_url;
                }
        }
        
        delete apod_page;
        return "";
}

string basename(string loc) {
 return loc.substr(loc.find_last_of("/") + 1);       
}

bool download_image(string url) {
        if(!url.empty()) {
                MemoryPage* image = get_page(url);
                if(image->size) {
                        ofstream myfile(basename(url), ios::binary);
                        if(myfile.is_open()) {
                                myfile.write(image->memory, image->size);
                                myfile.close();
                        } else {
                                delete image;
                                return false;
                        }
                        delete image;
                        return true;
                }

                return false;
        }

        return false;
}

void set_gnome_wallpaper(string loc) {
       char cmd[PATH_MAX] = {0};
       sprintf(cmd, "gsettings set org.gnome.desktop.background picture-uri file://%s", loc.c_str());
       cerr << "CMD: " << cmd << endl;
       system(cmd); 
}

int main() {
        string image_url = get_apod_image_url();
        if(download_image(image_url)) {
                char path[PATH_MAX] = {0};
                char buffer[PATH_MAX] = {0};
                getcwd(path, PATH_MAX);
                cerr << "Current Path: " << path << endl;
                sprintf(buffer, "%s/%s", path, basename(image_url).c_str());
                set_gnome_wallpaper(string(buffer));
        }
}
