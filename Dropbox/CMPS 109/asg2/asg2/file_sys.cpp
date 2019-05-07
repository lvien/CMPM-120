// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

 //creates directory name by adding '/' to end
string valid_dir(const string& path)
{
    unsigned int pos = path.find_first_of("/");
    unsigned int size = path.size()-1;
    if (pos >= size) //slash char is omitted or at end
    {
        if (pos == size) //slash char is at end
            return path.substr(0,size); //remove slash
        return string(path); //return as is
    }
    return ""; //fail code
}

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

ostream& operator<< (ostream& outs, const inode& that)
{
    that.contents->print();
    return outs<<"";
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");

   // the root is a directory
   root = inode_ptr(new inode(file_type::DIRECTORY_TYPE));
   cwd = root; //cwd is the root
   cwd->contents->set_parent(root,"."); //itself is itself
   cwd->contents->set_parent(root,".."); //parent is itself
}

void inode_state::change_prompt(const wordvec& key)
{
    if (key.size() > 1) //user entered something
        prompt_ = key[1] + ' ';
}

const string& inode_state::prompt() const { return prompt_; }

void inode_state::print(const wordvec& filename)
{
    if (filename.size() > 1) //pathname specified
    {
        auto end = filename.size();
        for (unsigned int i = 1; i < end; i++)
        {
            inode_ptr path_file = find({"",filename[i]});
            if (path_file)
            {
                //is a directory
                if (path_file->type() == file_type::DIRECTORY_TYPE)
                {
                    string path = filename[i];
                    //no slash for special directories
                    if (path[0] != '.' && path != ".."
                            && path[0] != '/')
                        path = "/"+path;

                    cout<<filename[i]<<":"<<endl;
                    cout<<*path_file<<endl;
                }
                else
                    cout<<*path_file<<endl; //is a file
            }
            else
                complain()<<filename[i]+": No such directory exists\n";
        }
    }
    else
    {
        cout<<pwd()<<":"<<endl;
        cout<<*cwd<<endl;
    }
}

void inode_state::print_all(const wordvec& filename)
{
    if (filename.size() > 1) //pathname specified
    {
        auto end = filename.size();
        for (unsigned int i = 1; i < end; i++)
        {
            inode_ptr path_file = find({"",filename[i]});
            if (path_file)
            {
                //is a directory
                if (path_file->type() == file_type::DIRECTORY_TYPE)
                {
                    string path = filename[i];
                    //no slash for special directories
                    if (path[0] != '.' && path != ".."
                            && path[0] != '/')
                        path = "/"+path;

                    path_file->contents->print_all(path);
                }
                else
                    cout<<*path_file<<endl; //is a file
            }
            else
                complain()<<filename[i]+": No such directory"<<endl;
        }
    }
    else
        cwd->contents->print_all(pwd());

}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

string inode_state::pwd(wordvec pathname)
{
    if (pathname.size() == 0) //no argument
        pathname = cwd_path; //cwd_path

    string path = "";
    path += '/';
    if (pathname.size() > 0)
    {
        unsigned int end = pathname.size()-1;
        for (unsigned int i = 0; i < end; i++)
            path += pathname[i] + "/";
        path += pathname[end];
    }
    return path;
}

inode_ptr inode_state::find(const wordvec& filename)
{
    if (filename.size() > 1) //path is specified
    {
         //does path start with '/'?
        bool isroot = ((filename[1][0] == '/') ? true:false);
        //turn target path into wordvec
        wordvec target = split(filename[1], "/");
        if (target.size())
        {
            inode_ptr new_dir;
            if (isroot) //starting from root
            {
                //find target directory starting at root if possible
                new_dir = root->contents->find(target);
            }
            else //start at cwd
                //start search at cwd
                new_dir = cwd->contents->find(target);

            if (new_dir) //exists
                return new_dir;
            else
                return nullptr;
        }
    }
    return root;
}

void inode_state::chdir(const wordvec& cmd)
{
    inode_ptr file = find(cmd);

    if (file) //does file exist
    {
        if (file->type() == file_type::DIRECTORY_TYPE)
        {
            if (cmd.size() > 1) //more than one arg
                cwd_path = update_cwd(split(cmd[1],"/")); //update cwd
            else
                cwd_path = {}; //set cwd to root

            cwd = file; //set new cwd
        }
        else
            complain()<<cmd[1]+" is not a directory"<<endl;
    }
    else
        complain()<<cmd[1]<<": No such directory"<<endl;
}


size_t inode_state::size() const { return root->contents->size(); }

void inode_state::readfile(const wordvec& filename)
{
    if (filename.size() > 1) //filename(s) specified
    {
        auto size = filename.size(); //number of files to cat

        //cat all specified files
        for (unsigned int i = 1; i < size; i++)
        {
            string key = filename[i];
            inode_ptr file = cwd;
            //path specified
            wordvec path;
            if (key.find("/") < key.size())
            {
                path = {"", key};
                file = find(path);
            }
            else //file is within cwd
            {
                path = {"",filename[i]};
                file = find(path);
            }
            if (file)
            {
                if (file->contents->type() == file_type::PLAIN_TYPE)
                    cout<<file->contents->readfile()<<endl;
                else
                    complain()<<key<<" is a directory"<<endl;
            }
            else
                complain()<<key<<": No such file"<<endl;
        }
    }
    else
        complain()<<"No filename specified"<<endl;
}

wordvec inode_state::parent_dir (const string& newdata, string& target)
{
    //split into wordvec
    wordvec path = split(newdata,"/");
    if (path.size()) //not root
    {
        target = path.back(); //get target file
        path.pop_back();
        if (path.size())
            path = {"",pwd(path)}; //put path back as vec
        else
            path = {"",""};
        return path;
    }
    return {};
}

inode_ptr inode_state::pull_parent(const string& path)
{
    string filename;
    //make valid path
    wordvec dest = parent_dir(path,filename);
    return find(dest);
}

bool inode_state::writefile (const wordvec& newdata)
{
    if (newdata.size() > 1)
    {
        wordvec content; //contents of file
        string filename = newdata[1]; //filename
        inode_ptr parent = cwd;

        //it's a path
        if (newdata[1].find("/") < newdata[1].size())
        {
            wordvec path = parent_dir(newdata[1], filename);
            parent = pull_parent(newdata[1]);

            if (!parent) //parent dir not found
            {
                complain()<<newdata[1]
                   <<": No such directory"<<endl;
                return false;
            }
            if (parent->contents->type() != file_type::DIRECTORY_TYPE)
            {
                complain()<<newdata[1]
                   <<" is a file"<<endl;
                return false;
            }
        }
        //get contents
        if (newdata.size() > 2)
        {
            content = newdata;
            //remove cmd and pathname
            content.erase(content.begin());
            content.erase(content.begin());
        }
        parent = parent->contents->mkfile(filename);
        if (parent) //successfully made/overwritten
        {
            parent->contents->writefile(content);
            return true;
        }
        else
        {
            complain()<<filename<<" is a directory"<<endl;
            return false;
        }
    }
    complain()<<"No name specified"<<endl;
    return false;
}

bool inode_state::rmr(const wordvec &filename)
{
    if (filename.size() > 1)
    {
        inode_ptr file = cwd;
        wordvec path = cwd_path;
        string target = filename[1];
        //remove file within a path
        if (target.find("/") < target.size())
        {
            path = {"",target};
            parent_dir(filename[1], target);
            //pull file
            file = find(path);
        }
        else
            file = find(filename);

        //don't ever try to delete parent folders!
        //also if file exists
        if (file && (target != ".." && target != "."))
        {
            if (file == root)
            {
                complain()<<"Cannot delete root!"<<endl;
                return false;
            }
            //target is plain file
            if (file->contents->type() == file_type::PLAIN_TYPE)
            {
                //delete file from parent directory
                file = pull_parent(filename[1]);
                file->contents->remove(target);
            }
            else
            {
                file->contents->rmr(); //delete everything within
                //remove folder itself
                pull_parent(filename[1])->contents->remove(target);

            }
            return true;
        }
        else
            complain()<<target<<": No such file"<<endl;
    }
    else
        complain()<<"No name specified"<<endl;
    return false;
}

bool inode_state::remove (const wordvec &filename)
{
    if (filename.size() > 1)
    {
        inode_ptr file = cwd;
        wordvec path;
        string target = filename[1];
        //remove file within a path
        if (target.find("/") < target.size())
        {
            parent_dir(filename[1], target);
            //find parent directory
            //since remove can only be called from directory type
            file = pull_parent(filename[1]);
            if (!file)
            {
                complain()<<filename[1]<<" is an invalid path"<<endl;
                return false;
            }
        }

        //don't ever try to delete parent folders!
        if (target != ".." && target != ".")
        {
            if (file->contents->type() == file_type::DIRECTORY_TYPE)
            {
                file->contents->remove(target);
                return true;
            }
            complain()<<target<<" is a file"<<endl;
        }
        else
            complain()<<"Access denied"<<endl;
    }
    else
        complain()<<"No name specified"<<endl;
    return false;
}

void inode_state::mkdir (const wordvec& dirname)
{
    bool error = false;
    if (dirname.size() > 1)
    {
        unsigned int size = dirname.size(); //get total directories

        //this can make multiple directories
        for (unsigned int i = 1; i < size; i++)
        {
            //split path into vector
            string dest = dirname[i];
            inode_ptr file = cwd;
            inode_ptr newest = nullptr;

            if (dirname[i].find("/") < dirname[i].size())
            {
                parent_dir(dirname[i],dest);
                file = pull_parent(dirname[i]);
                if (file)
                    newest = file->contents->mkdir(dest);
                else
                {
                    error = true;
                    complain()<<dirname[i]<<" does not exist"<<endl;
                }
            }
            else
                newest = file->contents->mkdir(dest);

            //path was made
            if (newest) //path is valid
            {
                    newest->contents->set_parent(file, "..");
                    newest->contents->set_parent(newest,".");
            }
            else if (!error)
            {
                complain()<<dest
                   <<": Pathname already exists"<<endl;
            }
        }
    }
    else
        complain()<<"No name specified"<<endl;
}

//turn cmd into a wordvec target path
wordvec inode_state::update_cwd(const wordvec& cmd)
{
    wordvec path = cwd_path;
    unsigned int end = cmd.size();
    unsigned int size = path.size();
    string dir = "";
    for (unsigned int i = 0; i < end; i++)
    {
        dir = cmd[i]; //get directory name
        if (dir == ".."){
            if (size > 0)
                path.pop_back(); //go up one level from cwd
        }
        else if (dir != ".")
            path.push_back(dir); //go down one folder from cwd
        size--;
    }
    return path;
}


inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

void inode::print_all(const string &path)
{
    contents->print_all(path);
}

void inode::rmr()
{
    contents->rmr();
}

inode_ptr inode::find(const wordvec& path)
{
    return contents->find(path);
}

size_t inode::size() { return contents->size(); }

file_type inode::type() { return contents->type(); }


file_error::file_error (const string& what):
            runtime_error (what) {
}

file_type plain_file::type() const { return file_type::PLAIN_TYPE; }

size_t plain_file::size() const {
   size_t size = 0;
   auto end = data.end();
   for (auto i = data.begin(); i != end; i++)
   {
       size += i->size();
       if (i != end-1)
           size++;
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

void plain_file::set_parent(const inode_ptr&, const string&){
    throw file_error ("is a plain file");
}

const wordvec& plain_file::readfile() {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   data = words;
}

void plain_file::rmr()
{
    throw file_error("is a plain file");
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::find(const wordvec&) {
    throw file_error ("is a plain file");
}

void plain_file::print()
{
    for (auto i = data.begin(); i != data.end(); i++)
        cout<<*i<<" ";
}

void plain_file::print_all(const string &)
{
}

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() {
   throw file_error ("is a directory");
}

file_type directory::type() const
{
    return file_type::DIRECTORY_TYPE;
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   if (file_exists(filename))
   {
       inode_ptr file = dirents[filename]; //get file at index
       //is plain file
       if (file->type() == file_type::PLAIN_TYPE)
       {
           dirents[filename] = nullptr;
           dirents.erase(dirents.find(filename)); //erase from map
       }
       else if (file->type() == file_type::DIRECTORY_TYPE
                && file->size() <= 2)
       {
           dirents[filename] = nullptr;
           dirents.erase(dirents.find(filename)); //erase from map
       }
       else
           complain()<<filename<<": Directory is full"<<endl;
   }
   else
       complain()<<filename<<": No such file or directory"<<endl;
}

void directory::rmr ()
{
   auto end = dirents.end();
   file_type type;
   string index;

   //remove parent directories
   dirents["."] = nullptr;
   dirents[".."] = nullptr;
   dirents.erase(dirents.find("."));
   dirents.erase(dirents.find(".."));

   for (auto i = dirents.begin(); i != end; i++)
   {
       type = i->second->type();
       index = i->first;
       if (type == file_type::PLAIN_TYPE)
       {
           dirents[index] = nullptr;
           dirents.erase(dirents.find(index)); //erase from map
       }
       else
       {
           //recursively remove subdirectories
           dirents[index]->rmr();
           dirents[index] = nullptr;
           dirents.erase(dirents.find(index));
       }
   }
}

bool directory::file_exists(const string &name)
{
    return dirents.find(name) != dirents.end();
}

void directory::set_parent(const inode_ptr& me, const string& key)
{
    dirents[key] = me;
}

inode_ptr directory::mkdir (const string& dirname) {
    // directory name does not exist
    if (!file_exists(dirname))
    {
        inode_ptr temp(new inode(file_type::DIRECTORY_TYPE));
        //create new directory within map
        dirents[dirname] = temp;

        return temp;
    }
   DEBUGF ('i', dirname);
   return nullptr;
}

inode_ptr directory::mkfile (const string& filename) {
    // filename doesn't exist
    if (!file_exists(filename))
    {
        //make new plain file
        inode_ptr temp(new inode(file_type::PLAIN_TYPE));
        dirents[filename] = temp; //create new plain file within map

        return temp; //return newly created file
    }
    else
    {
        if (dirents[filename]->type() == file_type::PLAIN_TYPE)
            //return existing file for overwriting
            return dirents[filename];
    }
   DEBUGF ('i', filename);
   return nullptr; //directory has same name
}

inode_ptr directory::find(const wordvec& path) { //recursive function
    if (path.size() < 1) //no path specified
        return nullptr;

    string key = path[0]; //get name of next directory
    wordvec new_path (path);
    //remove name of first directory
    new_path.erase(new_path.begin());

    if (file_exists(key)) //name exists
    {
        if (new_path.size() > 0) //still more directories to go
        {
            //path is a directory
            if (dirents[key]->type() == file_type::DIRECTORY_TYPE)
                return dirents[key]->find(new_path); //recursive search
        }
        else
            return dirents[key]; //this is the final directory
    }
    return nullptr; //not found
}

void directory::print()
{
    //loop through table
    for (auto i = dirents.begin(); i != dirents.end(); i++)
    {
        auto index = i->first; //get directory
        //print size and directory name
        cout<<"     "<<dirents[index]->get_inode_nr()
           <<"       "<<dirents[index]->size()<<"  "<<index;

        //is directory type
        if (dirents[index]->type() == file_type::DIRECTORY_TYPE)
        {
            //not the parent folders
            if (index != "." && index != "..")
                cout<<"/"; //indicate it's a directory
        }
        cout<<endl;
    }
}

void directory::print_all(const string& path)
{
    cout<<path<<":"<<endl; //display path
    print(); //print this directory first
    //go through all files in directory
    for (auto i = dirents.begin(); i != dirents.end(); i++)
    {
        auto dir = dirents[i->first];
        //is a directory
        if (dir->type() == file_type::DIRECTORY_TYPE)
        {
            //don't go into parent folders
            if (i->first != "." && i->first != "..")
                //print slash between only if it is not root
                dir->print_all(path+((path=="/")?"":"/")+i->first);
        }
    }
}
