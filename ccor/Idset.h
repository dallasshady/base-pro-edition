#ifndef H6FF9C81E_D5C0_4944_8393_149322CE4948
#define H6FF9C81E_D5C0_4944_8393_149322CE4948
#include "../shared/ccor.h"
namespace ccor {

class Idset {
public:

    // load ids from data file
    // (return false if file open error)
    bool load(const char * fname);

    // get id name
    // This function may return NULL if id is not found
    const char * getName(globid_t id);

    // find id by its name
    // note! this function is too slow because it performs complete search
    globid_t find(const char * name);

    // enumerate all trigger ids
    void enumTriggers(std::vector<globid_t> & ids);

private:

    std::vector<char> names;

    std::vector<int> nsStart;

    std::vector<int> nameInd;

    void add(const char * name, int id);

    globid_t findAtNamespace(int ns, const char * name);

};


}
#endif
