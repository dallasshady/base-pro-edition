#ifndef H8D5B79DD_34FE_4718_86CA_6014F3878DE5
#define H8D5B79DD_34FE_4718_86CA_6014F3878DE5
namespace ccor {

class VtableDisplace {
public:

    void * displaceInterface(void * pThis);

    void reset();

    void clear();

private:

    std::vector<char*> _p;

};

}
#endif
