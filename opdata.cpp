/*	skc105@ecs.soton.ac.uk */

#include "opdata.h"

    #ifdef _WIN32
        #include "opdata_win.h"
    #elif __unix ///No idea if this works on unix, have not tested.
        #include "opdata_unix.h"
    #else
        return EXIT_FAILURE;
    #endif

OpData::OpData(string new_src_img_path, bool simple_save)
{
    fields = new vector<pair<string, string>*>;
    imgs = new vector<pair<string, Mat*>*>;
    src_img_path = new string(new_src_img_path);
    src_img = new Mat;
    simple_output = simple_save;

    addField("date_modified", getTimestamp());
    addField("src_no", getImgName());
}
OpData::OpData(string new_src_img_path, Mat& new_src_img, bool simple_save)
{
    fields = new vector<pair<string, string>*>;
    imgs = new vector<pair<string, Mat*>*>;
    src_img_path = new string(new_src_img_path);
    src_img = new Mat;
    *src_img = new_src_img.clone();
    simple_output = simple_save;

    addField("date_modified", getTimestamp());
    addField("src_no", getImgName());
}

OpData::~OpData()
{
    for(int i=0; i<fields->size(); i++)
        delete fields->at(i);

    for(int i=0; i<imgs->size(); i++)
    {
       // delete imgs->at(i)->second;
        delete imgs->at(i);
    }

    delete fields;
    delete imgs;
    delete src_img_path;
    delete src_img;
}

void OpData::addImg(string name, Mat& img)
{
    Mat* img_cpy = new Mat;
    *img_cpy = img.clone();

    imgs->push_back(new pair<string, Mat*>(name, img_cpy));
    return;
}
void OpData::addField(string field, string value)
{
    fields->push_back(new pair<string, string>(field, value));
    return;
}
void OpData::addField(string field, int value)
{
    fields->push_back(new pair<string, string>(field, static_cast<ostringstream*>(&(ostringstream() << value))->str()));
    return;
}

void OpData::addField(string field, float value)
{
    fields->push_back(new pair<string, string>(field, static_cast<ostringstream*>(&(ostringstream() << value))->str()));
    return;
}

string OpData::getImgName()
{
    string img_name = *src_img_path;
    string::size_type ext_pos = img_name.find(".img");
    if(ext_pos == string::npos)
        return "";
    img_name.erase(ext_pos, img_name.length());

    string::size_type slash_pos = img_name.rfind("/");
    if(ext_pos != string::npos)
        img_name.erase(0, slash_pos + 1);

    return img_name;
}

string OpData::getImgDir()
{
    string img_dir = *src_img_path;

    string::size_type slash_pos = img_dir.rfind("/");
    if(slash_pos != string::npos)
        img_dir.erase(slash_pos, img_dir.length());

    return img_dir;
}

/*bool OpData::save(string dest_dir, string format)
{
    string img_name = getImgName();
    string img_dir = getImgDir();
    char data_dir_name[256];
    char data_file_name[256];

    if(!simple_output)
    {
        bool use_default_dir = (dest_dir == "")? true:false;

        if(use_default_dir)
        {
            strcpy(data_dir_name, img_dir.c_str());
        }
        else
        {
            strcpy(data_dir_name, dest_dir.c_str());
        }


        strcat(data_dir_name, "/");
        strcat(data_dir_name, img_name.c_str());
        strcat(data_dir_name, "/");
        strcpy(data_file_name, data_dir_name);
        strcat(data_file_name, img_name.c_str());
        strcat(data_file_name, ".dat");

        if(use_default_dir)
        {
            makeDir(img_dir + "/" + img_name);
            if(!src_img->empty())
                imwrite(img_dir + "/" + img_name + format, *src_img);
        }
        else
        {
            makeDir(dest_dir + "/" + img_name);
            if(!src_img->empty())
                imwrite(dest_dir + "/" + img_name + format, *src_img);
        }

        for(int i=0; i<imgs->size(); i++)
            imwrite(data_dir_name + imgs->at(i)->first + format, *imgs->at(i)->second);
    }
    else
        strcpy(data_file_name,(img_dir + "/" + img_name + ".dat").c_str());


    ofstream data_file;
    data_file.open(data_file_name, ofstream::out);
    if(!data_file.good())
    {
        return false;
    }
    else
        cout << "Data file created";

    for(int i=0; i<fields->size(); i++)
        data_file << fields->at(i)->first << ":" << fields->at(i)->second << endl;

    data_file.close();

    return true;
}*/

bool OpData::save(string dest_dir, string format)
{
    ofstream opstream((dest_dir + "/output.csv").c_str(), ios::out | ios::app);
    if(!opstream.is_open())
        return false;

    for(int i=0; i < fields->size(); i++)
    {
        opstream << fields->at(i)->second;
        if(i < fields->size()-1)
            opstream << ',';
    }
    opstream << endl;

    opstream.close();

    string img_name = getImgName();
    string img_dir = getImgDir();

    for(int i=0; i<imgs->size(); i++)
        imwrite(dest_dir + "/" + imgs->at(i)->first + format, *imgs->at(i)->second);

    return true;
}
