#include "headers.h"
#include "Idset.h"
namespace ccor {

bool Idset::load(const char * fname) {

    names.clear();
    nsStart.clear();
    nameInd.clear();

    // Read data from a file
    FILE * fp = fopen(fname,"rt");
    if (!fp) return false;
    char line[256];
    std::string name;
    typedef std::map<int, std::string> IdMap;
    IdMap idmap;
    int id;
    int maxId = 0;
    while ((fgets(line, 256, fp))!=NULL) {

        int c = 0;
        while (line[c] && line[c]!='\n' && isspace(line[c])) ++c;
        if (!line[c] || line[c]=='\n') continue;

        int d = c;
        while (line[d] && line[d]!='\n' && !isspace(line[d]) && line[d]!='=') ++d;
        if (!line[d] || line[d]=='\n') continue;

        name.assign(line, c, d-c);
        while (line[d] && line[d]!='\n' && line[d]!='=') ++d;
        if (line[d]!='=') continue;

        if (sscanf(&line[d+1], "%x", &id) == 1) {

            // Check identifiers for validity
            if (id==0) {
                fclose(fp);
                throw Exception("ccor : Invalid identifier for \"%s\".\n"
                    "Please run mkid.bat on source code.", name.c_str());
            }
            if (idmap.find(id)!=idmap.end()) {
                fclose(fp);
                throw Exception("ccor : Duplicate identifier 0x%4X for \"%s\" and \"%s\".\n"
                    "Please run mkid.bat on source code.", 
                    id, idmap.find(id)->second.c_str(), name.c_str());
            }

            idmap[id] = name;
            if (maxId < id) maxId = id;
        }
    }
    fclose(fp);

    // Now process pairs <id,name> we've just read
    int maxNs = maxId >> 16;
    nsStart.resize(maxNs+1);
    for (int ns=0; ns <= maxNs; ++ns) {

        IdMap::const_iterator a = idmap.lower_bound(ns << 16);
        IdMap::const_iterator b = idmap.upper_bound((ns << 16) | 0xffff);
        nsStart[ns] = nameInd.size();
        int ind = 0;
        for ( ; a != b; ++a ) {
            if ((a->first >> 16) != ns)
                continue;
            int id = a->first & 0xffff;
            assert(id>=ind);
            for ( ; ind<=id; ++ind ) {
                nameInd.push_back(-1);
            }
            nameInd.back() = names.size();
            names.insert(names.end(), a->second.begin(), a->second.begin()+a->second.size()+1);
        }
    }

    return true;
}


const char * Idset::getName(globid_t id) {

    unsigned int ns = id >> 16;
    unsigned int n = id & 0xffff;
    if (ns>=0 && ns<nsStart.size()) {
        unsigned int index = nsStart[ns] + n;
        if (index >= 0) {
            if (ns < nsStart.size()-1 && index < nsStart[ns+1] ||
                ns==nsStart.size()-1 && index < nameInd.size()) 
            {
                int iname = nameInd[index];
                if (iname>=0) 
                    return &names[iname];
            }
        }
    }

    return NULL;

}


static bool extract_namespace(
    const char * name, 
    const char * & nsStart, 
    const char * & nsEnd
    ) 
{
    nsStart = nsEnd = 0;
    const char s_interface[] = "interface.";
    const char s_trigger[] = "trigger.";
    const char s_event[] = "event.";
    if (strncmp(name,s_interface,sizeof(s_interface)-1)==0) {
        nsStart = &name[sizeof(s_interface)-1];
        nsEnd = strchr(nsStart,'.');
    }
    else if (strncmp(name,s_trigger,sizeof(s_trigger)-1)==0) {
        nsStart = &name[sizeof(s_trigger)-1];
        nsEnd = strchr(nsStart,'.');
    }
    else if (strncmp(name,s_event,sizeof(s_event)-1)==0) {
        nsStart = &name[sizeof(s_event)-1];
        nsEnd = strchr(nsStart,'.');
    }
    return nsStart!=0;
}


globid_t Idset::find(const char * name) {

    const char * nsBegin, * nsEnd;
    if (extract_namespace(name, nsBegin, nsEnd)) {

        // Try to find equivalent namespace
        for (int ns = 0; ns < nsStart.size(); ++ns) {

            if (ns<nsStart.size()-1 && nsStart[ns]==nsStart[ns+1])
                continue;
            int idName = nameInd[nsStart[ns]];
            if (idName<0)
                continue;
            const char * id = &names[idName];
            const char * a, * b;
            if (!extract_namespace(id, a, b))
                continue;
            if (b-a!=nsEnd-nsBegin || strncmp(a, nsBegin, b-a)!=0)
                continue;

            // Perform search within selected namespace only
            globid_t gid = findAtNamespace(ns, name);
            if (gid >= 0) return gid;
        }
    }

    // Perform complete search within all namespaces
    for (int ns = 0; ns < nsStart.size(); ++ns) {
        globid_t gid = findAtNamespace(ns, name);
        if (gid >= 0) return gid;
    }

    return -1;
}


globid_t Idset::findAtNamespace(int ns, const char * name) {

    int next;
    if (ns < nsStart.size()-1) {
        next = nsStart[ns+1];
    }
    else next = nameInd.size();

    for (int i=nsStart[ns]; i < next; ++i) {
        int iname = nameInd[i];
        if (iname>=0 && strcmp(name, &names[iname]) == 0) {
            return (ns << 16) | ((i - nsStart[ns]) & 0xffff);
        }
    }

    return -1;

}


void Idset::enumTriggers(std::vector<globid_t> & ids) {

    const char s_trigger[] = "trigger.";

    ids.clear();

    // enumerate all triggers
    for (int ns=0; ns < nsStart.size(); ++ns) {
        if (ns<nsStart.size()-1 && nsStart[ns]==nsStart[ns+1])
            continue;

        int next;
        if (ns < nsStart.size()-1) {
            next = nsStart[ns+1];
        }
        else next = nameInd.size();

        for (int i=nsStart[ns]; i < next; ++i) {
            int iname = nameInd[i];
            if (iname>=0) {
                const char * name = &names[iname];
                if (strncmp(name, s_trigger, sizeof(s_trigger)-1) == 0) {
                    int id = (ns << 16) | ((i - nsStart[ns]) & 0xffff);
                    ids.push_back(id);
                }
            }
        }
    }

}


}
