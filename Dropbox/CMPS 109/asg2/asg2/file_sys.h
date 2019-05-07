// $Id: file_sys.h,v 1.6 2018-06-27 14:44:57-07 - - $

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

string valid_dir(const string& path);
#include "util.h"

// inode_t -
//    An inode is either a directory or a plain file.

template <typename K, typename V>
ostream& operator<< (ostream& outs, const map<K,V>& that){
    const auto end = that.end();
    for (auto i = that.begin(); i != end; i++)
        outs<<i->first<<": "<<i->second<<endl;
    return outs;
}

enum class file_type {PLAIN_TYPE, DIRECTORY_TYPE};
class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
ostream& operator<< (ostream&, file_type);

// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.

class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_ptr root {nullptr}; //headptr
      inode_ptr cwd {nullptr}; //current directory
      string prompt_ {"% "};
      wordvec cwd_path; //cwd path as wordvec
   public:
      string pwd(wordvec pathname = {});
      void print(const wordvec& filename);
      void print_all(const wordvec& filename);
      inode_state (const inode_state&) = delete; // copy ctor
      inode_state& operator= (const inode_state&) = delete; // op=
      inode_state();
      const string& prompt() const;
      wordvec parent_dir (const string &newdata, string& target);
      void change_prompt(const wordvec& key);
      size_t size() const;
      void readfile(const wordvec &filename);
      bool writefile(const wordvec& newdata);
      bool rmr (const wordvec& filename);
      bool remove(const wordvec& filename);
      inode_ptr pull_parent(const string& path);
      void mkdir(const wordvec& dirname);
      wordvec update_cwd(const wordvec& cmd);
      inode_ptr find(const wordvec& filename);
      void chdir(const wordvec& cmd);
};

// class inode -
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//    

class inode {
   friend class inode_state;
   private:
      static int next_inode_nr;
      int inode_nr;
      base_file_ptr contents;
      void set_parent(const inode_ptr& me, const string &key);
   public:
      friend ostream& operator<< (ostream& outs, const inode& that);
      void print_all(const string &path);
      inode (file_type);
      size_t size();
      file_type type();
      void rmr ();
      inode_ptr find(const wordvec& path);
      int get_inode_nr() const;
};


// class base_file -
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.

class file_error: public runtime_error {
   public:
      explicit file_error (const string& what);
};

class base_file {
   protected:
      base_file() = default;
   public:
      virtual void print() = 0;
      virtual void print_all(const string &) = 0;
      virtual ~base_file() = default;
      virtual void set_parent(const inode_ptr&, const string &) = 0;
      base_file (const base_file&) = delete;
      base_file& operator= (const base_file&) = delete;
      virtual size_t size() const = 0;
      virtual const wordvec& readfile() = 0;
      virtual void writefile (const wordvec& newdata) = 0;
      virtual void rmr () = 0;
      virtual void remove (const string& filename) = 0;
      virtual inode_ptr mkdir (const string& dirname) = 0;
      virtual inode_ptr mkfile (const string& filename) = 0;
      virtual inode_ptr find(const wordvec&) = 0;
      virtual file_type type() const = 0;
};

// class plain_file -
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
// writefile -
//    Replaces the contents of a file with new contents.

class plain_file: public base_file {
   private:
      wordvec data;
   public:
      virtual void print() override;
      virtual void print_all(const string &) override;
      virtual void set_parent(const inode_ptr&, const string &) override;
      virtual size_t size() const override;
      virtual const wordvec& readfile() override;
      virtual void writefile (const wordvec& newdata) override;
      virtual void rmr () override;
      virtual void remove (const string& filename) override;
      virtual inode_ptr mkdir (const string& dirname) override;
      virtual inode_ptr mkfile (const string& filename) override;
      virtual inode_ptr find(const wordvec&) override;
      virtual file_type type() const override;
};

// class directory -
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an file_error if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and 
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory: public base_file {
private:
    // Must be a map, not unordered_map, so printing is lexicographic
    using dir_map = map<string,inode_ptr>;
    dir_map dirents;
public:
    virtual void set_parent(const inode_ptr& me, const string &key) override;
    virtual void print() override;
    virtual void print_all(const string &path) override;
    bool file_exists(const string& name);
    virtual size_t size() const override;
    virtual const wordvec& readfile() override;
    virtual void writefile (const wordvec& newdata) override;
    virtual void rmr () override;
    virtual void remove (const string& filename) override;
    virtual inode_ptr mkdir (const string& dirname) override;
    virtual inode_ptr mkfile (const string& filename) override;
    virtual inode_ptr find(const wordvec& path) override;
    virtual file_type type() const override;
};

#endif

