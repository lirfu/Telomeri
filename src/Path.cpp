#include <Path.hpp>

#include <ostream>

ulong Path::length() const { 
    // FIXME Store this value in a member when path has been constructed since
    // it is accessed multiple times.
    ulong l = 0;
    for (const OverlapGraph::Edge *e:edges_) {
        l += e->overlap_length; 
        // FIXME Must calculate the extension lengths as well.
    }
    return l;
}

std::ostream& operator<< (std::ostream& s, const Path& p) {
    bool e = false;
    int ni = 0;
    for (int i = 0, n = static_cast<int>(p.nodes_.size() + p.edges_.size());
            i < n;
            i++) {
        if (e) {
            s << "-";
            e = false;
        } else {
            if (p.nodes_[ni]->anchor) {
                s << '*';
            }
            s << 'n' << p.nodes_[ni]->index;
            ni++;
            e = true;
        }
    }
    return s;
}

bool Path::operator==(const Path& other) const {
    return nodes_.size() == other.nodes_.size()
            && std::equal(nodes_.begin(), nodes_.end(), other.nodes_.begin());
}
