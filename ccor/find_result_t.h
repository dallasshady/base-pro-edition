#ifndef HF0985599_63C6_4204_9E92_4C8C98A1BCD9
#define HF0985599_63C6_4204_9E92_4C8C98A1BCD9
#include "../shared/ccor.h"

// structure to hold find result information
template <typename T> class find_result_t {
    int numItems;
    const T * items;
public:
    find_result_t() { numItems = 0; items = 0; }
    find_result_t(const std::vector<T>& v) { numItems = v.size(); items = &v[0]; }
    find_result_t(int numItems, const T * items) { this->numItems=numItems; this->items=items; }
    const T * begin() const { return items; }
    const T * end() const   { return &items[numItems]; }
    const T& front() const  { return items[0]; }
    const T& back() const   { return items[numItems-1]; }
    bool empty() const      { return numItems<=0; }
    int size() const        { return numItems; }
    const T& operator[](int i) const { return items[i]; }
};

// Result type for ICore::findByType()
typedef find_result_t<ccor::entid_t> find_entity_t;

// Result type for ICore::findTriggers()
typedef find_result_t<ccor::trigid_t> find_trigger_t;

// Result type for IParamPack::findParams()
typedef find_result_t<ccor::paramid_t> find_param_t;

// Result type for ICore::findTypesForInterface()
typedef find_result_t<const char*> find_str_t;

#endif
