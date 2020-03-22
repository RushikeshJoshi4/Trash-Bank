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

RecordBasedFileManager::RecordBasedFileManager() = default;

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
//    if (this->appendSignedPageInFile(fileHandle) != 0)
//        return -1;

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


//RC RecordBasedFileManager::insertFormatDataToFile(FileHandle fileHandle,
//                                                  const void * formatData, const unsigned short int &dataSize, RID &rid) {
//// find available page
//// suppose the following step would insert data and slot
//    if (this->findAvailablePageInFile(fileHandle, dataSize, rid.pageNum) != 0) {
//        cout
//                << "!!!fail to find suitable insert position in RecordBasedFileManager::insertRecord"
//                << endl;
//        return -1;
//    }
//
//// get page data
//    void * pageData = malloc(PAGE_SIZE);
//    if (fileHandle.readPage(rid.pageNum, pageData) != 0) {
//        free(pageData);
//        cout << "!!!fail to read page in RecordBasedFileManager::insertRecord"
//             << endl;
//        return -1;
//    }
//
//// insert format data into page data
//    if (this->insertRecordToPage(pageData, formatData, dataSize, rid.slotNum)
//        != 0) {
//        free(pageData);
//        cout
//                << "!!!fail to insert record into page in RecordBasedFileManager::insertRecord"
//                << endl;
//        return -1;
//    }
//
////	cout << "slotNum:" << rid.slotNum << endl;
//
//// write back page data to file
//    if (fileHandle.writePage(rid.pageNum, pageData) != 0) {
//        free(pageData);
//        cout
//                << "!!!fail to write back page data in RecordBasedFileManager::insertRecord."
//                << endl;
//    }
//
//    free(pageData);
//    return 0;
//}
//
//
//
//RC RecordBasedFileManager::findAvailablePageInFile(FileHandle &fileHandle,
//                                                   const unsigned short int dataSize, unsigned &pageNum) {
//
//    PageNum total_pages = fileHandle.getNumberOfPages();
//    PageNum curr_page_index = 0;
//
//    // insert page should start from second page
//    if (total_pages < 2) {
//        cout << "Not the first page." << endl;
//        return -1;
//    }
//    // a more easier strategy
//    curr_page_index = (total_pages - 1);
//
//    // try to find slot without append from
//    bool curr_page_available = false;
//    void * page_data = malloc(PAGE_SIZE);
//
//    while(curr_page_index < total_pages){
//        if(fileHandle.readPage(currentPageIndex, pageData)!=0){
//            free(pageData);
//            return -1;
//        }
//        if(this->isCurrentPageAvailableInsert(pageData, dataSize, currentPageAvailable)!=0){
//            free(pageData);
//            return -1;
//        }
//        if(currentPageAvailable){
//            pageNum = currentPageIndex;
//            free(pageData);
//            return 0;
//        }
//        currentPageIndex++;
//    }
//
//    // else
//    // append next page
//    if (this->appendSignedPageInFile(fileHandle) != 0) {
//        free(pageData);
//        cout
//                << "!!!fail to append new signed page in RecordBasedFileManager::findSlotPosition."
//                << endl;
//        return -1;
//    }
//
//    // read page
//    if(fileHandle.readPage(currentPageIndex, pageData)!=0){
//        free(pageData);
//        return -1;
//    }
//
//    // check if page is available
//    if(this->isCurrentPageAvailableInsert(pageData, dataSize, currentPageAvailable)!=0){
//        free(pageData);
//        return -1;
//    }
//    if(currentPageAvailable){
//        // return pageNum
//        pageNum = currentPageIndex;
//        free(pageData);
//        return 0;
//    }
//
//    free(pageData);
//    cout << "!!!fail to find available page" << endl;
//    return -1;
//}


/* ------------------------------------* */



void RecordBasedFileManager::reorderSlots(const Offset target_offset, const Offset start, const Offset end, char* &page_data)
{
    Offset slot_count;
    memcpy(&slot_count, page_data + PAGE_SIZE - sizeof(Offset), sizeof(Offset));
    if (start > slot_count - 1)
        return;
    //Find the first undeleted slot in this range ([startSlot, endSlot])
    bool found_start = false;
    Offset start_offset;
    for (Offset i = start; i <= end; i++){

        memcpy(&start_offset, page_data + PAGE_SIZE - sizeof(Offset) * (i + 2), sizeof(Offset));
        if (start_offset != DELETEDSLOT)
        {
            found_start = true;
            break;
        }
    }
    //If all the slots in this range has been deleted before
    if (!found_start)
        return;
    Offset end_offset;
    for (Offset i = end; i >= start; i--){

        memcpy(&end_offset, page_data + PAGE_SIZE - sizeof(Offset) * (i + 2), sizeof(Offset));
        if (end != DELETEDSLOT)
            break;
    }
    Offset end_slot_size;
    memcpy(&end_slot_size, page_data + end_offset, sizeof(Offset));
    end_offset += end_slot_size;

    Offset move_len = end_offset - start_offset;
    memmove(page_data + target_offset, page_data + start_offset, move_len);
    Offset delta = target_offset - start_offset;
    for (Offset i = start; i <= end; i++){

        Offset curr_slot_offset;
        memcpy(&curr_slot_offset, page_data + PAGE_SIZE - sizeof(Offset) * (i + 2), sizeof(Offset));
        //If the slot has been deleted before, then we do not need to change its offset, which is -1
        if (curr_slot_offset != DELETEDSLOT)
        {
            curr_slot_offset += delta;
            memcpy(page_data + PAGE_SIZE - sizeof(Offset) * (i + 2), &curr_slot_offset, sizeof(Offset));
        }
    }
}

RC RecordBasedFileManager::appendInPage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, int page_num, const void *data, RID &rid, Offset &field_info_size, char* &field_info, Offset &data_size)
{
    //Pretend that we read the page to get the page size
    Offset old_page_size = fileHandle.all_page_size[page_num];
    ++fileHandle.readPageCounter;

    Offset slot_size = sizeof(Offset) + 2 * sizeof(Marker) + field_info_size + data_size; // slotSizeNumber + isUpdatedRecord + version + fieldInfoSize + dataSize
    Offset new_page_size = old_page_size + slot_size + sizeof(Offset); // In fact newPageSize would be oldPageSize + slotSize, if we decide to reuse a previous deleted slot
    if (new_page_size <= PAGE_SIZE){
        char *page_data = (char*)malloc(PAGE_SIZE);
        RC status = fileHandle.readPage(page_num, page_data);
        if (status == -1){
            cerr << "Cannot read page " << page_num << " while append data in that page" << endl;
            free(page_data);
            return -1;
        }

        Offset old_slot_count;
        memcpy(&old_slot_count, page_data + PAGE_SIZE - sizeof(Offset), sizeof(Offset));
        Offset target_slot = generateSlotTable(page_data, old_slot_count, slot_size);
        Offset new_slot_count;
        memcpy(&new_slot_count, page_data + PAGE_SIZE - sizeof(Offset), sizeof(Offset));
        Offset new_slot_table_size = (new_slot_count + 1) * sizeof(Offset);
        Offset old_slot_table_size = (old_slot_count + 1) * sizeof(Offset);
        if (new_slot_table_size != old_slot_table_size)
        {
            //If we add a new slot in the end of the table
            fileHandle.all_page_size[page_num] = new_page_size;
            int null_field_bits = ceil((double)recordDescriptor.size() / CHAR_BIT);
            Offset offset = 0;
            memcpy(page_data + offset, &new_page_size, sizeof(Offset));
            offset += old_page_size - old_slot_table_size;
            memcpy(page_data + offset, &slot_size, sizeof(Offset));
            offset += sizeof(Offset);
            Marker isUpdated = RecordUpdateMarker ::Origin;
            memcpy(page_data + offset, &isUpdated, sizeof(Marker));
            offset += sizeof(Marker);
            Marker version = 'a';
            memcpy(page_data + offset, &version, sizeof(Marker));
            offset += sizeof(Marker);
            memcpy(page_data + offset, field_info, field_info_size);
            offset += field_info_size;
            memcpy(page_data + offset, (char *)data + null_field_bits, data_size);
        }
        else
        {
            //If we reuse a previous deleted slot
            new_page_size -= sizeof(Offset); //Since we didn't change the size of slot table
            fileHandle.all_page_size[page_num] = new_page_size;
            Offset offset = 0;
            memcpy(page_data + offset, &new_page_size, sizeof(Offset));
            Offset target_slot_offset;
            memcpy(&target_slot_offset, page_data + PAGE_SIZE - sizeof(Offset) * (target_slot + 2), sizeof(Offset));
            if (target_slot < old_slot_count - 1) // We need to move back other slots to make more space if we insert the slot in the middle
                reorderSlots(target_slot_offset + slot_size, target_slot + 1, old_slot_count - 1, page_data);
            offset = target_slot_offset;
            int null_field_bits = ceil((double)recordDescriptor.size() / CHAR_BIT);
            memcpy(page_data + offset, &slot_size, sizeof(Offset));
            offset += sizeof(Offset);
            Marker isUpdated = RecordUpdateMarker ::Origin;
            memcpy(page_data + offset, &isUpdated, sizeof(Marker));
            offset += sizeof(Marker);
            Marker version = 'a';
            memcpy(page_data + offset, &version, sizeof(Marker));
            offset += sizeof(Marker);
            memcpy(page_data + offset, field_info, field_info_size);
            offset += field_info_size;
            memcpy(page_data + offset, (char *)data + null_field_bits, data_size);
        }
        ++fileHandle.insertCount;
        status = fileHandle.writePage(page_num, page_data);
        if (status == -1){
            cerr << "Cannot write page " << page_num << endl;
            fileHandle.all_page_size[page_num] = old_page_size;
            free(page_data);
            return -1;
        }
        rid.pageNum = page_num;
        rid.slotNum = target_slot;
        free(page_data);
    }
    else{
        return 0;
    }
    return 1;
}


Offset RecordBasedFileManager::generateSlotTable(char* &data, Offset &slot_count, Offset &slot_size)
{
    Offset new_slot_count = slot_count + 1;
    if (slot_count == 0){
        //If the slot table is empty
        memcpy(data + PAGE_SIZE - sizeof(Offset), &new_slot_count, sizeof(Offset));
        Offset first_slot_offset = sizeof(Offset);
        memcpy(data + PAGE_SIZE - sizeof(Offset) * (new_slot_count + 1), &first_slot_offset, sizeof(Offset));
    }
    else{
        //If the slot table is not empty
        //First find out whether there is a deleted slot in the table
        //If not, we will add the slot in the back
        for (Offset i = 0; i < slot_count; i++){

            Offset slot_offset;
            memcpy(&slot_offset, data + PAGE_SIZE - sizeof(Offset) * (i + 2), sizeof(Offset));
            if (slot_offset == DELETEDSLOT){

                //If deleted slot is not the last slot in table
                if (i < slot_count - 1){
                    //Find the offset of an undeleted slot to replace the offset of current slot
                    bool found_notdeleted = false;
                    for (Offset j = i + 1; j < slot_count; j++){
                        Offset next_slot_offset;
                        memcpy(&next_slot_offset, data + PAGE_SIZE - sizeof(Offset) * (j + 2), sizeof(Offset));
                        if (next_slot_offset != DELETEDSLOT){
                            memcpy(data + PAGE_SIZE - sizeof(Offset) * (i + 2), &next_slot_offset, sizeof(Offset));
                            found_notdeleted = true;
                            break;
                        }
                    }
                    //If all the slot after current slot were deleted, we will look forward to generate the offset of current slot
                    if (!found_notdeleted){
                        //Whether current slot is the first slot in the slot table
                        //If not, the slotOffset = previousSlotOffset + previousSlotSize
                        if (i == 0){
                            Offset targetSlotOffset = sizeof(Offset);
                            memcpy(data + PAGE_SIZE - sizeof(Offset) * (i + 2), &targetSlotOffset, sizeof(Offset));
                        }
                        else{
                            Offset previous_slot_offset;
                            memcpy(&previous_slot_offset, data + PAGE_SIZE - sizeof(Offset) * (i + 1), sizeof(Offset));
                            Offset prev_slot_size;
                            memcpy(&prev_slot_size, data + previous_slot_offset, sizeof(Offset));
                            Offset target_slot_offset = previous_slot_offset + prev_slot_size;
                            memcpy(data + PAGE_SIZE - sizeof(Offset) * (i + 2), &target_slot_offset, sizeof(Offset));
                        }
                    }
                }
                else{
                    Offset prev_slot_offset;
                    memcpy(&prev_slot_offset, data + PAGE_SIZE - sizeof(Offset) * (i + 1), sizeof(Offset));
                    Offset prev_slot_size;
                    memcpy(&prev_slot_size, data + prev_slot_offset, sizeof(Offset));
                    Offset target_slot_offset = prev_slot_offset + prev_slot_size;
                    memcpy(data + PAGE_SIZE - sizeof(Offset) * (i + 2), &target_slot_offset, sizeof(Offset));
                }
                return i;
            }
        }

        Offset last_slot_offset;
        memcpy(&last_slot_offset, data + PAGE_SIZE - sizeof(Offset) * (slot_count + 1), sizeof(Offset));
        Offset last_slot_size;
        memcpy(&last_slot_size, data + last_slot_offset, sizeof(Offset));
        Offset new_slot_offset = last_slot_offset + last_slot_size;
        memcpy(data + PAGE_SIZE - sizeof(Offset), &new_slot_count, sizeof(Offset));
        memcpy(data + PAGE_SIZE - sizeof(Offset) * (new_slot_count + 1), &new_slot_offset, sizeof(Offset));
    }
    return slot_count;
}

RC RecordBasedFileManager::addInNewPage(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid, Offset &field_info_size, char* &field_info, Offset &data_size)
{
    Offset slot_size = sizeof(Offset) + 2 * sizeof(Marker) + field_info_size + data_size; // slotSizeNumber + isUpdatedRecord + version + fieldInfoSize + dataSize
    int null_field_bits = ceil((double)recordDescriptor.size() / CHAR_BIT);
    char *new_data = (char*)calloc(PAGE_SIZE, 1);
    Offset slot_count = 0;
    Offset targetSlot = generateSlotTable(new_data, slot_count, slot_size);
    if (targetSlot != 0){
        cerr << "Cannot generate slot table when add data in a new page" << endl;
        free(new_data);
        return -1;
    }
    Offset page_size = sizeof(Offset) + slot_size + sizeof(Offset) * 2; //pageSizeNum + slotSize + slotTableSize
    fileHandle.all_page_size.push_back(page_size);
    Offset offset = 0;
    memcpy(new_data + offset, &page_size, sizeof(Offset));
    offset += sizeof(Offset);
    memcpy(new_data + offset, &slot_size, sizeof(Offset));
    offset += sizeof(Offset);
    Marker is_updated_record = RecordUpdateMarker::Origin;
    memcpy(new_data + offset, &is_updated_record, sizeof(Marker));
    offset += sizeof(Marker);
    Marker version = 'a';
    memcpy(new_data + offset, &version, sizeof(Marker));
    offset += sizeof(Marker);
    memcpy(new_data + offset, field_info, field_info_size);
    offset += field_info_size;
    memcpy(new_data + offset, (char *)data + null_field_bits, data_size);
    fileHandle.insertCount++;
    RC status = fileHandle.appendPage(new_data);
    if (status == -1){
        cerr << "Cannot append a page while insert a record" << endl;
        fileHandle.all_page_size.pop_back();
        free(new_data);
        return -1;
    }
    int total_pages = fileHandle.getNumberOfPages();
    rid.pageNum = total_pages - 1;
    rid.slotNum = 0;
    free(new_data);
    return 0;
}

void RecordBasedFileManager::generateFieldData(const vector<Attribute> &recordDescriptor, const void *data, Offset &result_len, char* &result, Offset &data_size)
{
    Offset data_offset = 0;
    bool null_bit = false;
    int null_fields_bits = ceil((double)recordDescriptor.size() / CHAR_BIT);
    unsigned char *null_fields_indicator = (unsigned char*)malloc(null_fields_bits);
    memcpy(null_fields_indicator, (char *)data + data_offset, null_fields_bits);
    data_offset += null_fields_bits;

    Offset n_fields = recordDescriptor.size();
    result_len = sizeof(Offset) + n_fields * sizeof(Offset);
    result = (char*)malloc(result_len);
    Offset offset = 0;
    memcpy(result + offset, &n_fields, sizeof(Offset));
    offset += sizeof(Offset);
    for (size_t i = 0; i < recordDescriptor.size(); i++)
    {
        Offset field_offset = 0;
        null_bit = null_fields_indicator[i / CHAR_BIT] & (1 << (CHAR_BIT - 1 - i % CHAR_BIT));
        if (!null_bit){
            AttrType type = recordDescriptor[i].type;
            if (type == TypeInt){
                field_offset = data_offset;
                data_offset += sizeof(int);
            }
            else if (type == TypeReal){
                field_offset = data_offset;
                data_offset += sizeof(float);
            }
            else if (type == TypeVarChar){
                field_offset = data_offset;
                int str_len;
                memcpy(&str_len, (char *)data + data_offset, sizeof(int));
                data_offset += sizeof(int);
                data_offset += str_len;
            }
        }
        else
            field_offset = NULLDATA;
        if (field_offset != NULLDATA)
            field_offset += sizeof(Offset) + 2 * sizeof(Marker) + result_len - null_fields_bits;
        memcpy(result + offset, &field_offset, sizeof(Offset));
        offset += sizeof(Offset);
    }
    free(null_fields_indicator);
    data_size = data_offset - null_fields_bits;
}



/* --------------------------------------------------------------------------- */

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const void *data, RID &rid) {
    cout<<"calling insert "<<endl;
    int n_pages = fileHandle.getNumberOfPages();
    Offset data_size;
    Offset field_info_size;
    char* field_info;
    generateFieldData(recordDescriptor, data, field_info_size, field_info, data_size);
    if (n_pages == 0){
        RC status = addInNewPage(fileHandle, recordDescriptor, data, rid, field_info_size, field_info, data_size);
        if (status == -1){
            cerr << "Cannot append a page while insert a record and PageNum = 0" << endl;
            free(field_info);
            return -1;
        }
    }
    else{
        RC status = appendInPage(fileHandle, recordDescriptor, n_pages - 1, data, rid, field_info_size, field_info, data_size);
        if (status == -1){
            cerr << "Cannot add data in the last page while insert a record" << endl;
            free(field_info);
            return -1;
        }
        else if (status == 0){
            for (int i = 0; i < n_pages - 1; i++)
            {
                status = appendInPage(fileHandle, recordDescriptor, i, data, rid, field_info_size, field_info, data_size);
                if (status == -1){
                    cerr << "Cannot add data in page " << i << " while insert a record" << endl;
                    free(field_info);
                    return -1;
                }
                else if (status == 1){
                    free(field_info);
                    return 0;
                }
            }
            status = addInNewPage(fileHandle, recordDescriptor, data, rid, field_info_size, field_info, data_size);
            if (status == -1){
                cerr << "Cannot append a page while insert a record and all the other pages are full" << endl;
                free(field_info);
                return -1;
            }
            free(field_info);
            return 0;
        }
    }
    free(field_info);
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

    if (this->formatData2Output(recordDescriptor, format_data, data,
                              format_data_size) != 0) {
        free(format_data);
        cout << "!!!fail to format data to data" << endl;
        return -1;
    }
    free(format_data);
    return 0;
}


RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle, const std::vector<Attribute> &recordDescriptor,
                                        const RID &rid) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const std::vector<Attribute> &recordDescriptor, const void *data) {


    Offset offset = 0;
    bool null_bit = false;
    int null_fields_bits_actual = ceil((double)recordDescriptor.size() / CHAR_BIT);
    unsigned char *null_fields_bits = (unsigned char*)malloc(null_fields_bits_actual);
    memcpy(null_fields_bits, (char *)data + offset, null_fields_bits_actual);
    offset += null_fields_bits_actual;
    for (size_t i = 0; i < recordDescriptor.size(); i++){

        cout << recordDescriptor[i].name << ": ";
        null_bit = null_fields_bits[i / CHAR_BIT] & (1 << (CHAR_BIT - 1 - i % CHAR_BIT));
        if (!null_bit)
        {
            AttrType type = recordDescriptor[i].type;
            if (type == TypeInt)
            {
                int int_val;
                memcpy(&int_val, (char *)data + offset, sizeof(int));
                offset += sizeof(int);
                cout << int_val << "\t";
            }
            else if (type == TypeReal)
            {
                float float_val;
                memcpy(&float_val, (char *)data + offset, sizeof(float));
                offset += sizeof(float);
                cout << float_val << "\t";
            }
            else if (type == TypeVarChar)
            {
                int str_len;
                memcpy(&str_len, (char *)data + offset, sizeof(int));
                offset += sizeof(int);
                char* str = (char*)malloc(str_len + 1);
                str[str_len] = '\0';
                memcpy(str, (char *)data + offset, str_len);
                offset += str_len;
                cout << str << "\t";
                free(str);
            }
        }
        else
            cout << "Null\t";
    }
    cout << endl;
    free(null_fields_bits);
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
