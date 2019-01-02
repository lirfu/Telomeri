#include <Path.hpp>

#include <ostream>

void Path::updateLength() {
    length_ = 0;
    for (const OverlapGraph::Edge *e:edges_) {
        length_ += e->overlap_length;
        // FIXME Must calculate the extension lengths as well.
    }
}

ulong Path::length() const {
    return length_;
}

std::ostream &operator<<(std::ostream &s, const Path &p) {
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

bool Path::operator==(const Path &other) const {
    return nodes_.size() == other.nodes_.size()
           && std::equal(nodes_.begin(), nodes_.end(), other.nodes_.begin());
}
