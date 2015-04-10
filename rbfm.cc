
#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    pfm = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
    return pfm->createFile(fileName);
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    return pfm->destroyFile(fileName);
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    return pfm->openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {

    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    // Go through all the attributes and print the data
    int numFields = ceil((double)recordDescriptor.size() / CHAR_BIT);
    for (auto it = recordDescriptor.begin(); it != recordDescriptor.end(); ++it) {
        // test to see if the field is NULL
        int i = it - recordDescriptor.begin();
        if (isFieldNull(data, numFields, i)) {
            std::cout << it->name + ": " << "NULL\t";
        } else {
            

        } 

    }
    return 0;
}


bool isFieldNull(const void *data, int fields, int i) {
    // create an bitmaks to test if the field is null
    char *bitmask = new char[fields * CHAR_BIT];
    *bitmask = 0x1;
     

}

