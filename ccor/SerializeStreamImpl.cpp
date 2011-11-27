#include "headers.h"
#include "../shared/ccor.h"
#include "SerializeStreamImpl.h"

SerializeStreamImpl::SerializeStreamImpl(ccor::IResource * resource, bool saving) {

    _lastNextEntry = 0;

}


void SerializeStreamImpl::assertVersion(long versionId) {

    if (false)
    //if (_versionId!=versionId)
        throw ccor::Exception("core : Invalid serialize stream version");

}


void SerializeStreamImpl::setLabel(const char * label) {

    if (label==NULL)
        label = "";

    if (_fpLoad) {

        LabelMap::const_iterator it = _labelMap.find(label);
        if (it==_labelMap.end()) {//throw ccor::Exception("core : label '%s' not found in a loading stream", label);
            findLabelInFile(label);
        }
        else {
            long pos = it->second;
            fseek(_fpLoad, pos, SEEK_SET);
        }
    }

    if (_fpSave) {

        LabelMap::const_iterator it = _labelMap.find(label);
        if (it!=_labelMap.end()) throw ccor::Exception("core : label '%s' already exists in a saving stream", label);

        fseek(_fpSave, 0, SEEK_END);
        long pos = ftell(_fpSave);
        // save label's begin pos
        fseek(_fpSave, _lastNextEntry, SEEK_SET);
        fwrite(&pos, sizeof(pos), 1, _fpSave);
        // save label info
        fseek(_fpSave, pos, SEEK_SET);
        serializeData((void*)label, 'L', sizeof(char), strlen(label)+1);
        // reserve place for next label pointer
        _lastNextEntry = ftell(_fpSave);
        fwrite(&_lastNextEntry, sizeof(_lastNextEntry), 1, _fpSave);
        // store pos 
        pos = ftell(_fpSave);
        _labelMap.insert(LabelMap::value_type(label,pos));

    }
}


void SerializeStreamImpl::serializeData(void * data, char fmt, int itemSize, int numItems) {

    if (numItems!=1) fmt = toupper(fmt);
    if (_fpSave) {
        fwrite(&fmt, sizeof(fmt), 1, _fpSave);
        if (numItems != 1) fwrite(&numItems, sizeof(numItems), 1, _fpSave);
        fwrite( data, itemSize, numItems, _fpSave );
    }
     
    if (_fpLoad) {
        int n = 1;
        char f;
        fread(&f, sizeof(f), 1, _fpLoad);
        if (fmt!=f) throw ccor::Exception("core : Invalid data format to load");
        if (numItems != 1) fread(&n, sizeof(n), 1, _fpLoad);
        if (numItems < n) throw ccor::Exception("core : Invalid number of elements to load");
        fread( data, itemSize, numItems, _fpLoad );
    }
}


int SerializeStreamImpl::getNumElements() {

    if (_fpLoad) {

        int n = 1;
        char f;
        int r = fread(&f, 1, sizeof(f), _fpLoad);
        if (f=='I' || f=='F' || f=='B' || f=='C' || f=='D' || f=='T' || f=='Q' || f=='G' || f=='L') {
            r += fread(&n, 1, sizeof(n), _fpLoad);
        }

        fseek(_fpLoad, -r, SEEK_CUR);
        return n;

    }

    return 0;
}


long SerializeStreamImpl::findLabelInFile(const char * label) {

    assert(_fpLoad==NULL);
    assert(_fpSave!=NULL);

    long curPos;
    long nextPos = _lastNextEntry;
    
    while (true) {

        fseek(_fpLoad, nextPos, SEEK_SET);
        curPos = nextPos;
        fread(&nextPos, sizeof(nextPos), 1, _fpLoad);
        if (curPos==nextPos) {
            // we didn't find it
            throw ccor::Exception("core : label '%s' not found in a loading stream", label);
        }

        // load label string
        fseek(_fpLoad, nextPos, SEEK_SET);
        int n = getNumElements();
        std::string strLabel;
        strLabel.resize(n-1);
        serializeData((void*)strLabel.c_str(), 'L', sizeof(char), n);
        nextPos = ftell(_fpLoad);

        // compare it to what we want to find
        if (strLabel==label) {
            long _v;
            fread(&_v, sizeof(_v), 1, _fpLoad);
            // store label for future use
            long pos = ftell(_fpLoad);
            _labelMap.insert(LabelMap::value_type(label,pos));
            _lastNextEntry = nextPos;
            return pos;
        }
    }

    return 0;
}
