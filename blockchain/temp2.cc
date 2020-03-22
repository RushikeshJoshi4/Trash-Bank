#include "rbfm.h"
#include <iostream>
#include <cstddef>
#include <bitset>
#include <cmath>


using namespace std;



RecordBasedFileManager &RecordBasedFileManager::instance() {
    static RecordBasedFileManager _rbf_manager = RecordBasedFileManager();
    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager(){
    pagedFileManager = &PagedFileManager::instance();

};

RecordBasedFileManager::~RecordBasedFileManager() = default;

RecordBasedFileManager::RecordBasedFileManager(const RecordBasedFileManager &) = default;

RecordBasedFileManager &RecordBasedFileManager::operator=(const RecordBasedFileManager &) = default;

//functions pertaining to IO operations

/* -------------------------------------------------------------------------- */


/**
 * update page information
 * param: pageData, firstAvailableSlotIndex, slotNumRecord
 * update page information at the header of page(the end of page).
 */
RC RecordBasedFileManager::updatePageInfo(void * pageData,
                                          const unsigned short int firstAvailableSlotIndex,
                                          const unsigned short int slotNumRecord) {
    memcpy((byte*) pageData + PAGE_SIZE - 2 * sizeof(unsigned short int), &firstAvailableSlotIndex, sizeof(unsigned short int));
    memcpy((byte*) pageData + PAGE_SIZE - sizeof(unsigned short int),
           &slotNumRecord, sizeof(unsigned short int));
    return 0;
}


/**
 * initial page information for blank page
 * return: pageData;
 * return a signed page data
 */
RC RecordBasedFileManager::initPageInfo(void * pageData) {
    unsigned short int firstAvailableIndex = 0;
// attention the slot num is 1, because it includes header record
    unsigned short int slotNum = 1;
    this->updatePageInfo(pageData, firstAvailableIndex, slotNum);
    return 0;
}


/**
 * append signed page
 * param: fileHandle
 */
RC RecordBasedFileManager::appendSignedPageInFile(FileHandle &fileHandle) {
    void *data = malloc(PAGE_SIZE);
    if (this->initPageInfo(data) != 0) {
        free(data);
        cout << "!!!fail to append signed page" << endl;
        return -1;
    }
    if (fileHandle.appendPage(data) != 0) {
        free(data);
        cout << "!!!fail to append signed page" << endl;
        return -1;
    }
    free(data);

    return 0;
}

/* -------------------------------------------------------------------------- */


RC RecordBasedFileManager::createFile(const std::string &fileName) {


    if (pagedFileManager->createFile(fileName) != 0)
        return -1;

// append one page;
    FileHandle fileHandle;
    if (this->openFile(fileName, fileHandle) != 0)
        return -1;
    if (this->appendSignedPageInFile(fileHandle) != 0)
        return -1;
    if (this->appendSignedPageInFile(fileHandle) != 0)
        return -1;


    return 0;
}

RC RecordBasedFileManager::destroyFile(const std::string &fileName) {
    return -1;
}

RC RecordBasedFileManager::openFile(const std::string &fileName, FileHandle &fileHandle) {
    return pagedFileManager->openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return -1;
}


//functions pertaining to read record

/* -------------------------------------------------------------------------- */

/**
 * data to format data and calculate data size
 * convert data to format data for 0(1) field access in the future and by the way calculate data size
 */
RC RecordBasedFileManager::data2FormatData(
        const vector<Attribute> &recordDescriptor, const void *data,
        void *format_data, unsigned short int &data_size) {

    unsigned n_fields = recordDescriptor.size();

    if (n_fields == 0) {
        cout << "!!!empty record descriptor" << endl;
        return -1;
    }

    unsigned null_fields_bits = ceil((double) n_fields / CHAR_BIT);


    unsigned char * null_fields_indicator = new unsigned char [null_fields_bits];
    memcpy(null_fields_indicator, data, null_fields_bits);

    unsigned field_offset = sizeof(RID)
                              + (n_fields + 1) * sizeof(unsigned short int)
                              + null_fields_bits;


    unsigned short int* field_start = new unsigned short int[n_fields + 1];
    field_start[0] = field_offset;
    unsigned short int data_offset = null_fields_bits;

    for (unsigned i = 0; i < n_fields; i++) {

        bool null_bit = null_fields_indicator[i / CHAR_BIT] & (1 << (CHAR_BIT - 1 - i % CHAR_BIT));

        if (!null_bit) { // not null
            AttrType type = recordDescriptor[i].type;
            if (type == TypeInt) {
                data_offset += sizeof(int);
                field_offset += sizeof(int);
            } else if (type == TypeReal) {
                data_offset += sizeof(float);
                field_offset += sizeof(float);
            } else {
                int len = 0;
                memcpy(&len, (byte*) data + data_offset, sizeof(int));
                data_offset += sizeof(int) + len;
                field_offset += sizeof(int) + len;
            }
        }
        field_start[i + 1] = field_offset;


    }


    unsigned short int offset = 0;
    RID rid;
    rid.pageNum = 0;
    rid.slotNum = 0;

    // copy first rid
    memcpy((byte*) format_data + offset, &rid.pageNum, sizeof(rid.pageNum));
    offset += sizeof(rid.pageNum);
    memcpy((byte*) format_data + offset, &rid.slotNum, sizeof(rid.slotNum));
    offset += sizeof(rid.slotNum);

    memcpy((byte*) format_data + offset, field_start,
           sizeof(unsigned short int) * (n_fields + 1));
    offset += sizeof(unsigned short int) * (n_fields + 1);

    // position offset is the truly length of data, including null indicator and fields' data.
    memcpy((byte*) format_data + offset, data, field_offset - offset);
    offset = field_offset;

    // copy last rid
    memcpy((byte*) format_data + offset, &rid.pageNum, sizeof(rid.pageNum));
    offset += sizeof(rid.pageNum);
    memcpy((byte*) format_data + offset, &rid.slotNum, sizeof(rid.slotNum));
    offset += sizeof(rid.slotNum);

    // return dataSize
    data_size = offset;

    delete[] null_fields_indicator;
    return 0;
}


RC RecordBasedFileManager::insertFormatDataToFile(FileHandle fileHandle,
                                                  const void * formatData, const unsigned short int &dataSize, RID &rid) {
// find available page
// suppose the following step would insert data and slot
    if (this->findAvailablePageInFile(fileHandle, dataSize, rid.pageNum) != 0) {
        cout
                << "!!!fail to find suitable insert position in RecordBasedFileManager::insertRecord"
                << endl;
        return -1;
    }

// get page data
    void * pageData = malloc(PAGE_SIZE);
    if (fileHandle.readPage(rid.pageNum, pageData) != 0) {
        free(pageData);
        cout << "!!!fail to read page in RecordBasedFileManager::insertRecord"
             << endl;
        return -1;
    }

// insert format data into page data
    if (this->insertRecordToPage(pageData, formatData, dataSize, rid.slotNum)
        != 0) {
        free(pageData);
        cout
                << "!!!fail to insert record into page in RecordBasedFileManager::insertRecord"
                << endl;
        return -1;
    }

//	cout << "slotNum:" << rid.slotNum << endl;

// write back page data to file
    if (fileHandle.writePage(rid.pageNum, pageData) != 0) {
        free(pageData);
        cout
                << "!!!fail to write back page data in RecordBasedFileManager::insertRecord."
                << endl;
    }

    free(pageData);
    return 0;
}



RC RecordBasedFileManager::findAvailablePageInFile(FileHandle &fileHandle,
                                                   const unsigned short int dataSize, unsigned &pageNum) {

    PageNum totalPage = fileHandle.getNumberOfPages();
    PageNum currentPageIndex = 0;
    // insert page should start from second page
    if (totalPage < 2) {
        cout << "!!!page not initial." << endl;
        return -1;
    }
    // a more easier strategy
    currentPageIndex = (totalPage - 1);
    // try to find slot without append from
    bool currentPageAvailable = false;
    void * pageData = malloc(PAGE_SIZE);
    while(currentPageIndex < totalPage){
        if(fileHandle.readPage(currentPageIndex, pageData)!=0){
            free(pageData);
            return -1;
        }

        if(this->isCurrentPageAvailableInsert(pageData, dataSize, currentPageAvailable)!=0){
            free(pageData);
            return -1;
        }

        if(currentPageAvailable){
            pageNum = currentPageIndex;

            free(pageData);
            return 0;
        }

        currentPageIndex++;
    }


    // else
    // append next page
    if (this->appendSignedPageInFile(fileHandle) != 0) {
        free(pageData);
        cout
                << "!!!fail to append new signed page in RecordBasedFileManager::findSlotPosition."
                << endl;
        return -1;
    }

    // read page
    if(fileHandle.readPage(currentPageIndex, pageData)!=0){
        free(pageData);
        return -1;
    }

    // check if page is available
    if(this->isCurrentPageAvailableInsert(pageData, dataSize, currentPageAvailable)!=0){
        free(pageData);
        return -1;
    }
    if(currentPageAvailable){
        // return pageNum
        pageNum = currentPageIndex;
        free(pageData);
        return 0;
    }

    free(pageData);
    cout << "!!!fail to find available page" << endl;
    return -1;
}




/* slot management in page level*/
/**
 * is current page available to insert data
 */
RC RecordBasedFileManager::isCurrentPageAvailableInsert(const void * pageData,
                                                        const unsigned short int &dataSize, bool &currentPageAvailable) {
    // attention: currentSize include slotInfo size
    // when update both currentSize and oldSize do not need include slotInfo
    // when insert currentSize need include slotInfo

    unsigned short int firstAvailableSlotIndex;
    unsigned short int slotNumRecord;
    if (this->getPageInfoByData(pageData, firstAvailableSlotIndex,
                                slotNumRecord) != 0) {
        return -1;
    }
    // attention: insert formatData also need insert one slotInfo
    if (firstAvailableSlotIndex * sizeof(byte)
        + (slotNumRecord + 1) * 2 * sizeof(unsigned short int)
        + dataSize < PAGE_SIZE) {
//		cout << "can insert:" << endl;
//		cout << "currentData:" << dataSize << endl;
//		cout << "firstAvailableSlotIndex" << firstAvailableSlotIndex << endl;
//		cout << "slotNumRecord" << slotNumRecord << endl;
        currentPageAvailable = true;
    } else {
        currentPageAvailable = false;
    }
    return 0;
}

/**
 * get page information by page data
 * param: format page data
 * return: slotNumRecord,firstAvailableSlotIndex
 * get the page information: slot number record, first avaiable slot index
 */
RC RecordBasedFileManager::getPageInfoByData(const void * pageData,
                                             unsigned short int &firstAvailableSlotIndex,
                                             unsigned short int &slotNumRecord) {
    if (pageData == NULL) {
        cout << "!!!page data is null, page data cannot be decoded" << endl;
        return -1;
    }
    memcpy(&firstAvailableSlotIndex,
           (byte*) pageData + PAGE_SIZE - 2 * sizeof(unsigned short int),
           sizeof(unsigned short int));
    memcpy(&slotNumRecord,
           (byte*) pageData + PAGE_SIZE - sizeof(unsigned short int),
           sizeof(unsigned short int));

    return 0;
}



/**
 * insertRecordToPage
 * param: fileHanle, data, dataSize, rid
 * save record into page according to rid
 * problem: not use the firstAvailableSlotIndex param from before method, considering concurrent visit
 * First: read page information
 * Second: find available slot number, if not, make a new one
 * Third: insert information into slot, and update page information
 * Forth: insert record
 */

RC RecordBasedFileManager::insertRecordToPage(void * pageData, const void *data,
                                              const unsigned short int &dataSize, unsigned short &slotNum) {

    // get page information
    unsigned short int firstAvailableSlotIndex;
    unsigned short int slotNumRecord;
    if (this->getPageInfoByData(pageData, firstAvailableSlotIndex,
                                slotNumRecord) != 0) {
        cout << "!!!fail to get page info, in insertRecord2Slot." << endl;
        return -1;
    }

    /* TODO: find available slot number */
    // find an available slot
    unsigned short int currentSlotNum = 2;
    unsigned short int currentSlotStartIndex;
    unsigned short int currentSlotOffset;
    bool isOldSlot = false; // flag current slot is an old slot
    while (currentSlotNum <= slotNumRecord) {
        // get slot offset information
        if (this->getSlotInfo(pageData, currentSlotNum, currentSlotStartIndex,
                              currentSlotOffset) != 0) {
            cout
                    << "!!!fail to get slot info in RecordBasedFileManager::insertRecordToPage"
                    << endl;
            return -1;
        }
        if (currentSlotOffset == 0) {
            isOldSlot = true;
            break;
        }
        currentSlotNum++;
    }

    // update slot num
    slotNum = currentSlotNum;

    // if insert in new slot
    if (!isOldSlot) {
//		cout << "insert in new slot" << endl;
        if(this->insertRecordToPageAtNextSlot(pageData, data, dataSize, slotNum)!=0){
            return -1;
        }
        return 0;
    }

    // else if the slot is old, update old slot
    if(this->updateRecordToPageAtSlotNumber(pageData, data, dataSize, slotNum)!=0){
        return -1;
    }
    return 0;


}


/**
 * get slot information
 * param: pageData, slotNum
 * return startSlotIndex, slotOffsetNumber
 * get the slot information according to slot number in the page
 */
RC RecordBasedFileManager::getSlotInfo(const void * pageData,
                                       const unsigned short int slotNum, unsigned short int &slotStartIndex,
                                       unsigned short int &slotOffsetNumber) {
    // check slotNum
    if(slotNum < 2){
        cout << "!!!slotNum lower than bound getSlotInfo."<< endl;
        return -1;
    }
    unsigned short int offset = PAGE_SIZE - 2 * sizeof(unsigned short int) * (slotNum);
    memcpy(&slotStartIndex, (byte*) pageData + offset,
           sizeof(unsigned short int));
    memcpy(&slotOffsetNumber,
           (byte*) pageData + offset + sizeof(unsigned short int),
           sizeof(unsigned short int));
    return 0;
}



/**
 * insert record to page at fixed slot number
 * currentSlotNum must be the next of currentSlot in page
 */
RC RecordBasedFileManager::insertRecordToPageAtNextSlot(void * pageData,
                                                        const void * formatData, const unsigned short int &dataSize,
                                                        const unsigned short int &currentSlotNum) {

    // read page information
    unsigned short int firstAvailableSlotIndex;
    unsigned short int slotNumRecord;
    if (this->getPageInfoByData(pageData, firstAvailableSlotIndex,
                                slotNumRecord) != 0) {
        cout << "!!!fail to get page info, in RecordBasedFileManager::insertRecordToPageAtSlotNumber." << endl;
        return -1;
    }

    // slotNumRecord update
    slotNumRecord++;
    // check currentSlotNum
    if(currentSlotNum != slotNumRecord){
        cout << "!!!currentSlotNum is not the next of slotNumRecord" << endl;
        return -1;
    }

    // copy data into pageData
    memcpy((byte*)pageData + firstAvailableSlotIndex, formatData, dataSize);

    // update slotInfo
    if (this->updateSlotInfo(pageData, currentSlotNum, firstAvailableSlotIndex,
                             dataSize) != 0) {
        cout << "!!!fail to update slot information" << endl;
        return -1;
    }

//	cout << "record insert start:"<< firstAvailableSlotIndex<< "dataSize:"<< dataSize << endl;

    // update firstAvailableIndex and slotNumRecord
    firstAvailableSlotIndex += dataSize;
    if (this->updatePageInfo(pageData, firstAvailableSlotIndex, slotNumRecord)
        != 0) {
        cout << "!!!fail to update page information" << endl;
        return -1;
    }
//	cout << "current slotNum" << slotNumRecord << "firstAvailableSlotIndex" << firstAvailableSlotIndex << endl;
    return 0;
}




/**
 * update slot information
 * param: pageData, slotPosition, newSlotStartIndex, newSlotOffsetNumer
 * update the pointed slot information: start index and offset
 */
RC RecordBasedFileManager::updateSlotInfo(void *pageData,
                                          const unsigned short int slotNum,
                                          const unsigned short int slotStartIndex,
                                          const unsigned short int slotOffset) {
    if (slotNum < 2) {
        cout
                << "!!!cannot update the first slot information, because it is related to the whole page in RecordBasedFileManager::updateSlotInfo"
                << endl;
        return -1;
    }
    unsigned short int offset = PAGE_SIZE
                                - 2 * sizeof(unsigned short int) * (slotNum);
    memcpy((byte*) pageData + offset, &slotStartIndex,
           sizeof(unsigned short int));
    offset += sizeof(unsigned short int);
    memcpy((byte*) pageData + offset, &slotOffset, sizeof(unsigned short int));

    return 0;
}


/**
 * update record to page at slot number
 * need to move record in page
 */
RC RecordBasedFileManager::updateRecordToPageAtSlotNumber(void * pageData,
                                                          const void * formatData, const unsigned short int &dataSize,
                                                          const unsigned short int &currentSlotNum) {
// read page information
    unsigned short int firstAvailableSlotIndex;
    unsigned short int slotNumRecord;
    if (this->getPageInfoByData(pageData, firstAvailableSlotIndex,
                                slotNumRecord) != 0) {
        cout << "!!!fail to get page info, in insertRecord2Slot." << endl;
        return -1;
    }

// read current slot info
    unsigned short int currentSlotStartIndex;
    unsigned short int currentSlotOffset;
    if (this->getSlotInfo(pageData, currentSlotNum, currentSlotStartIndex,
                          currentSlotOffset) != 0) {
        cout
                << "!!!fail to get slot info in RecordBasedFileManager::insertRecordToPage"
                << endl;
        return -1;
    }

    if((dataSize - currentSlotOffset + firstAvailableSlotIndex + slotNumRecord * 2 * sizeof(unsigned short int)) >= PAGE_SIZE){
        cout << "!!!calculate error"<< endl;
        return -1;
    }

//	cout << "before insert, following start from:" << currentSlotStartIndex + currentSlotOffset <<endl;
    // move records
    memmove((byte*) pageData + currentSlotStartIndex + dataSize,
            (byte*) pageData + currentSlotStartIndex + currentSlotOffset,
            firstAvailableSlotIndex - currentSlotStartIndex - currentSlotOffset);
    // calculate move offset maybe positive
    short int moveOffset = dataSize - currentSlotOffset;
//	cout << "move offset: " << moveOffset << endl;

    // copy formatData into pageData
    memcpy((byte*) pageData + currentSlotStartIndex, formatData, dataSize);

    // update currentSlotOffset
    currentSlotOffset = dataSize;
    if (this->updateSlotInfo(pageData, currentSlotNum, currentSlotStartIndex,
                             currentSlotOffset) != 0) {
        cout << "!!!fail to update slot information" << endl;
        return -1;
    }

// update all following slot information
    unsigned short int followingSlotNum = currentSlotNum + 1;
    while (followingSlotNum <= slotNumRecord) {
        unsigned short int followingSlotStartIndex;
        unsigned short int followingSlotOffset;
        if (this->getSlotInfo(pageData, followingSlotNum,
                              followingSlotStartIndex, followingSlotOffset) != 0) {
            cout
                    << "!!!cannot get slot information in RecordBasedFileManager::deleteRecordInPage."
                    << endl;
            return -1;
        }

        // update following slot start index
        followingSlotStartIndex += moveOffset;
//		cout << "following slot:" << followingSlotNum << ",startIndex:"<< followingSlotStartIndex << ",slotOffset:" << followingSlotOffset << endl;
        if (this->updateSlotInfo(pageData, followingSlotNum,
                                 followingSlotStartIndex, followingSlotOffset) != 0) {
            cout
                    << "!!!cannot update slot information in RecordBasedFileManager::deleteRecordInPage."
                    << endl;
            return -1;
        }
        followingSlotNum++;
    }
// update firstAvailableIndex
    firstAvailableSlotIndex += moveOffset;
    if (this->updatePageInfo(pageData, firstAvailableSlotIndex, slotNumRecord)
        != 0) {
        cout << "!!!fail to update page information" << endl;
        return -1;
    }
    return 0;
}
/* --------------------------------------------------------------------------- */

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const void *data, RID &rid) {
    if (recordDescriptor.size() == 0) {
        cout << "record descriptor is empty in RecordBasedFileManager::insertRecord"<< endl;
        return -1;
    }


// format data
// get format data size
    unsigned short int format_data_size;
    void * format_data = malloc(PAGE_SIZE);

    cout<<"ankitesh real data "<<data<<" "<<fileHandle.n_pages<< endl;

    if (this->data2FormatData(recordDescriptor, data, format_data,
                              format_data_size)) {
        free(format_data);
        cout << "!!!fail to format data in RecordBasedFileManager::insertRecord" << endl;
        return -1;

    }

    cout<<"inserted "<<format_data<<endl;
    if (this->insertFormatDataToFile(fileHandle, format_data, format_data_size,
                                     rid)) {
        free(format_data);
        cout << "!!!fail to insert format data to file in InsertRecord" << endl;
        return -1;
    }

    free(format_data);
    return 0;
}


//functions pertaining to read record

/* -------------------------------------------------------------------------- */

RC RecordBasedFileManager::formatData2Output(
        const vector<Attribute> &recordDescriptor, const void * format_data,
        void * data, const unsigned short int &format_data_size) {

    unsigned short int offset = 0;
    offset += sizeof(RID)
              + (recordDescriptor.size() + 1) * sizeof(unsigned short int);

    memcpy(data, (byte*) format_data + offset,
           format_data_size - offset - sizeof(RID));

    offset = format_data_size - sizeof(RID);


    return 0;
}



RC RecordBasedFileManager::readInFormatFromFile(FileHandle fileHandle,
            const RID &rid, void * format_data, unsigned short int &format_data_size) {
    // read page
    void * page_data = malloc(PAGE_SIZE);
    if (fileHandle.readPage(rid.pageNum, page_data) != 0) {
        free(page_data);
        cout << "Can't read the page"<<endl;
        return -1;
    }

    if (this->fetchFormatDataPage(page_data, rid.slotNum, format_data,
                                  format_data_size) != 0) {
        free(page_data);
        cout << "Can't find the data in the page";
        return -1;
    }

    //free the page data as it is copied on the format data
    free(page_data);

    // check first rid
    bool is_current_data = false;


    if (this->isFormatDataCurrentLink(format_data, format_data_size, is_current_data) != 0) {
        return -1;
    }


    if (!is_current_data) {
        return 0;
    }
    else {

        RID first_rid;
        if (this->fetchFirstInFormat(format_data, format_data_size, first_rid)!= 0) {
            return -1;
        }

        if (this->readInFormatFromFile(fileHandle, first_rid, format_data,format_data_size) != 0) {
            cout << "failed to find next record"<<endl;
            return -1;
        }
    }
    return 0;
}

RC RecordBasedFileManager::fetchFormatDataPage(const void * page_data,
                                               const unsigned short int &slot_num, void * data,
                                               unsigned short int &data_size) {

// get slot information
    unsigned short int start_index;
    unsigned short int slot_offset;
    if (this->fetchSlotInfo(page_data, slot_num, start_index, slot_offset)
        != 0) {
        cout << "No slot available"<<endl;
        return -1;
    }

    if (slot_offset == 0) {
        cout << "This slot is no longer present"<<endl;
        return -1;
    }

    memcpy(data, (byte*) page_data + start_index, slot_offset);
    data_size = slot_offset;
    return 0;
}


RC RecordBasedFileManager::fetchSlotInfo(const void * page_data,
                                       const unsigned short int slot_num, unsigned short int &slot_start,
                                       unsigned short int &slot_offset) {
    // check slotNum
    if(slot_num < 2){
        cout << "slot is below the bounds"<< endl;
        return -1;
    }

    unsigned short int offset = PAGE_SIZE - 2 * sizeof(unsigned short int) * (slot_num);
    memcpy(&slot_start, (byte*) page_data + offset,
           sizeof(unsigned short int));
    memcpy(&slot_offset,
           (byte*) page_data + offset + sizeof(unsigned short int),
           sizeof(unsigned short int));
    return 0;

}


RC RecordBasedFileManager::isFormatDataCurrentLink(const void * format_data,
                                            const unsigned short int data_size, bool &is_current_data) {
    RID first_rid;
    is_current_data = false;
    if (this->fetchFirstInFormat(format_data, data_size, first_rid) != 0) {
        return -1;
    }
    if (first_rid.pageNum != 0) {
        is_current_data = true;
    }
    return 0;
}


RC RecordBasedFileManager::fetchFirstInFormat(const void * format_data,
                                                   const unsigned short int format_data_size, RID &rid) {
    if (format_data_size < sizeof(RID)) {
        cout << "RID is out of bounds" << endl;
        return -1;
    }
    memcpy(&rid.pageNum, (byte*) format_data, sizeof(rid.pageNum));
    memcpy(&rid.slotNum, (byte*) format_data + sizeof(rid.pageNum), sizeof(rid.slotNum));
    return 0;
}

/* -------------------------------------------------------------------------- */

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                      const RID &rid, void *data) {


    void * format_data = malloc(PAGE_SIZE);
    unsigned short int format_data_size;

    if (this->readInFormatFromFile(fileHandle, rid, format_data, format_data_size)
        != 0) {
        free(format_data);
        return -1;
    }
    cout<<"ankitesh read from format "<<format_data<<endl;
//    format_data = (void*)0x62100000a100;
    cout<<"ankitesh read from format "<<format_data<<endl;

    if (this->formatData2Output(recordDescriptor, format_data, data,
                              format_data_size) != 0) {
        free(format_data);
        cout << "!!!fail to format data to data" << endl;

        return -1;
    }
    cout<<"ankitesh final data "<<data<<endl;
//    data = (void* )0x60b000000040;
    free(format_data);
    return 0;
}


RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const RID &rid) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const std::vector<Attribute> &recordDescriptor, const void *data) {
    unsigned fieldNum = recordDescriptor.size();
    if (fieldNum == 0) {
        cout
                << "!!!empty record descriptor in RecordBasedFileManager::printRecord"
                << endl;
        return -1;
    }
    unsigned nullFieldsIndicatorActualSize = ceil((double) fieldNum / CHAR_BIT);
    unsigned char * nullFieldsIndicator = new unsigned char[nullFieldsIndicatorActualSize];
    memcpy(nullFieldsIndicator, data, nullFieldsIndicatorActualSize);
    // set field position point array
    int * fieldPosition = new int[fieldNum];
// initial point offset
    cout<<"============="<<endl;
    unsigned positionOffset = nullFieldsIndicatorActualSize;
    for (unsigned i = 0; i < fieldNum; i++) {
        cout<<"ankitesh "<<endl;

        cout << recordDescriptor[i].name << ":\t";

        bool nullBit = nullFieldsIndicator[i / CHAR_BIT]
                       & (1 << (CHAR_BIT - 1 - i % CHAR_BIT));
        if (!nullBit) { // if not null
            AttrType type = recordDescriptor[i].type;
            if (type == TypeInt) {
                int * buffer = new int;
                memcpy(buffer, (byte*) data + positionOffset, sizeof(int));

                cout << *(int*) buffer << "\t";

                delete buffer;
                positionOffset += sizeof(int);
            } else if (type == TypeReal) {
                float * buffer = new float();
                memcpy(buffer, (byte*) data + positionOffset, sizeof(float));

                cout << *(float*) buffer << "\t";

                delete buffer;
                positionOffset += sizeof(float);
            } else {

                int len = 0;
                memcpy(&len, (byte*) data + positionOffset, sizeof(int));
                positionOffset += sizeof(int);
                cout<<"inside "<<len<<endl;
                char * buffer = new char[len + 1];
                memcpy(buffer, (byte*) data + positionOffset,
                       len * sizeof(char));
                cout<<"inisde"<<endl;

                buffer[len] = '\0';

                cout << (char*) (buffer) << "\t\t";

                delete[] buffer;
                positionOffset += len;
            }
        } else { // if null

            cout << "NULL" << "\t";

            positionOffset += 0;
        }
        fieldPosition[i] = positionOffset;


    }

    cout << "" << endl;

    delete[] nullFieldsIndicator;
    return 0;
}

RC RecordBasedFileManager::updateRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const void *data, const RID &rid) {
    return -1;
}

RC RecordBasedFileManager::readAttribute(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                         const RID &rid, const std::string &attributeName, void *data) {
    return -1;
}

RC RecordBasedFileManager::scan(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                const std::string &conditionAttribute, const CompOp compOp, const void *value,
                                const std::vector<std::string> &attributeNames, RBFM_ScanIterator &rbfm_ScanIterator) {
    return -1;
}
