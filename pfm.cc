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
    delete _pf_manager;
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
        if(fileHandle.infile != NULL && fileHandle.outfile == NULL) {
            // this means the handle is associated with another file
            return -1; 
        }
        // link this new file handle to this opened file
        fileHandle.infile = new ifstream(fileName, ios::binary);
        fileHandle.outfile = new ofstream(fileName, ios::binary | ios::in | ios::out);
        return 0;
    } else {
        return -1;
    }
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if (fileHandle.infile == NULL && fileHandle.outfile == NULL) {
        // file is not associated with a file (error)
        return -1;
    }
    // check to see if the file is open and close it
    if (fileHandle.infile->is_open() && fileHandle.outfile->is_open()) {
        // the file exists and its open
        fileHandle.infile->close();
        fileHandle.outfile->close();
        delete fileHandle.infile;
        delete fileHandle.outfile;
        fileHandle.infile = NULL;
        fileHandle.outfile = NULL;  
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
    infile = NULL;
    outfile = NULL;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    if (infile != NULL && infile->is_open()) {
        infile->seekg(pageNum * PAGE_SIZE, ios::beg);
        infile->read(((char *) data), PAGE_SIZE);
        readPageCounter++;
        return 0;
    } else {
        return -1;
    }
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    if (outfile != NULL && outfile->is_open()) {
        outfile->seekp(pageNum * PAGE_SIZE, ios::beg);
        outfile->write(((char *) data), PAGE_SIZE);
        writePageCounter++;
        return 0;
    } else {
        return -1;
    }
}


RC FileHandle::appendPage(const void *data)
{
    if (outfile != NULL && outfile->is_open()) {
        outfile->seekp(0, ios::end);
        outfile->write(((char *) data), PAGE_SIZE);
        appendPageCounter++;
        numPages++;
        return 0;
    } else {
        return -1;
    }
}


unsigned FileHandle::getNumberOfPages()
{
    return numPages;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = readPageCounter;
    writePageCount = writePageCounter;
    appendPageCount = appendPageCounter;
	return 0;
}
