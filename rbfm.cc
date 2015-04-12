
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
    // lets determine if we need to append a new page or just write to a page
    int length = getRecordSize(data, recordDescriptor);

    // findOpenSlot() will search for an open slot in the slot directory 
    // if it finds one it will update the rid and return the new offset for the record
    int newOffset = findOpenSlot(fileHandle, length, rid); 
    if (newOffset == -1) {
        // first thing we need to do is write the current page to file and free up the memory
        // only if there is 1 or more pages in the file Handle
        if (fileHandle.getNumberOfPages() != 0 && fileHandle.currentPage != NULL) {
            if (fileHandle.writePage(fileHandle.getNumberOfPages() - 1, fileHandle.currentPage)) {
                // error writing to file
                return -1;
            }
            free(fileHandle.currentPage);
        }
        
        // we need to append a new page
        fileHandle.currentPage = malloc(PAGE_SIZE);
        void *newPage = fileHandle.currentPage;
        memset(newPage, 0, PAGE_SIZE);
        
        // update the RID
        rid.pageNum = fileHandle.currentPageNum = fileHandle.getNumberOfPages();
        rid.slotNum = 0;
        
        // Now let's add the new record
        int length = getRecordSize(data, recordDescriptor);
        setUpNewPage(newPage, data, length);
        fileHandle.appendPage(newPage);
        return 0;
    } else {
        
        // copy the record the current page
        memcpy((char *) fileHandle.currentPage + newOffset, (char *) data, length);
        
        // update the number of records and free space
        int numRecords;
        memcpy(&numRecords, (char *) fileHandle.currentPage + N_OFFSET, sizeof(int));
        numRecords++;
        memcpy((char *) fileHandle.currentPage + N_OFFSET, &numRecords, sizeof(int));

        int freeSpace;
        memcpy(&freeSpace, (char *) fileHandle.currentPage + F_OFFSET, sizeof(int));
        freeSpace = freeSpace - (length + SLOT_SIZE);
        memcpy((char *) fileHandle.currentPage + F_OFFSET, &freeSpace, sizeof(int));
        
        // now we need to enter in the slot directory entry
        int slotEntryOffset = N_OFFSET - (numRecords * SLOT_SIZE); 
        memcpy((char *) fileHandle.currentPage + slotEntryOffset, &newOffset, sizeof(int));
        memcpy((char *) fileHandle.currentPage + slotEntryOffset + sizeof(int), &length, sizeof(int));
        return 0; 
    }
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    // First generate a page to transfer over from the file, using the pageNum
    if (rid.pageNum != fileHandle.currentPageNum) {
        fileHandle.currentPage = malloc(PAGE_SIZE);
        fileHandle.currentPageNum = rid.pageNum;
        fileHandle.readPage(rid.pageNum, fileHandle.currentPage);
    }

    void *page = fileHandle.currentPage;
    // we now need to get the page offset and data offset

    int offset;
    int length;
    getSlotFile(rid.slotNum, page, &offset, &length);

    // Now copy the entire contents into data
    memcpy((char *) data, (char *) page + offset, length);
    return 0;
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
        s += isFieldNull(data, numNullBytes, i) ? "NULL" : 
                                extractType(data, &offset, it->type, it->length); 
        s += '\t';
    }
    cout << s << endl;
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

std::string extractType(const void *data, int *offset, AttrType t, AttrLength l) {
    if (t == TypeInt) {
        int value; 
        memcpy(&value, (char *) data + *offset, sizeof(int));
        *offset += sizeof(int);
        return std::to_string((long long) value);
    } else if (t == TypeReal) {
        float val;
        memcpy(&val, (char *) data + *offset, sizeof(float));
        *offset += sizeof(float);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << val;
        return ss.str();
    } else if (t == TypeVarChar) {
        // first extract the length of the char
        int varCharLength; 
        memcpy(&varCharLength, (char *) data + *offset, sizeof(int));
        
        // now generate a C string with the same length plus 1
        *offset += sizeof(int);
        char s[varCharLength];
        memcpy(&s, (char *) data + *offset, varCharLength);

        std::string str(s);
        *offset += varCharLength;
        return str;
    } else {
        // this shouldn't happen since we assume all incoming data is correct
        return "ERROR EXTRACTING"; 
    }
}


int getRecordSize(const void *data, const vector<Attribute> &descriptor) {
    int dataOffset = 0;
    // Copy null field 
    int numNullBytes = ceil((double)descriptor.size() / CHAR_BIT);
    dataOffset += numNullBytes;
    
    for (auto it = descriptor.begin(); it != descriptor.end(); ++it) {
        if (it->type == TypeInt) {
            dataOffset += sizeof(int);
        } else if (it->type == TypeReal) {
            dataOffset += sizeof(float);
        } else if (it->type == TypeVarChar) {
            int varCharLength;
            memcpy(&varCharLength, (char *) data + dataOffset, sizeof(int));
            dataOffset += sizeof(int) + varCharLength;
        } else {
            // this should not happen since we assume all data coming it is always correct, for now
        }
    } // end of for loop
    return dataOffset; 
}

void getSlotFile(int slotNum, const void *page, int *offset, int *length) {
    // first lets get the slot offset 
    int location = PAGE_SIZE - (((slotNum + 1) * SLOT_SIZE) + SLOT_SIZE);
    memcpy(offset, (char *) page + location, sizeof(int)); 
    memcpy(length, (char *) page + location + sizeof(int), sizeof(int));
}


int findOpenSlot(FileHandle &handle, int size, RID &rid) {
    // first we need to check and see if the current page has available space
    int pageNum = handle.getNumberOfPages() - 1;
    if (pageNum < 0) {
        // this means we have no pages in a file and must generate a page
        return -1;
    }

    // int sizeOfFile = PAGE_SIZE * handle.getNumberOfPages();
    void *page = handle.currentPage; 
    
    int freeSpace;
    memcpy(&freeSpace, (char *) page + F_OFFSET, sizeof(int));
    if (freeSpace > (size + SLOT_SIZE)) {
        // the current page has enough space to fit a new record
        rid.pageNum = pageNum;
        return scanSlotDirectoryForFreeSpace(page, rid);
    }
     
    /*    
    for (int i = 0; i < (sizeOfFile - 1); i++) {
        // check each page for available space
        handle.readPage(i, page);
        
        // extract the free space and compare to the size of the new record
        int freeSpace;
        memcpy(&freeSpace, (char *) page + F_OFFSET, sizeof(int));
        
        // if the free space is big enough to accomodate the new record then stick it in.
        if (freeSpace > (size + SLOT_SIZE)) {
            rid.pageNum = i;
            return scanSlotDirectoryForFreeSpace(page, rid);  
        }
    } */
    // if we get here than no space was available and we need to append
    return -1;
}


int scanSlotDirectoryForFreeSpace(const void *data, RID &rid) {
    // here we just need to add up all the lengths of the records and that will
    // give us the offset for the record.  We also need to add a new slot directory entry
    int numRecords;
    memcpy(&numRecords, (char *) data + N_OFFSET, sizeof(int));
    rid.slotNum = numRecords; 
    
    int newOffset = 0;
    int slotOffset = PAGE_SIZE - ((numRecords * SLOT_SIZE) + SLOT_SIZE);
    for (int i = 0; i < numRecords; i++) {
        int _temp;
        memcpy(&_temp, (char *) data + slotOffset + sizeof(int), sizeof(int));
        newOffset += _temp;
        slotOffset += SLOT_SIZE;
    }
    return newOffset;
}

void setUpNewPage(const void *newPage, const void *data, int length) {
    // for the next part we only want to know the length of the record and then copy all it's contents over
    memcpy((char *) newPage, (char *) data, length);
        
    // we need put 1 page in the slot directory meta data
    int numRecords = 1;
    memcpy((char *) newPage + N_OFFSET, &numRecords, sizeof(int));

    // next we need to add slot 1 meta data, each slot is 2 ints (8 bytes) in length to fit the offset and length
    int slotOneOffset = N_OFFSET - (2 * sizeof(int));;
        
    // enter the offset first which is zero because its the first record in a page
    int offset = 0;
    memcpy((char *) newPage + slotOneOffset, &offset, sizeof(int));
    slotOneOffset += sizeof(int);
    memcpy((char *) newPage + slotOneOffset, &length, sizeof(int));

    // now we need to enter in the free space
    int freeSpace = PAGE_SIZE - (length + (numRecords * SLOT_SIZE) + SLOT_SIZE);
    memcpy((char *) newPage + F_OFFSET, &freeSpace, sizeof(int));
}


