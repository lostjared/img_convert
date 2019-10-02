#include<iostream>
#include<sstream>
#include<string>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<opencv2/opencv.hpp>

void add_directory(std::string path, std::string file_type, std::vector<std::string> &files) {
    DIR *dir = opendir(path.c_str());
    if(dir == NULL) {
        std::cerr << "img_convert: Error could not open directory: " << path << "\n";
        exit(EXIT_FAILURE);
        return;
    }
    dirent *file_info;
    while( (file_info = readdir(dir)) != 0 ) {
        std::string f_info = file_info->d_name;
        if(f_info == "." || f_info == "..")  continue;
        std::string fullpath=path+"/"+f_info;
        struct stat s;
#ifdef WIN32
        stat(fullpath.c_str(), &s);
#else
        lstat(fullpath.c_str(), &s);
#endif
        if(S_ISDIR(s.st_mode)) {
            if(f_info.length()>0 && f_info[0] != '.')
                add_directory(path+"/"+f_info, file_type, files);
            
            continue;
        }
        if(f_info.length()>0 && f_info[0] != '.') {
            std::string ext;
            auto pos = f_info.rfind(".");
            if(pos != std::string::npos) {
                std::string ext = f_info.substr(pos+1, f_info.length()-pos);
                std::string filename = f_info.substr(0, pos);
                if(ext == file_type) {
                    std::cout << "img_convert: added: " << fullpath << "\n";
                    files.push_back(fullpath);
                    continue;
                }
            }
        }
    } closedir(dir);
}

int main(int argc, char **argv) {
    std::string path = ".";
    std::string ext_from = "jpg";
    std::string ext_to = "png";
    int opt = 0;
    while((opt = getopt(argc, argv, "p:f:t:")) != -1) {
        switch(opt) {
            case 'p':
                path = optarg;
                break;
            case 'f':
                ext_from = optarg;
                break;
            case 't':
                ext_to = optarg;
                break;
        }
    }
    std::vector<std::string> file_names;
    add_directory(path,ext_from,file_names);
    if(file_names.size()>0) {
        for(unsigned int i = 0; i < file_names.size(); ++i) {
            cv::Mat img;
            img = cv::imread(file_names[i]);
            if(img.empty()) {
                std::cerr << "img_convert: Could not load image: " << file_names[i] << "\n";
                continue;
            }
            auto pos = file_names[i].rfind(".");
            if(pos != std::string::npos) {
                std::string ext = file_names[i].substr(pos+1, file_names[i].length()-pos);
                std::string filename = file_names[i].substr(0, pos);
                std::ostringstream stream;
                stream << filename << "." << ext_to;
                cv::imwrite(stream.str(), img);
                std::cout << "[" << i << "/" << file_names.size() << "] - Converted Image: " << file_names[i] << " to: " << stream.str() << "\n";
            }
        }
    } else {
        std::cout << "img_convert: Found zero images in path: " << path << "\n";
    }
    return 0;
}

