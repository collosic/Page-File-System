#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}

    
PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const string &fileName)
{
    // we must first test to see if the file already exists
    ifstream test_file(fileName);
    if (test_file.good()) {
        return -1;
    } else {
        ofstream new_file(fileName, ios::binary);
        if (new_file.is_open()) {
            new_file.close();
            return 0;
        } else {
            return -1;
        }
    }
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    if (!std::remove(fileName.c_str())) {
        return 0;
    } else {
        return -1;
    }
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    ifstream file(fileName);
    // check if the file exists
    if (file.good() && file.is_open()) {
        file.close();
        if(fileHandle.file != NULL) {
            // this means the handle is associated with another file
            return -1; 
        }
        // link this new file handle to this opened file
        fileHandle.file = new fstream(fileName, ios::binary);
        return 0;
    } else {
        return -1;
    }
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if (fileHandle.file == NULL) {
        // file is not associated with a file (error)
        return -1;
    }
    // check to see if the file is open and close it
    if (fileHandle.file->is_open()) {
        // the file exists and its open
        fileHandle.file->close();
        return 0;
    }
    return -1;
}


FileHandle::FileHandle()
{
	readPageCounter = 0;
	writePageCounter = 0;
	appendPageCounter = 0;
    numPages = 0;
    file = NULL;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
    return numPages;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
	return -1;
}
