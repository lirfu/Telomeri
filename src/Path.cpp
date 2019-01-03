#include <Path.hpp>

#include <ostream>
#include <iostream>

void Path::updateLength() {
    length_ = edges_[0]->t_end - 0; // First anchor.
    for (uint i = 1; i < edges_.size(); i++) {
        length_ += edges_[i]->t_end - edges_[i - 1]->q_end;
    }
    length_ += nodes_.back()->length - edges_.back()->q_end; // Last anchor.
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
