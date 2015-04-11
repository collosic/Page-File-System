
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
    std::string s;
    int numNullBytes = ceil((double)recordDescriptor.size() / CHAR_BIT);
    int offset = numNullBytes;
    for (auto it = recordDescriptor.begin(); it != recordDescriptor.end(); ++it) {
        // test to see if the field is NULL
        int i = it - recordDescriptor.begin();
        s += it->name + ": ";
        if (isFieldNull(data, numNullBytes, i)) {
            s += "NULL\t";
        } else {
            s += extractType((char *)data + offset,  it->type, it->length);
        } 

    }
    return 0;
}


bool isFieldNull(const void *data, int bytes, int i) {
    // create an bitmask to test if the field is null
    unsigned char *bitmask = (unsigned char*) malloc(bytes);
    *bitmask = 0x1;
    *bitmask <<= (bytes * CHAR_BIT) - 1;
    *bitmask >>= i;
    
    // extract the NULL fields indicator from the data
    unsigned char *nullField = (unsigned char*) malloc(bytes);
    memcpy(nullField, (char *) data, bytes);
    bool retVal = (*bitmask & *nullField) ? true : false;
    free(bitmask);
    free(nullField);
    return retVal;
}

std::string extractType(void *data, AttrType t, AttrLength l) {
    unsigned a = l;
}
