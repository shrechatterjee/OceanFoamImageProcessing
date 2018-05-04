/*	skc105@ecs.soton.ac.uk */

#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <string>
#include <cmath>

#include "img_proc.h"
#include "opdata.h"

#ifdef _WIN32
    #include <Windows.h>
    //skeleton code taken from http://stackoverflow.com/questions/20860822/finding-the-file-path-of-all-files-in-a-folder
    // folder must end with "/", e.g. "D:/images/"
    vector<string> get_all_files_full_path_within_folder(string folder)
    {
        vector<string> names;
        char search_path[200];
        sprintf(search_path, "%s*.*", folder.c_str());
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(search_path, &fd);
        if(hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                // read all (real) files in current folder, delete '!' read other 2 default folder . and ..
                if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                {
                    names.push_back(folder+fd.cFileName);
                }
            }while(::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
        }
        return names;
    }
#endif // _WIN32

void showHelp();
bool process_img(string img_path, string op_path);

///*TODO* Add Boost Filesystem in order to get file timestamps (cross platform solution)
///*TODO* Make false whitecap detection less frequent (subimg corner type artefacts)

static bool ONLY_UNDISTORT = false;

int main(int argc, char *argv[])
{
    //string src_path = "E:/FoamCam Project/data/2013-11-02_12-01-24/000004/00004113.img";
    string src_path;// = "ip_imgs.xml";
    string op_path;// = "output_data.csv";
    bool output_mode = false; //true for simple output, false for advanced output (file structure creation and image output)

    for(int i=1; i<argc; i++)
    {
        if((!strcmp(argv[i], "-s")) && (i+1 <= argc))
            src_path = argv[i+1];
        else if((!strcmp(argv[i], "-d")) && (i+1 <= argc))
            op_path = argv[i+1];
        /*else if((!strcmp(argv[i], "-m")) && (i+1 <= argc)) ///*TODO* add exception handling for invalid input
            output_mode = argv[i+1];*/
        else if((!strcmp(argv[i], "-v")) && (i+1 <= argc))
            set_output_mode(true);
        else if((!strcmp(argv[i], "-u")) && (i+1 <= argc))
            ONLY_UNDISTORT = true;
        /*else if((!strcmp(argv[i], "-c")) && (i+1 <= argc))
        {
            if(src_path.empty() || op_path.empty())
                return -1;

            convertRaw(src_path, op_path, argv[i+1]);
            return 0;
        }*/
        else if(!strcmp(argv[i], "-?"))
        {
            showHelp();
            return 0;
        }
    }
    if(src_path == "")
    {
        cout << "No source path given, exiting." << endl;
        return 0;
    }
    size_t ext = src_path.find(".xml");
    bool foundxml = (ext!=string::npos);
    ext = src_path.find(".img");
    bool foundimg = (ext!=string::npos);
    bool founddir = (src_path[src_path.length()-1] == '/');

    if(foundxml)
        cout << "Getting images from xml image list." << endl;
    else if(foundimg)
        cout << "Loading image from .img file." << endl;
    else if(founddir)
        cout << "Searching directory for images." << endl;
    else
    {
        cerr << "Invalid source. Exiting program." << endl;
        return 1;
    }

    if(foundimg)
    {
        process_img(src_path, op_path);
    }
    else if(foundxml)
    {
        FileStorage fs;
        fs.open(src_path, FileStorage::READ);
        if (!fs.isOpened())
        {
            cerr << "Failed to open xml file " << src_path << endl;
            return 1;
        }

        FileNode n = fs["images"];
        if (n.type() != FileNode::SEQ)
        {
            cerr << "Could not read image list" << endl;
            return 1;
        }

        FileNodeIterator it = n.begin(), it_end = n.end();
        for (; it != it_end; ++it)
        {
            process_img((string)*it, op_path);
        }
    }
    else if(founddir)
    {
        vector<string> files = get_all_files_full_path_within_folder(src_path);

        for(int i=0; i<files.size(); i++)
        {
            size_t ext = files[i].find(".img");
            if(ext!=string::npos)
            {
                cout << "Processing image: " << files[i] << endl;
                process_img(files[i], op_path);
            }
        }
    }
    return 0;
}

bool process_img(string img_path, string op_path)
{
    Mat src = imreadRaw(img_path);
    if(!src.data)
    {
        cerr << "Invalid source image " << img_path << endl;
        return 1;
    }

    OpData data(img_path, src, false);
    data.addImg(data.getImgName(), src);
    showImg("Source", src);

    if(!ONLY_UNDISTORT)
        maskFrame(src);

    removeBarrelDist(src);
    data.addImg(data.getImgName() + "_undistorted", src);

    showImg("Source - Undistorted", src);

    if(!ONLY_UNDISTORT)
        extractWhitecaps(src, data);

    cout << "data writing: ";
        if(data.save(op_path))
            cout << "success" << endl;
        else
            cout << "fail" << endl;
    if(SHOW_DEBUG_IMGS)
        waitKey(0);// wait for a keystroke in the window

    return 0;
}

void showHelp()
{
    cout << endl << "Optional command line arguments:" << endl;
    cout << "-s [src] -d [dest] -v -u" << endl;
    cout << "-s: .img source file or .xml image list" << endl;
    cout << "-d: data output destination directory" << endl;
   // cout << "-c: convert .img file to specified format" << endl;
    //cout << "-m: output mode, 0 = advanced, 1 = simple (.dat file only, in source directory)" << endl;
   // cout << " Overrides other arguments" << endl;
    cout << "-v: view debug images" << endl;
    cout << "-u: only undistort image" << endl;

    cout << " Formats: " << SUPPORTED_IMG_FORMATS << endl;
    cout << "-?: help" << endl;
}
